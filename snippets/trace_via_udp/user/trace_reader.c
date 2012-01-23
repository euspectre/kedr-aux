#include "trace_server.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include <assert.h>

#include <getopt.h>

/* Default client port */
#ifndef CLIENT_PORT
#define CLIENT_PORT 9999
#endif

/* Default server address */
#ifndef SERVER_ADDRESS
#define SERVER_ADDRESS "127.0.0.1"
#endif

/* Usefull macros for type convertion */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({                      \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
         (type *)( (char *)__mptr - offsetof(type,member) );})


void print_usage(const char* command)
{
	fprintf(stderr, "Usage:\n\n");
	fprintf(stderr, "  %s [option ...]\n\n", command);
	fprintf(stderr, "Where option may be:\n\n");
	
	fprintf(stderr, "  --server-address <net-address>\n");
	fprintf(stderr, "      Address of the server in numbers-and-dots notation.\n");
	fprintf(stderr, "    If this option is not supplied, "
		"server address assumed to be \"%s\".\n\n", SERVER_ADDRESS);

	fprintf(stderr, "  --server-port <port>\n");
	fprintf(stderr, "      Port of the server.");
	fprintf(stderr, "    If this option is not supplied, "
		"server port assumed to be %d.\n\n", (int)TRACE_SERVER_PORT);

	fprintf(stderr, "  --client-port <port>\n");
	fprintf(stderr, "      Port of the client.\n");
	fprintf(stderr, "    If this option is not supplied, "
		"server port assumed to be %d.\n\n", (int)CLIENT_PORT);

	fprintf(stderr, "  --events-limit <n>\n");
	fprintf(stderr, "      Limit of events recieved. After client receive"
		"this number of events, \n    it sends STOP command to the server.\n");
	fprintf(stderr, "    Note, that because of transmitted time, real number "
		"of received events may exceed this limit.\n");
	fprintf(stderr, "    If this option is not supplied or n is non-positive,\n "
		"    client do not send STOP command to the server in any case.\n\n");

	fprintf(stderr, "  -h, --help\n");
	fprintf(stderr, "      Print this help.\n\n");
}

int parse_arguments(int argc, char** argv,
	const char** server_address, unsigned short* server_port,
	unsigned short* client_port,
	int* events_limit)
{
#define SERVER_ADDRESS_OPT 	1
#define SERVER_PORT_OPT		2
#define CLIENT_PORT_OPT		3
#define EVENTS_LIMIT_OPT	4
#define HELP_OPT			'h'
	// Available program's options
	static const char short_options[] = "h";
	static struct option long_options[] = {
		{"server-address", 1, 0, SERVER_ADDRESS_OPT},
		{"server-port", 1, 0, SERVER_PORT_OPT},
		{"client-port", 1, 0, CLIENT_PORT_OPT},
		{"events-limit", 1, 0, EVENTS_LIMIT_OPT},
		{"help", 1, 0, HELP_OPT},
		{0, 0, 0, 0}
	};
	int opt;

	/* Default values */
	*server_address = SERVER_ADDRESS;
	*server_port = TRACE_SERVER_PORT;
	*client_port = CLIENT_PORT;
	*events_limit = 0;

	for(opt = getopt_long(argc, argv, short_options, long_options, NULL);
		opt != -1;
		opt = getopt_long(argc, argv, short_options, long_options, NULL))
    {
        long value;
        char* endptr;
        switch(opt)
        {
        case '?':
            //error in options
            if(optind < argc)
                fprintf(stderr, "Unknown option '%s'.\n"
                    "Execute '%s -h' to see the description of program's parameters.",
                    argv[0], argv[optind]);
            return -1;
        case SERVER_ADDRESS_OPT:
            *server_address = optarg;
            break;
        case SERVER_PORT_OPT:
            endptr = optarg + strlen(optarg);
            value = strtol(optarg, &endptr, 0);
            if((*endptr != '\0') || (value <= 0) || (value > 0xffff))
            {
				fprintf(stderr, "Incorrect port number: %s", optarg);
				return -1;
            }
            *server_port = (unsigned short)value;
            break;
        case CLIENT_PORT_OPT:
            endptr = optarg + strlen(optarg);
            value = strtol(optarg, &endptr, 0);
            if((*endptr != '\0') || (value <= 0) || (value > 0xffff))
            {
				fprintf(stderr, "Incorrect port number: %s", optarg);
				return -1;
            }
            *client_port = (unsigned short)value;
            break;
        case EVENTS_LIMIT_OPT:
            endptr = optarg + strlen(optarg);
            value = strtol(optarg, &endptr, 0);
            if(*endptr != '\0')
            {
				fprintf(stderr, "Incorrect events limit: %s", optarg);
				return -1;
            }
            *events_limit = (value > 0) ? (int)value : 0;
            break;
        case HELP_OPT:
            print_usage(argv[0]);
            return 1;
        default:
            //strange result from getopt_long
            fprintf(stderr, "getopt_long return strange result: %d.", opt);
            return -1;
        }
    }
    return 0;
}

