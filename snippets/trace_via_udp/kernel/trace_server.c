#include "trace_server.h"

#include <linux/module.h>
#include <linux/init.h>

#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/inet.h>

#include <linux/wait.h>
#include <linux/sched.h>

#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include <linux/debugfs.h>

/*
 * Interval between sending trace packets in ms.
 * 
 * NOTE: messages with trace marks ignore this interval.
 */
#define PACKETS_INTERVAL 100

#define PACKETS_INTERVAL_JIFFIES (PACKETS_INTERVAL * HZ / 1000)
/*
 * Sensitivity of the server for new trace events.
 * 
 * Interval(in ms) between new event arrival and sending it
 * if no limit from PACKETS_INTERVAL.
 */
#define TRACE_EVENTS_SENSITIVITY 1000

#define TRACE_EVENTS_SENSITIVITY_JIFFIES (TRACE_EVENTS_SENSITIVITY * HZ / 1000)

//Initial buffer size
unsigned short server_port = TRACE_SERVER_PORT;
module_param(server_port, ushort, S_IRUGO);

/* Internal representation of the trace event on the server */
struct server_trace_event
{
	/* FIFO organization of the events */
	struct server_trace_event* next;
	/* Allocated array of bytes as event content */
	char* content;
	/* Size of the event content */
	int content_size;
	/* When event was generated*/
	u64 timestamp;
};

struct server_trace_events
{
	/* Pointer to the first event */
	struct server_trace_event* first;
	/* Pointer to the 'next' field of last event */
	struct server_trace_event** last_p;
	
	/* 
	 * Counter of the current event in the trace.
	 * 
	 * It is used to form seq number of udp packet.
	 */
	int32_t counter;
	/* Protect FIFO of events */	
	spinlock_t lock;
};

static int server_trace_events_init(struct server_trace_events* events)
{
	events->first = NULL;
	events->last_p = NULL;
	spin_lock_init(&events->lock);
	
	return 0;
}

static void server_trace_events_destroy(struct server_trace_events* events)
{
	while(events->first)
	{
		struct server_trace_event* event = events->first;
		events->first = event->next;
		
		kfree(event->content);
		kfree(event);
	}
}

//************** Helpers for write and extract events ****************//
 /*
 * Add event with given parameters into queue.
 * 
 * 'content' should be allocated using kmalloc and after function call
 * it belongs to the events queue.
 */
static int server_trace_add_event(struct server_trace_events* events,
	char* content, int content_size)
{
	unsigned long flags;
	
	struct server_trace_event* event = kmalloc(sizeof(*event), GFP_KERNEL);
	if(event == NULL)
	{
		pr_err("Failed to allocate trace event.");
		return -ENOMEM;
	}
	event->content = content;
	event->content_size = content_size;
	event->next = NULL;
	
	spin_lock_irqsave(&events->lock, flags);
	
	if(events->last_p)
	{
		*events->last_p = event;
	}
	else
	{
		events->first = event;
	}
	events->last_p = &event->next;

	event->timestamp = ktime_to_ns(ktime_get());
	
	spin_unlock_irqrestore(&events->lock, flags);
	
	return 0;
}
/*
 * Deque first event in the queue and return its parameters.
 * 
 * After processing, 'content' should be freed.
 * 
 * Return 1 if queue is empty.
 */
static int server_trace_deque_event(struct server_trace_events* events,
	char** content, int* content_size, u64* ts)
{
	struct server_trace_event* event;
	unsigned long flags;
	
	spin_lock_irqsave(&events->lock, flags);
	
	event = events->first;
	if(event == NULL) goto queue_empty;
	
	events->first = event->next;
	if(event->next == NULL) events->last_p = NULL;
	
	spin_unlock_irqrestore(&events->lock, flags);
	
	*content = event->content;
	*content_size = event->content_size;
	*ts = event->timestamp;

	kfree(event);

	return 0;

queue_empty:
	spin_unlock_irqrestore(&events->lock, flags);
	
	return 1;
}

//****************** Events generator ********************************//
/*
 * Writing to predefined file in debugfs will generate event.
 * Data written will become content of the event.
 */

struct events_generator
{
	struct server_trace_events* events;
	
	struct dentry* event_file;
};

static int event_file_open(struct inode* inode, struct file* filp)
{
	filp->private_data = inode->i_private;
	
	return nonseekable_open(inode, filp);
}

