//
// Server Implementation Model: https://www.toptal.com/software/guide-to-multi-processing-network-server-models
// General theory we used: ...
//

#include "config.h"
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <factory/factory.h>
#include <stdbool.h>

#include "utility/new.h"
#include "utility/log/log.h"
#include "server.h"

void init_thread_attr(pthread_attr_t *attr, int PTHREAD_STACK_SIZE) {

    int threadReturnValue = SUCCESS;
    threadReturnValue = pthread_attr_init(attr);

    threadReturnValue = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
    //threadReturnValue = pthread_attr_setstacksize(attr, PTHREAD_STACK_SIZE);

    return;
}

int init_socket(int *sock_fd, server_ctx *sctx, int sock_type, int protocol, struct sockaddr_in *server_addr) {

    struct sockaddr_in host;
    int ret;
    int true_ = 1;

    *sock_fd = socket(AF_INET, sock_type, protocol);
    if (*sock_fd == INVALID) {
        close(*sock_fd);
        return SOCK_FAILED;
    }

    memset(&host, MEM_SET, sizeof(host));
    host.sin_family = AF_INET;
    host.sin_port = htons(sctx->port);
    host.sin_addr.s_addr = inet_addr(sctx->ip);

    ret = setsockopt(*sock_fd, SOL_SOCKET, SO_REUSEADDR, &true_, sizeof(int));
    if (ret != 0) {
        LM_PRINT(sctx->log_level, "%s", "setting reuse_address option failed\n");
        close(*sock_fd);
        return SOCK_FAILED;
    }
    const int opt_one = 1;
    if (setsockopt(*sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt_one, sizeof opt_one) == -1) {

        LM_PRINT(sctx->log_level, "%s", "setting reuse_address option failed \n");
        close(*sock_fd);
        return SOCK_FAILED;
    }

    ret = bind(*sock_fd, (struct sockaddr *) &host, sizeof(host));
    if (ret != SUCCESS) {
        LM_PRINT(sctx->log_level, "%s", "error in binding socket\n");

        if (sctx->ip != NULL) {
            if (strlen(sctx->ip) > 0)
                LM_PRINT(sctx->log_level, "error binding socket with ip: %s\n", sctx->ip);
        }
        close(*sock_fd);
        return SOCK_FAILED;
    }

    server_addr = &host;
    LM_PRINT(sctx->log_level, "%s", "Socket creation successful \n");

    return SUCCESS;
}

int get_th_indx(thread_ctx *tctx) {

    for (int indx = 0; indx < tctx->thread_count; ++indx) {
        if (tctx->thread_id[indx] == pthread_self()){
           return indx;
        }

    }
    return -1;
}

void *recv_func(void *ptr) {

    server_app *app = (server_app*)ptr;
    thread_ctx *th_ctx = app->srv_ctx->th_ctx;

    int thread_indx = get_th_indx(th_ctx);
    if(thread_indx == INVALID || thread_indx >= th_ctx->thread_count) {
        LM_PRINT(app->srv_ctx->log_level , "exiting thread, thread id: %u\n", pthread_self());
        pthread_exit(NULL);
    }
    th_ctx->thread_term[thread_indx] = false;
    th_ctx->thread_guard = true;

    int sock_fd;
    struct sockaddr_in server_addr;
    int ret = init_socket(&sock_fd, app->srv_ctx, SOCK_DGRAM, udp, &server_addr);

    if (ret != SUCCESS) {
        LM_PRINT(app->srv_ctx->log_level , "%s", "[ERROR] scoket creation failed\n");
        return NULL;
    }

    // server_app's thread_ctx initialization
    thread_ctx *tctx = calloc(1, sizeof(thread_ctx));
    app->initialize_thd_ctx(app->srv_ctx, app->app_ctx, tctx);

    while (!app->srv_term) {

        net_element *net_node = new(NetElement, app);
        struct msghdr *msg = calloc(1, sizeof(struct msghdr));
        struct iovec *iov = calloc(1, sizeof(struct iovec));

        net_node->thd_ctx = tctx;

        msg->msg_name           = &server_addr;
        msg->msg_namelen        = sizeof(struct sockaddr_in);
        msg->msg_iov            = iov;
        msg->msg_iov->iov_base  = net_node->rcv_buf;
        msg->msg_iov->iov_len   = MAX_PACKET;
        msg->msg_iovlen         = 1;

        net_node->versatile = msg;
        net_node->peer      = &server_addr;
        net_node->sock_fd   = sock_fd;

        net_node->rcv_buf_len = recvmsg(sock_fd, net_node->versatile, 0);

        // insert transaction start time in 'tlog'
        struct timeval start;
        gettimeofday(&start, NULL);
        net_node->tlog_info->time_req = start.tv_sec + (start.tv_usec / 1000000.0);

        if (net_node->rcv_buf_len <= 0) {
            LM_DEBUG(net_node->srv_ctx->log_level ,net_node->tlog_info->ctx, "buffer: %s \n","Empty buffer received" );
            free(net_node->rcv_buf);
            delete(net_node);
            continue;
        }

        app->handle_request(net_node);
    }

    // server_app's thread_ctx initialization
    app->destroy_thd_ctx(tctx);
    free(tctx);

    close(sock_fd);
    th_ctx->thread_term[thread_indx] = true;
    return SUCCESS;
}

int setup_recievers(server_app *app){

    int thread_count = app->srv_ctx->th_ctx->thread_count;
    pthread_attr_t attr;

    init_thread_attr(&attr, 0);

    for(int indx = 0; indx < thread_count ; ++indx){

        app->srv_ctx->th_ctx->thread_guard = false;
        pthread_create(&app->srv_ctx->th_ctx->thread_id[indx], &attr, &recv_func, (void*)app);
        while(app->srv_ctx->th_ctx->thread_guard != true)
            usleep(10);
    }

    return 0;
}

// 'server' implementation
int run_server(server_app *app) {

    if (app->initialize) {
        app->initialize(app->srv_ctx, app->app_ctx);
    }
    else {
        LM_PRINT(app->srv_ctx->log_level, "%s", "[ERROR] DNS Initializer cannot be null \n");
        return -1;
    }

    if (app->srv_ctx->protocol == udp) {
        LM_PRINT(app->srv_ctx->log_level, "\"[INFO] creating udp Server  \n");
        setup_recievers(app);
    }

//socket should be closed in signal handler

    return 0;
}