struct trace_client
{
    /* Socket for send messages and listen server. */
    int sock;
    /*
     * For unknown reason, connect() for udp socket forces recv() fail.
     * 
     * So do not use connect(), and store server address and port for
     * sending via sendto().
     */
    in_addr_t server_addr;
    in_port_t server_port;
};

static int trace_client_init(struct trace_client* client,
    unsigned short client_port)
{
    struct sockaddr_in receivesocket;
    
    int result;

    client->sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client->sock < 0)
    {
        perror("Failed to create client socket");
        return -1;
    }

    memset(&receivesocket, 0, sizeof(receivesocket));  
    receivesocket.sin_family = AF_INET; 
    receivesocket.sin_addr.s_addr = htonl(INADDR_ANY);
    receivesocket.sin_port = htons(client_port);

    result = bind(client->sock, (struct sockaddr*)&receivesocket,
        sizeof(receivesocket));
    if(result < 0)
    {
        perror("Failed to bind client socket");
        return -1;
    }

    return 0;
}

static void trace_client_destroy(struct trace_client* client)
{
    close(client->sock);
}

static int trace_client_connect(struct trace_client* client,
    const char* server_addr, unsigned short server_port)
{
    client->server_addr = inet_addr(server_addr);
    client->server_port = htons(server_port);
    
    if(client->server_addr == INADDR_NONE)
    {
        fprintf(stderr, "Incorrect format of server address: %s.\n",
            server_addr);
    }

    return 0;
}

static int trace_client_send_command(struct trace_client* client,
    int command)
{
    int result;
    struct trace_client_msg client_msg;
    
    client_msg.type = (__u8)command;
    
    struct sockaddr_in sendsocket;

    memset(&sendsocket, 0, sizeof(sendsocket));
    sendsocket.sin_family = AF_INET;
    sendsocket.sin_addr.s_addr = client->server_addr;
    sendsocket.sin_port = client->server_port;
    
    result = sendto(client->sock, &client_msg, sizeof(client_msg), 0,
        (struct sockaddr *) &sendsocket, sizeof(sendsocket));
    if(result < 0)
    {
        perror("Failed to send command");
        return -1;
    }
    
    return 0;
}

/*
 * Return 1 if nothing for receive(e.g., non-blocking mode).
 * 
 * On successfull call, 'server_msg' and 'server_msg_len' will be set.
 * 'server_msg' should be freed when no longer needed.
 */
static int trace_client_receive_msg(struct trace_client* client,
    struct trace_server_msg** server_msg, size_t* server_msg_len)
{
    int result;
    
    struct trace_server_msg* server_msg_buf =
        malloc(TRACE_SERVER_MSG_LEN_MAX);
    if(server_msg_buf == NULL)
    {
        fprintf(stderr, "Failed to allocate buffer for receiving message.\n");
        return -1;
    }
    
    result = recv(client->sock, server_msg_buf,
        TRACE_SERVER_MSG_LEN_MAX, 0);
    /*result = recvfrom(client->sock, server_msg_buf,
        TRACE_SERVER_MSG_LEN_MAX, 0, NULL, NULL);*/
    if(result < 0)
    {
        perror("Failed to receive message");
        goto err;
    }
    if(result > TRACE_SERVER_MSG_LEN_MAX)
    {
        fprintf(stderr, "Message received exceed maximum length.\n");
        goto err;
    }
    
    if(result < offsetof(struct trace_server_msg, end_struct))
    {
        fprintf(stderr, "Received message length is too little.\n");
        goto err;
    }


    *server_msg = server_msg_buf;
    if(server_msg_len)
        *server_msg_len = result;
    
    return 0;

err:
    free(server_msg_buf);
    return -1;
}

/* Parse message from server */

/*
 * If given message contains mark, set 'mark' to value of the mark,
 * and return non-zero value. Otherwise return 0.
 */