static ssize_t event_file_write(struct file* filp,
	const char __user *buf, size_t count, loff_t *f_pos)
{
	int result;
	
	char* content;
	struct events_generator* generator = filp->private_data;
	
	BUG_ON(generator == NULL);
	
	content = kmalloc(count, GFP_KERNEL);
	if(content == NULL)
	{
		pr_err("Failed to allocate event content.");
		return -ENOMEM;
	}
	
	if(copy_from_user(content, buf, count))
	{
		kfree(content);
		return -EFAULT;
	}
	
	result = server_trace_add_event(generator->events, content, count);
	if(result)
	{
		kfree(content);
		return result;
	}
	
	return count;
}

static int events_generator_init(struct events_generator* generator,
	struct server_trace_events* events,
	struct dentry* control_dir)
{
	static struct file_operations event_file_ops =
	{
		.owner = THIS_MODULE,
		.open = event_file_open,
		.write = event_file_write
	};
	
	generator->events = events;
	generator->event_file = debugfs_create_file("events",
		S_IWUGO,
		control_dir,
		generator,
		&event_file_ops);
	if(generator->event_file == NULL)
	{
		pr_err("Failed to create control file for events generator.");
		return -EINVAL;
	}
	
	return 0;
}
static void events_generator_destroy(struct events_generator* generator)
{
	debugfs_remove(generator->event_file);
}


//************************ Events sender *****************************//

enum events_sender_state_type
{
	/* Uninitialized */
	events_sender_state_invalid = 0,
	/* Initialized and wait commands */
	events_sender_state_ready,
	/* Start to send trace... */
	events_sender_state_starting,
	/* Send trace */
	events_sender_state_send,
	/* Stop to send trace... */
	events_sender_state_stopping,
};

/*
 * State of the sender may be changed in the recieve message callback,
 * so it cannot be protected by mutex, only spinlock.
 * But some actions, which change state, cannot be performed under
 * spinlock. E.g., message sending.
 * 
 * In that case, we firstly read and change state(under spinlock), and
 * then perform action, corresponded to state read. But if this action
 * need to access to state-dependent variables, it cannot take these
 * variables from the sender object, because them may be staled.
 * 
 * For resolve this situation, we copy state-dependend variables when
 * we read and possibly change state(under spinlock). Without lock,
 * actions use this copy instead of sender object's variables.
 * 
 * This structure incorporate all state and state-dependend variables
 * for simplifiy copy of them.
 */
struct events_sender_state
{
	enum events_sender_state_type type;
	/* Whether terminate command has issued(modificator for state) */
	int is_terminated;
	/* Next fields are used only when sender send messages */
	__be32 client_addr;
	__be16 client_port;
};

struct events_sender
{
	struct server_trace_events* events;
	/* Whether no events has not sent till this moment */
	int is_first_event;
	/* State of the sender, also contain state-dependend variables */
	struct events_sender_state state;
	/* Protect state changes */
	spinlock_t lock;
	/* 
	 * Sequential number of the last packet.
	 * 
	 * It is accessed (and changed) only in callback for work, which
	 * is serialized in respect to itself. So, accesses to this field
	 * do not require any sync.
	 */
	int32_t seq;

	/* Is used for send messages */
	struct socket* clientsocket;
	/* Work for send packets to the client */
	struct delayed_work work;
	/* Workqueue for pending 'work' */
	struct workqueue_struct* wq;
	/* Waitqueue for wait until reader stops */
	wait_queue_head_t stop_waiter;
};

/* Helper for send message */
static int events_sender_send_msg(struct events_sender* sender,
	struct events_sender_state* state,
	struct kvec* vec, size_t vec_num, size_t size)
{
	int result;
	struct msghdr msg;

	struct sockaddr_in to;

	BUG_ON(state->type == events_sender_state_ready);
	
	/* Form destination address */
	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = state->client_addr;
	to.sin_port = state->client_port;
	
	/* Form message itself */
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &to;
	msg.msg_namelen = sizeof(to);

	msg.msg_control = NULL;
	msg.msg_controllen = 0;

	result = kernel_sendmsg(sender->clientsocket, &msg, vec, vec_num, size);
	if(result < 0)
	{
		pr_err("Error occure while sending message.");
		return result;
	}

	return 0;
}

/* 
 * Dequeue and send trace packet.
 * 
 * If trace is empty, return 1.
 */
static int events_sender_send_trace_packet(struct events_sender* sender,
	struct events_sender_state* state)
{
	int result;
	int32_t seq;
	
	struct trace_server_msg_packet msg_packet;
	struct kvec vec[2] =
	{
		{
			.iov_base = &msg_packet,
			.iov_len = offsetof(struct trace_server_msg_packet, event.context)
		}
	};
	
