
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "config.h"
#include "utility/new.h"
#include "server/server.h"
#include "dns_packet.h"
#include "dns_app.h"
#include "dns_packet_parser.h"
#include "dns_response.h"
#include "err_checker.h"

#ifndef EXIT_CODE_DNS
    #define EXIT_CODE_DNS 101
#endif

void  __match_proto__(initialize_server_app)
initialize_dns_app(server_ctx *srv_ctx, void *app_ctx) {
    dns_ctx *ctx = app_ctx;
    int status = ctx->lookup_initialize(ctx->lookup_ctx);
    if (status) {   //db doesn't open successfully.
        LM_PRINT(srv_ctx->log_level, "[ERROR] Cannot open lookup db. (status %d)\n", status );
        goto ERROR;
    }

    LM_PRINT(srv_ctx->log_level,"Successfully opened lookup db\n" );

    status = ctx->geoip_initialize(ctx->geoip_ctx);
    if (status) {   //db doesn't open successfully.
        LM_PRINT(srv_ctx->log_level,"[ERROR] Cannot open geoip db. (status %d)\n", status);
        goto ERROR;
    }

    LM_PRINT(srv_ctx->log_level,"Successfully opened geoip db\n");
    LM_PRINT(srv_ctx->log_level,"Successfully initialized dns_app\n");

    return;

    ERROR:
    {
        destroy_dns_app(ctx);
        LM_PRINT(srv_ctx->log_level,"Cannot initialize dns_app\n");
        exit(EXIT_CODE_DNS);
    }
}

void __match_proto__(destroy_server_app)
destroy_dns_app(void *app_ctx) {
    if (app_ctx) {
        int status;
        dns_ctx *dctx = (dns_ctx *) app_ctx;

        if (dctx->lookup_ctx) {
            status = dctx->lookup_destroy(dctx->lookup_ctx);

            LM_PRINT(dctx->log_level,"%sClosed lookup db (%d)\n", status ? "[ERROR] " : "", status);
            dctx->lookup_ctx = NULL;
        }

        if (dctx->geoip_ctx) {
            status = dctx->geoip_destroy(dctx->geoip_ctx);
            LM_PRINT(dctx->log_level,"%sClosed geoip db (%d)\n", status ? "[ERROR] " : "", status);
            dctx->geoip_ctx = NULL;
        }

        LM_PRINT(dctx->log_level,"Successfully closed dns_app\n");
    }
}

char *get_dns_buffer(int len) {
    dns_info *info = create_info();     // TODO NULL Check
    return (char *) info;
}




void *__match_proto__(request_handler)
request_dns_handler(void *srv_ctx) {
    // DNS
    net_element *net_node = (net_element *) srv_ctx;                // TODO NULL Check

    dns_info *info = (dns_info *) net_node->rcv_buf;                // TODO NULL Check
    common_ctx  *cctx = malloc(sizeof(common_ctx));
    cctx->log = net_node->tlog_info;
    cctx->lctx = net_node->tlog_info->ctx;
    cctx->log_level = net_node->srv_ctx->log_level;
    cctx->thd_ctx = net_node->thd_ctx->thread_app_ctx;

    int send_len = 0;

    char *conn_ip = malloc(sizeof(char) * 20);                      // TODO NULL Check, may be this should be in 'creat_info()'
    sprintf(conn_ip, "%s", inet_ntoa(net_node->peer->sin_addr));
    info->conn_ip = conn_ip;


//    info->packet_length = MAX_PACKET;      // TODO this or below?
    info->packet_length = (uint16_t) net_node->rcv_buf_len;

    if(cctx->log_level & L_DEBUG) {
        char buf[MAX_PACKET] = {0};
        sprintf(buf, "0x");
        for (size_t count = 0; count < net_node->rcv_buf_len; count++)
            sprintf(buf, "%s%02x", buf, *(info->packet.buf + count));

        LM_DEBUG(cctx->log_level, cctx->lctx, "%s", buf);
    }

    error_list err_type = error_check(info, type_header);
    if (err_type != hdr_no_error) {

        LM_ERR(cctx->log_level,cctx->lctx, "Header Error: %d\n", err_type);

        if (err_type == err_TC_Flag) {
            goto CLEAN_UP;
        }
        else {
            goto ERROR_ANSWER;
        }
    }

    err_type = parse_dns_packet(info);  // 0 = Question Format error

      if (!err_type) {

        info->packet.header.rcode = format_error;
        goto ERROR_ANSWER;
    }

    err_type = error_check(info, type_question);
    if (err_type != no_error){
        LM_ERR(cctx->log_level, cctx->lctx, "Question Error: %d\n", err_type);
        goto ERROR_ANSWER;
    }


    if (info->packet.header.rd && RECURSION_ENABLE)
        recursive_call(info->questions->qname);
    else
        info->packet.header.ra = 0;

    send_len = get_dns_response(info, net_node->app_ctx, cctx);   // May return 'name_error' or response. For 'name_error',
                                                            // send_len will be set to 0. So it does not need to goto
                                                            // ERROR_ANSWER just like normal procedure.

    goto SEND;

    ERROR_ANSWER:
    {
        send_len = net_node->rcv_buf_len;
    }

    SEND:
    {
        #if FLAG_ENABLE_TEST && FLAG_UNITTEST
            return net_node;
        #endif

        //now reply the client with the same rdata
        info->packet.header.qr = 1;

        struct msghdr *msg = net_node->versatile;
        msg->msg_iov->iov_len = send_len;
        msg->msg_iov->iov_base= info->packet.buf;

        if(sendmsg(net_node->sock_fd, net_node->versatile, 0) < 0)
            LM_ERR(cctx->log_level,cctx->lctx, "Error in sending data\n");

        struct timeval end;
        gettimeofday(&end, NULL);
        net_node->tlog_info->time_processing = (end.tv_sec + end.tv_usec / 1000000.0) - net_node->tlog_info->time_req;
        transaction_info(cctx->log);
    }

    CLEAN_UP:
    {
        free_dns_packet(&info);
        delete(net_node);
        free(cctx);
    }

    return NULL;
}