static int is_mark(struct trace_server_msg* server_msg,
	size_t server_msg_len, unsigned char* mark)
{
	if(server_msg->type == TRACE_SERVER_MSG_TYPE_MARK)
	{
		assert(server_msg_len >= offsetof(struct trace_server_msg_mark, end_struct));

		*mark = ((struct trace_server_msg_mark*)(server_msg))->mark;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 * If given message contains trace packet, extracts its parameters into
 * 'event_context', 'event_context_size', 'timestamp' and return
 * non-zero value. Otherwise return 0.
 */
static int is_trace_packet(struct trace_server_msg* server_msg,
	size_t server_msg_len, char** event_context,
	__u16* event_context_size, __u64* timestamp)
{
	if(server_msg->type == TRACE_SERVER_MSG_TYPE_PACKET)
	{
		struct trace_server_msg_packet* msg_packet = 
			(struct trace_server_msg_packet*)server_msg;

		assert(server_msg_len >=
			offsetof(struct trace_server_msg_packet, event.context));

		__u16 __event_context_size = ntohs(msg_packet->event.context_size);
		
		assert(server_msg_len >=
			offsetof(struct trace_server_msg_packet, event.context)
				+ __event_context_size);
		
		*event_context = msg_packet->event.context;
		
		*event_context_size = __event_context_size;
		*timestamp = timestamp_nt_get(&msg_packet->event.timestamp);
		return 1;
	}
	else
	{
		return 0;
	}
}


int main(int argc, char **argv)
{
    int result;
    struct trace_client client;

    int events_limit;
    const char* server_address;
    unsigned short server_port;
    unsigned short client_port;
    
    int events_count = 0;

    result = parse_arguments(argc, argv, &server_address,
		&server_port, &client_port, &events_limit);
	if(result) return result;
	
	if(events_limit)
	{
		printf("Client-server session is limited by %d events.\n",
			events_limit);
	}
    
    result = trace_client_init(&client, client_port);
    if(result) return result;
    
    result = trace_client_connect(&client, server_address,
        server_port);
    
    if(result)
    {
        trace_client_destroy(&client);
        return result;
    }

    result = trace_client_send_command(&client, TRACE_CLIENT_MSG_TYPE_START);
    if(result) goto err;
    
    struct trace_server_msg* server_msg;
    size_t server_msg_len;
    unsigned char mark;
    

    /* First message should contain SESSION_BEGIN mark */
    result = trace_client_receive_msg(&client, &server_msg, &server_msg_len);
    if(result) goto err;
    
    if(!is_mark(server_msg, server_msg_len, &mark)
		|| (mark != TRACE_SERVER_MSG_MARK_SESSION_BEGIN))
	{
		fprintf(stderr, "First packet from the trace server should"
			"contain SESSION_BEGIN mark.\n");
        free(server_msg);
        goto err;
	}
	printf("Receive session begins.\n");
	free(server_msg);
    
	/* Read futher trace events in cycle */
    for(result = trace_client_receive_msg(&client, &server_msg, &server_msg_len);
		result == 0;
		result = trace_client_receive_msg(&client, &server_msg, &server_msg_len)
	)
    {
        char* event_context;
        __u16 event_context_size;
        __u64 timestamp;
        
        if(is_trace_packet(server_msg, server_msg_len,
			&event_context, &event_context_size, &timestamp))
		{
			int ts_sec = (timestamp / 1000000000L);
			int ts_msec = (timestamp % 1000000000L) / 1000;
        
			printf("(%d.%d): size=%d, content=%.*s\n", ts_sec, ts_msec,
				(int)event_context_size, (int)event_context_size, event_context);
			free(server_msg);
			
			events_count++;
			if(events_limit && (events_count == events_limit))
			{
				result = trace_client_send_command(&client, TRACE_CLIENT_MSG_TYPE_STOP);
				if(result) goto err;
				printf("Send STOP command to the server.\n");
			}
		}
        else if(is_mark(server_msg, server_msg_len, &mark))
        {
			free(server_msg);
			if(mark == TRACE_SERVER_MSG_MARK_TRACE_BEGIN)
			{
				printf("Trace begins.\n");
			}
			else if(mark == TRACE_SERVER_MSG_MARK_TRACE_END)
			{
				printf("Trace ends.\n");
			}
			else break;
        }
        else
        {
			printf("Incorrect message format.\n");
			free(server_msg);
			goto err;
        }
    }
    if(result) goto err;

    /* SESSION_END */
	if(mark != TRACE_SERVER_MSG_MARK_SESSION_END)
	{
   		fprintf(stderr, "Unexpected mark while receiving trace: %d\n",
			(int)mark);
        goto err;
	}
    
    printf("Receive session ends.\n");

    trace_client_destroy(&client);

    return 0;

err:
    trace_client_destroy(&client);
    return -1;
}