	char* event_content;
	int event_size;
	u64 ts;
	
	result = server_trace_deque_event(sender->events,
		&event_content, &event_size, &ts);
	if(result > 0) return 1;//nothing to send
	if(result < 0)
	{
		pr_err("Error occures while dequeue event.");
		return result;
	}
	
	seq = sender->seq++;
	msg_packet.base.seq = htons(seq);
	msg_packet.base.type = TRACE_SERVER_MSG_TYPE_PACKET;
	
	msg_packet.event.context_size = htons(event_size);
	timestamp_nt_set(&msg_packet.event.timestamp, ts);
	
	vec[1].iov_base = event_content;
	vec[1].iov_len = event_size;
	
	result = events_sender_send_msg(sender, state, vec, ARRAY_SIZE(vec),
		offsetof(struct trace_server_msg_packet, event.context) + event_size);
	
	//pr_info("Message %.*s has been sent.", (int)event_size, event_content);
	
	kfree(event_content);

	return result;
}

/* 
 * Send given trace mark.
 */
static int events_sender_send_trace_mark(struct events_sender* sender,
	struct events_sender_state* state,
	char mark)
{
	int32_t seq;
	
	struct trace_server_msg_mark msg_mark;
	struct kvec vec =
	{
		.iov_base = &msg_mark,
		.iov_len = offsetof(struct trace_server_msg_mark, end_struct)
	};
	
	seq = sender->seq++;
	msg_mark.base.seq = htonl(seq);
	msg_mark.base.type = TRACE_SERVER_MSG_TYPE_MARK;
	msg_mark.mark = mark;
	
	return events_sender_send_msg(sender, state, &vec, 1,
		offsetof(struct trace_server_msg_mark, end_struct));
}


/*
 * Work task for events sender.
 * 
 * Implements the most part of the server-client protocol.
 */
static void events_sender_work(struct work_struct *data)
{
	int result;
	
	struct events_sender_state state;
	unsigned long flags;

	struct events_sender* sender = container_of(to_delayed_work(data),
		struct events_sender, work);
	
	/* Read state and change it(if nessessary) at same time */
	spin_lock_irqsave(&sender->lock, flags);
	state = sender->state;
	
	switch(state.type)
	{
	case events_sender_state_send:
		//needn't to change state
		//pr_info("Continue to send trace events.");
	break;
	case events_sender_state_starting:
		sender->state.type = events_sender_state_send;
		//pr_info("Start to send trace events.");
	break;
	case events_sender_state_stopping:
		sender->state.type = events_sender_state_ready;
		//pr_info("Stop to send trace.");
		wake_up_all(&sender->stop_waiter);
	break;
	/* Execution in READY state is strange. */
	case events_sender_state_ready:
		pr_info("Work is executed in READY state.");
	break;
	default:
		pr_info("Invalid trace sender state %d.", (int)state.type);
	}

	spin_unlock_irqrestore(&sender->lock, flags);
	
	/* Similar switch, but now do real work */
	switch(state.type)
	{
	case events_sender_state_send:
		result = events_sender_send_trace_packet(sender, &state);
		if(result > 0)
		{
			if(state.is_terminated)
			{
				/* 
				 * Additional state change in terminated case.
				 * 
				 * Shouldn't conflict with concurrent changes.
				 * (Because in terminate state sender doesn't accept
				 * external commands.)
				 */
				spin_lock_irqsave(&sender->lock, flags);
				sender->state.type = events_sender_state_stopping;
				spin_unlock_irqrestore(&sender->lock, flags);
				
				/*
				 * Move work to queue without timeout.
				 * 
				 * Another possibility - perform all steps corresponding
				 * to STOPPING state and change state to READY.
				 */
				queue_work(sender->wq, &sender->work.work);
			}
			else
			{
				/* 
				 * Wait event in the trace.
				 */
				queue_delayed_work(sender->wq, &sender->work,
					TRACE_EVENTS_SENSITIVITY_JIFFIES);
			}
		}
		else if(result < 0)
		{
			/* Event become lost, but do not stop sending session */
			queue_work(sender->wq, &sender->work.work);
		}
		else
		{
			sender->is_first_event = 0;
			/*
			 * Wait a moment when we may send new packet.
			 */
			queue_delayed_work(sender->wq, &sender->work,
				PACKETS_INTERVAL_JIFFIES);
		}
		return;
	break;
	case events_sender_state_starting:
		events_sender_send_trace_mark(sender, &state,
			TRACE_SERVER_MSG_MARK_SESSION_BEGIN);
		if(sender->is_first_event)
		{
			events_sender_send_trace_mark(sender, &state,
				TRACE_SERVER_MSG_MARK_TRACE_BEGIN);
		}
		queue_delayed_work(sender->wq, &sender->work,
				PACKETS_INTERVAL_JIFFIES);
		return;
	break;
	case events_sender_state_stopping:
		if(state.is_terminated)
		{
			events_sender_send_trace_mark(sender, &state,
				TRACE_SERVER_MSG_MARK_TRACE_END);
		}
		events_sender_send_trace_mark(sender, &state,
			TRACE_SERVER_MSG_MARK_SESSION_END);
		
		pr_info("Stop to send trace.");
		return;
	default:
		return;
	}
}


