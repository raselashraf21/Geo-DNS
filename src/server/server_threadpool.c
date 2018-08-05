#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/time.h>

#include "utility/new.h"
#include "utility/log/log.h"
#include "utility/dns_log.h"
#include "config.h"
#include "server/server.h"
#include "server/thread_pool.h"
#include "thread_pool.h"

static void _init_thread_attr(pthread_attr_t *attr, int PTHREAD_STACK_SIZE) {

    int threadReturnValue = SUCCESS;
    threadReturnValue = pthread_attr_init(attr);

    threadReturnValue = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
    //threadReturnValue = pthread_attr_setstacksize(attr, PTHREAD_STACK_SIZE);

    return;
}

static int _init_socket_mthread(char *log_src, int *sock_fd, int port, int sock_type, int protocol, char *host_ip,
                                int recv_timeout) {

    struct sockaddr_in host;
    int ret;
//    struct linger optval;
//    struct timeval tv;
    int true_ = 1;

//    opt_len = sizeof(optval);
//    optval.l_onoff = LINGER_ON;
//    optval.l_linger = LINGER_OFF;
//    tv.tv_sec = 0;
//    tv.tv_usec = RECV_TIMEOUT;

    *sock_fd = socket(AF_INET, sock_type, protocol);
    if (*sock_fd == INVALID) {
        close(*sock_fd);
        return SOCK_FAILED;
    }

    memset(&host, MEM_SET, sizeof(host));
    host.sin_family = AF_INET;
    host.sin_port = htons(port);
    host.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(*sock_fd, (struct sockaddr *) &host, sizeof(host));
    if (ret != SUCCESS) {
        printf("error in binding socket\n");

        if (host_ip != NULL) {
            if (strlen(host_ip) > 0)
                printf("error binding socket with ip: %s\n", host_ip);
        }
        close(*sock_fd);
        return SOCK_FAILED;
    }

    ret = setsockopt(*sock_fd, SOL_SOCKET, SO_REUSEADDR, &true_, sizeof(int));
    if (ret != 0) {
        printf(" setting reuse_address option failed \n");
        close(*sock_fd);
        return SOCK_FAILED;
    }

    printf("Socket creation successful \n");
    return SUCCESS;
}

// utility functions
void free_net_node(net_element **net_node) {
    // TODO
}


//net_element * create_net_info(server_app *app){
//
//    net_element *net_node = new(NetElement, app);
//
//    net_node->srv_ctx = app->srv_ctx;
//    net_node->app_ctx = app->app_ctx;
//    net_node->rcv_buf = app->get_buffer(MAX_PACKET);    // TODO: Null Check
//
//    log_context *ctx = new(LogContext, NULL);
//    net_node->log =new(TransactionLog, NULL);
//    net_node->log->ctx = ctx;
//
//    net_node->th_status = 0;
//    net_node->rcv_type = 0;
//    net_node->peer_len = sizeof(net_node->peer);
//
//}


void *watch_log(void *ptr){

    _queue_node *nlog = (_queue_node *)ptr;
    time_t start_tm, end_tm;
    start_tm = time(NULL);
    long diff_tm =0;



    log_context *ctx = new(LogContext, NULL);
    ctx->log_app = NULL;

    while(1){

        end_tm = time(NULL);


        if( diff_tm = difftime(end_tm, start_tm) > WATCH_LOG_INTERVAL){

            log_error(ctx, "qSize: %u", nlog->qsize);
            start_tm =end_tm;
        }
    sleep(WATCH_LOG_INTERVAL);
    }

}

void create_watch_log(_queue_node *nlog){

    pthread_t tid;
    pthread_attr_t attr;
    _init_thread_attr(&attr, 1024);

    pthread_create(&tid, &attr, &watch_log, nlog);

}

// 'server' implementation
int run_server_t(server_app *app) {

    thread_ctx *tctx = calloc(1, sizeof(thread_ctx));
    if (app->initialize) {                // TODO: NULL Check app's callbacks
        app->initialize(app->srv_ctx, app->app_ctx);

        // server_app's thread_ctx initialization
        app->initialize_thd_ctx(app->srv_ctx, app->app_ctx, tctx);
    }
    else {
        _DEBUG_CHANGE printf("[ERROR] DNS Initializer cannot be null");   // TODO add NULL checker & exception
        return -1;
    }

    int ret;
    int sock_fd;

    if (app->srv_ctx->srv_type == UDP) {
        _DEBUG printf("creating udp run_server ...\n");
        ret = _init_socket_mthread(app->srv_ctx->log_name, &sock_fd, app->srv_ctx->port, SOCK_DGRAM, UDP,
                                   app->srv_ctx->ip, ON);
        if (ret != SUCCESS) {
            _DEBUG_CHANGE printf("scoket creation failed\n");
            return SOCK_FAILED;
        }
    }
    else if (app->srv_ctx->srv_type == TCP) {
        return -1;
    }
    else {
        _DEBUG_CHANGE printf("server type is invalid\n");
        return -1;
    }

    _thread_pool *thread_pool = create_thread_pool(THREAD_NUMBER, QUEUE_SIZE);

    _queue_node *node_log = thread_pool->queue_node;

    void *conf = new(ConfLog, "geodns", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL2);
    create_watch_log(node_log);

    if (thread_pool == NULL) {
        _DEBUG_CHANGE printf("error in creating thread pool\n");
        return -1;
    }



    while (1) {

        net_element *net_node = create_net_info(app);

        net_node->log->ctx->log_app = conf;
        net_node->sock_fd = sock_fd;
        net_node->rcv_buf_len = recvfrom(net_node->sock_fd, net_node->rcv_buf, MAX_PACKET, 0,
                                         (struct sockaddr *) &(net_node->peer), &(net_node->peer_len));

        gettimeofday(&net_node->start, NULL);

        if (net_node->rcv_buf_len <= 0) {
            free(net_node->rcv_buf);    // TODO check free?
            delete(net_node);
            continue;
        }

        net_node->handle_buf = app->handle_request;
        append_job(thread_pool, net_node, NULL);

    }

    close(sock_fd);
    app->destroy(app->app_ctx);

    // server_app's thread_ctx initialization
    app->destroy_thd_ctx(tctx);
    free(tctx);

    // free-up server_app
    free_app((void **)&app);

    printf("Recv thread ended\n");

    return SUCCESS;

//socket should be closed in signal handler
}
