
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "server/server.h"
#include "factory/factory.h"
#include "dns/dns_packet_parser.h"
#include "dns/dns_response.h"
#include "utility/new.h"

int test_packet_parse() {
    const char msg_hex[] = "ee14000000010000000000010c646e73696d616765737265730672696e67696403636f6d00000100010000292000000080000000";
//    const char msg_hex[] = "ee14040000010003000000000c646e73696d616765737265730672696e67696403636f6d00000100010c646e73696d616765737265730672696e67696403636f6d0367696405636f6d254100000100010000007d000468c124140c646e73696d616765737265730672696e67696403636f6d00000100010000007d000468c124150c646e73696d616765737265730672696e67696403636f6d02c41a0365e07f00000100010000007d000468c12416";
//    const char msg_hex[] = "57e4840000010002000000000c646e73696d616765737265730672696e67696403636f6d00000100010c646e73696d616765737265730672696e67696403636f6d0000010001000000580004671a73960c646e73696d616765737265730672696e67696403636f6d01250000010001000000580004671a7397";
//    const char msg_hex[] = "ee14040000010003000000000c646e73696d616765737265730672696e67696403636f6d00000100010c646e73696d616765737265730672696e67696403636f6d00000100010000007d000468c124140c646e73696d616765737265730672696e67696403636f6d00000100010000007d000468c124150c646e73696d616765737265730672696e67696403636f6d00000100010000007d000468c12416";

    _DEBUG {
        printf("Packet Size: %lu\n", strlen(msg_hex));
        if (strlen(msg_hex) % 2 == 1)
        {
            printf("Odd length of hex-string. Should be even length");
            return 0;
        }
        printf("0x%s\n", msg_hex);
    }

    const char *pos = msg_hex;

    // dummy packet creation
    uint8_t buf[MAX_PACKET];
    memset(buf, 0, sizeof(buf));
    int buff_len = (int)strlen(msg_hex) / 2;
    for(size_t count = 0; count < buff_len; count++) {
        sscanf(pos, "%2hhx", (buf + count));
        pos += 2;
    }

    // server_app creation
    server_app *app;
    make_app((void **) &app, NULL);

    // server_app initialization
    app->initialize(app->srv_ctx, app->app_ctx);

    // server_app's thread_ctx initialization
    thread_ctx *tctx = calloc(1, sizeof(thread_ctx));
    app->initialize_thd_ctx(app->srv_ctx, app->app_ctx, tctx);


    dns_info *info = create_info();
    memcpy(info, buf, buff_len);
    info->packet_length = buff_len;
    char *conn_ip = malloc(sizeof(char) * 20);
    sprintf(conn_ip, "%s", "38.108.92.201");
    info->conn_ip = conn_ip;
    parse_dns_packet(info);

    transaction_log *tlog_info = new(TransactionLog, NULL);
    tlog_info->ctx = new(LogContext, NULL);
    tlog_info->ctx->log_app = NULL;     // no need for syslog
    common_ctx *vctx = malloc(sizeof(common_ctx));
    vctx->log = tlog_info;
    vctx->lctx = tlog_info->ctx;
    vctx->thd_ctx = tctx->thread_app_ctx;

//    inspect_dns_packet(info, tlog_info->ctx);
    print_dns_packet(info);
    int send_len = get_dns_response(info, app->app_ctx, vctx);
//    inspect_dns_packet(info, tlog_info->ctx);
    print_dns_packet(info);

    _DEBUG_CHANGE {
        printf("0x");
        for (size_t count = 0; count < send_len; count++)
            printf("%02x", *(info->packet.buf + count));
        printf("\n");
    }
    free_dns_packet(&info);

    free(vctx);
    delete(tlog_info->ctx);
    delete(tlog_info);

    // server_app de-initialization
    app->destroy(app->app_ctx);

    // server_app's thread_ctx initialization
    app->destroy_thd_ctx(tctx);
    free(tctx);

    // free-up server_app
    free_app((void **)&app);

    return 0;
}