static int events_sender_init(struct events_sender* sender,
	struct server_trace_events* events)
{
	int result;
	
	result = sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP,
		&sender->clientsocket);
	if(result)
	{
		pr_err("Failed to create client socket.");
		return result;
	}

	sender->wq = create_singlethread_workqueue("sendtrace"); 
	if (!sender->wq){
		pr_err("Failed to create workqueue for sending trace.");
		sock_release(sender->clientsocket);
		return -ENOMEM;
	}

	sender->events = events;
	sender->is_first_event = 1;
		
	sender->state.type = events_sender_state_ready;
	sender->state.is_terminated = 0;
	//sender->state.is_started = 0;
	spin_lock_init(&sender->lock);
	
	sender->seq = 0;

	INIT_DELAYED_WORK(&sender->work, &events_sender_work);
	
	init_waitqueue_head(&sender->stop_waiter);
	
	return 0;
}

/*
 * Destroy events sender.
 * 
 * NOTE: May be called only in READY state.
 */
static void events_sender_destroy(struct events_sender* sender)
{
	BUG_ON(sender->state.type != events_sender_state_ready);
	
	/* Just in case */
    cancel_delayed_work(&sender->work);
    cancel_work_sync(&sender->work.work);
	
	flush_workqueue(sender->wq);
    destroy_workqueue(sender->wq);
	
	sock_release(sender->clientsocket);
	
	sender->state.type = events_sender_state_invalid;
}

/* 
 * Start to send events to the client.
 * 
 * May be executed in atomic context.
 */
static int events_sender_start(struct events_sender* sender,
	__be32 client_addr,
	__be16 client_port)
{
	int result = 0;
	unsigned long flags;
	spin_lock_irqsave(&sender->lock, flags);
	
	if(sender->state.is_terminated)
	{
		pr_err("No commands are expected after terminate command");
		result = -EINVAL;
		goto out;
	}
	
	if(sender->state.type != events_sender_state_ready)
	{
		pr_info("Ignore START command while sender has (another)client.");
		goto out;
	}
	
	sender->state.client_addr = client_addr;
	sender->state.client_port = client_port;
		
	queue_work(sender->wq, &sender->work.work);
				
	sender->state.type = events_sender_state_starting;

out:
	spin_unlock_irqrestore(&sender->lock, flags);
	
	pr_info("Start to send trace.");

	return result;
}

static void events_sender_stop(struct events_sender* sender)
{
	unsigned long flags;
	
	spin_lock_irqsave(&sender->lock, flags);

	if(sender->state.is_terminated)
	{
		pr_err("No commands are expected after terminate command");
		goto out;
	}
	
	if(sender->state.type != events_sender_state_send)
	{
		pr_info("Ignore STOP command when sender do not send trace.");
		goto out;
	}
	
	sender->state.type = events_sender_state_stopping;
	pr_info("Stop to send trace events.");	

out:
	spin_unlock_irqrestore(&sender->lock, flags);
	return;
}

/* 
 * Send rest of messages to the client and then send EOF message.
 * 
 * May not be executed in atomic context.
 */
static void events_sender_terminate(struct events_sender* sender)
{
	unsigned long flags;
	
	spin_lock_irqsave(&sender->lock, flags);

	if(sender->state.is_terminated)
	{
		pr_err("No commands are expected after terminate command");
		goto out;
	}

	sender->state.is_terminated = 1;
out:	
	spin_unlock_irqrestore(&sender->lock, flags);
	
	return;
}

/*
 * Helper for the next function.
 * 
 * Returns non-zero if events sender is in READY state, zero otherwise.
 */
static int events_sender_is_stopped(struct events_sender* sender)
{
	int result;
	unsigned long flags;
	
	spin_lock_irqsave(&sender->lock, flags);
	result = (sender->state.type == events_sender_state_ready);
	spin_unlock_irqrestore(&sender->lock, flags);
	
	return result;
}

