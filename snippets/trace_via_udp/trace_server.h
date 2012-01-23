/*
 * Common definitions for server and client-server protocol.
 * 
 * NOTE: Protocol messages may be sent from one machine and read on
 * another. Because architecture of sender and client may differ,
 * message types should be declared in architecture-independed way:
 *  - types sizes should be architecture-independed
 *  - types alignment should be architecture-independed(64bits types!)
 *  - types bytes ordering should be architecture-independed(better - network)
 * 
 * NOTE: __be* are types with network bytes ordering.
 */

#ifndef TRACE_SERVER_H
#define TRACE_SERVER_H

#include <linux/types.h>

/* Server will be running on this port by default */
#ifndef TRACE_SERVER_PORT
#define TRACE_SERVER_PORT 5556
#endif
/* 
 * Maximum length of message sent from the server to the client
 */
#define TRACE_SERVER_MSG_LEN_MAX 1500

#ifdef __KERNEL__
/*
 * Needed definitions in the kernel space.
 */

/* define htonl and others */
#include <linux/in.h>

#else /* __KERNEL__ */

/*
 * Needed definitions in the user space.
 */

/* define ntohs and others */
#include <arpa/inet.h>

#endif /* __KERNEL__ */

/* 
 * Linux kernel before 2.6.36 doesnt't contain 64bit type which suitable
 * for use in network message(see notes above).
 * So we define our one for timestamps.
 */
typedef struct {__be32 high, low;} __attribute__((aligned(32))) timestamp_nt;
/* Helpers for write timestamps to messages and extract them */
static inline void timestamp_nt_set(timestamp_nt *ts_nt, uint64_t ts)
{
    uint32_t ts_low = (uint32_t)(ts & 0xffffffff);
    uint32_t ts_high = (uint32_t)(ts >> 32);
    ts_nt->low = htonl(ts_low);
    ts_nt->high = htonl(ts_high);
}

static inline uint64_t timestamp_nt_get(timestamp_nt *ts_nt)
{
    return ntohl(ts_nt->low) + (((uint64_t)ntohl(ts_nt->high)) << 32);
}

/* Base type of messages from the trace server */
struct trace_server_msg
{
    __be32 seq;
    __u8 type;
    // May be used for determine precise size of data
    char end_struct[0];
};


/* Message contains packet of events */
#define TRACE_SERVER_MSG_TYPE_PACKET 1
/* Message contains some trace mark */
#define TRACE_SERVER_MSG_TYPE_MARK 2

/* Trace event will be transmitted via net in this form */
struct trace_event
{
    timestamp_nt timestamp;
    /* Size of the context */
    __be16 context_size;
    /* 
     * Event context as array of bytes.
     */
    __u8 context[0];
};

/* Message of type packet */
struct trace_server_msg_packet
{
    struct trace_server_msg base;
    
    /* In the simple implementation packet contains only one event */
    struct trace_event event;
};

/* 
 * Event marks.
 * 
 * May be used by the client for determine sequential ranges of
 * event packets, which was sent.
 * (But may be lost due to the net or other reasons.)
 */

/* 
 * Mark beginning of the trace.
 * 
 * This mark is sent before first event in the trace.
 */
#define TRACE_SERVER_MSG_MARK_TRACE_BEGIN      1
/* 
 * Mark ending of the trace.
 * 
 * This mark is sent after last event in the trace.
 */
#define TRACE_SERVER_MSG_MARK_TRACE_END        2
/*
 * Mark beginning of the sending session.
 * 
 * This mark is sent before any message in the session.
 * (And before TRACE_BEGIN mark, if it is).
 */
#define TRACE_SERVER_MSG_MARK_SESSION_BEGIN    3
/* 
 * Mark ending of the sending session.
 * 
 * This mark is sent after any message in the session.
 * (And after TRACE_END mark, if it is).
 */
#define TRACE_SERVER_MSG_MARK_SESSION_END      4


struct trace_server_msg_mark
{
    struct trace_server_msg base;
    __u8 mark;
    // May be used for determine precise size of data
    char end_struct[0];
};

/* Type of message to trace server from client */
struct trace_client_msg
{
    __u8 type;
};

/* 
 * Start message, after which server will sent trace packets to
 * the client.
 */
#define TRACE_CLIENT_MSG_TYPE_START 1
/* 
 * Stop message, after which server will not sent trace packets to
 * the client.
 * 
 * NOTE: SESSION_END mark is sent after this message for client
 * can determine whether it recieve all messages sent before stopping.
 */
#define TRACE_CLIENT_MSG_TYPE_STOP 2

#endif /* TRACE_SERVER_H */