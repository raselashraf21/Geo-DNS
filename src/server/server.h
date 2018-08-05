#ifndef GEODNS_SERVER_H
#define GEODNS_SERVER_H


#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "utility/dns_log.h"

#define SUCCESS 0

#define INVALID -1
#define SOCK_FAILED INVALID

#define MEM_SET 0

#define SLEEP_INTERVAL_MIN 10
////////////////////////////////

typedef enum _ptoto_type{
    tcp = 6,
    udp = 17
} proto_type;

typedef enum _server_type {
    dns_server = 0,
    cmd_server =  1
} server_type;

typedef struct _thread_server_ctx {
} thread_server_ctx;

typedef struct _thread_ctx {
    thread_server_ctx thread_srv_ctx;
    void *thread_app_ctx;               // not owner
    int thread_guard;
    int thread_count;
    int *thread_term;
    pthread_t *thread_id;
} thread_ctx;

typedef struct _server_ctx {
    thread_ctx *th_ctx;
    server_type srv_type;
    proto_type protocol;
    char *ip;           // should be malloced, not string
    int port;
    void *log_conf;     // not owner
    int log_level;
} server_ctx;

typedef void initialize_server_app(server_ctx *srv_ctx, void *app_ctx);

typedef void destroy_server_app(void *app_ctx);

typedef char *get_buffer_pointer(int len);

typedef void *request_handler(void *req_ctx);    // Mainly, req_ctx type is 'net_element'

typedef void initialize_thread_ctx(server_ctx *srv_ctx, void *app_ctx, thread_ctx *thd_ctx); // Mainly for initialization of thread_ctx's thread_app_ctx.

typedef void destroy_thread_ctx(thread_ctx *thd_ctx);    // Mainly for de-initialization of thread_ctx's thread_app_ctx.



/*!
 * WARNING: Don't manually create 'net_element'.
 * USAGE:
 *          net_element *net_node = new(NetElement, app);
 *          ...
 *          ...
 *          delete(net_node)
 *
 */
typedef struct _net_element {
    const void * class;             /* Must be first, must not use it */
    pthread_t id;
    int th_status;
    int sock_fd;
    int rcv_type;
    struct sockaddr_in *peer;
    socklen_t peer_len;

    transaction_log *tlog_info;

    server_ctx *srv_ctx;    // not owner;
    thread_ctx *thd_ctx;    // not owner;
    void *app_ctx;          // not owner
    char *rcv_buf;          // not owner
    ssize_t rcv_buf_len;

    void *versatile;        // implementation specific ownership
    request_handler *handle_buf;        // TODO : should not be here
} net_element;
extern const void *NetElement;

typedef struct _server_app {
    int srv_term;
    // run_server configuration
    server_ctx *srv_ctx;    // owner

    // server_app context
    void *app_ctx;          // not owner

    // server_app's callback functions
    initialize_server_app *initialize;
    destroy_server_app *destroy;
    get_buffer_pointer *get_buffer;
    request_handler *handle_request;

    initialize_thread_ctx *initialize_thd_ctx;
    destroy_thread_ctx *destroy_thd_ctx;
} server_app;

// Must be implemented by 'server'
void init_thread_attr(pthread_attr_t *attr, int PTHREAD_STACK_SIZE);
int init_socket(int *sock_fd, server_ctx *sctx, int sock_type, int protocol, struct sockaddr_in *server_addr);
int run_server(server_app *app);

#endif // GEODNS_SERVER_H