/*
 * Wait until events sender stops to send any packet and go into
 * READY state.
 * 
 * This function is intended to use before destroying events sender.
 */
static void events_sender_wait_stop(struct events_sender* sender)
{
	wait_event_killable(sender->stop_waiter, events_sender_is_stopped(sender));
	flush_work(&sender->work.work);
}
//***********************Protocol implementation**********************//
struct port_listener
{
	struct socket *udpsocket;
	/* Object which implements commands recieved by listener */
	struct events_sender* sender;
};

static void port_listener_cb_data(struct sock* sk, int bytes)
{
	struct port_listener* listener = (struct port_listener*)sk->sk_user_data;
	
	__be32 sender_addr;
	__be16 sender_port;
	struct trace_client_msg* msg;
	int msg_len;
	
	struct sk_buff *skb = NULL;
	
	skb = skb_dequeue(&sk->sk_receive_queue);
	if(skb == NULL)
	{
		pr_info("Failed to extract recieved skb.");
		return;
	}

	if(ip_hdr(skb)->protocol != IPPROTO_UDP)
	{
		pr_info("Ignore non-UDP packets.");
		goto out;
	}
	sender_addr = ip_hdr(skb)->saddr;
	sender_port = udp_hdr(skb)->source;
	
	pr_info("Sender: %x, port: %hu", ntohl(sender_addr), ntohs(sender_port));
	
	msg = (typeof(msg))(skb->data + sizeof(struct udphdr));
	msg_len = skb->len - sizeof(struct udphdr);
	
	if(msg_len < sizeof(*msg))
	{
		pr_info("Ignore incorrect request.");
		goto out;
	}
	
	switch(msg->type)
	{
	case TRACE_CLIENT_MSG_TYPE_START:
		events_sender_start(listener->sender, sender_addr, sender_port);
	break;
	case TRACE_CLIENT_MSG_TYPE_STOP:
		events_sender_stop(listener->sender);
	break;
	default:
		pr_info("Ignore incorrect request of type %d.", (int)msg->type);
		goto out;
	}
out:	
	kfree_skb(skb);
}

static int port_listener_init(struct port_listener* listener,
	int16_t port, struct events_sender* sender)
{
	struct sockaddr_in server;
	int result;

	result = sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP,
		&listener->udpsocket);
	
	if (result < 0) {
		pr_err("server: Error creating udpsocket.");
		return result;
	}
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	
	result = kernel_bind(listener->udpsocket,
		(struct sockaddr*)&server, sizeof(server));
	if (result)
	{
		pr_err("Failed to bind server socket.");
		sock_release(listener->udpsocket);
		return result;
	}

	listener->sender = sender;
	listener->udpsocket->sk->sk_user_data = listener;
	/* Barrier before publish callback for recieving messages */
	smp_wmb();
	listener->udpsocket->sk->sk_data_ready = port_listener_cb_data;
	
	return 0;
	
}

static void port_listener_destroy(struct port_listener* listener)
{
	sock_release(listener->udpsocket);
}

/* Concrete objects for module */
static struct server_trace_events events;
static struct events_generator generator;
static struct events_sender sender;
static struct port_listener listener;

static struct dentry *control_dir;

static int __init server_init( void )
{
	int result;
	
	result = server_trace_events_init(&events);
	if(result) goto events_err;
	
	control_dir = debugfs_create_dir(THIS_MODULE->name, NULL);
	if(control_dir == NULL)
	{
		result = -EINVAL;
		goto control_dir_err;
	}
	
	result = events_generator_init(&generator, &events, control_dir);
	if(result) goto generator_err;
	
	result = events_sender_init(&sender, &events);
	if(result) goto sender_err;
	
	result = port_listener_init(&listener, server_port, &sender);
	if(result) goto listener_err;
	
	return 0;

listener_err:
	events_sender_destroy(&sender);
sender_err:
	events_generator_destroy(&generator);
generator_err:
	debugfs_remove(control_dir);
control_dir_err:
	server_trace_events_destroy(&events);
events_err:

	return result;
}

static void __exit server_exit( void )
{
	port_listener_destroy(&listener);

	events_sender_terminate(&sender);
	events_sender_wait_stop(&sender);
	events_sender_destroy(&sender);

	events_generator_destroy(&generator);

	debugfs_remove(control_dir);

	server_trace_events_destroy(&events);
}

module_init(server_init);
module_exit(server_exit);
MODULE_LICENSE("GPL");
