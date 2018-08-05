
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "config.h"
#include "utility/new.h"
#include "server/server.h"
#include "factory/factory.h"

int test_parse()
{
//    const char msg_hex[] ="16b50100000100000000000006676f6f676c6503636f6d0000010001";   // Quesion: Type A(1), single
//    const char msg_hex[]="16b50100000200000000000006676f6f676c6503636f6d0000010001"
//                                                 "06676f6f676c6503636f6d0000010001";    // Quesion: Type A(1), double
//    const char msg_hex[]="17758180000100010000000006676f6f676c6503636f6d0000010001"
//                                                 "c00c000100010000007d0004d83ac54e";    // Response: Type A(1), single
//    const char msg_hex[]="a56a0100000100000000000002373803313937023538033231360769"
//                                                 "6e2d61646472046172706100000c0001";    // Question: Type PTR(12)
//    const char msg_hex[]="a55a8180000100010000000002373803313937023538033231360769"
//                         "6e2d61646472046172706100000c0001c00c000c00010000545f001b"
//                           "0f6d616130337332312d696e2d663134053165313030036e657400";    // Response: Type PTR(12)
//    const char msg_hex[]="0be40100000100000000000008636c69656e74733406676f6f676c65"
//                                                               "03636f6d0000010001";    // Request: Type A(1)
//    const char msg_hex[]="0be48180000100020000000008636c69656e74733406676f6f676c65"
//                         "03636f6d0000010001c00c000500010000010b000c07636c69656e74"
//                                       "73016cc015c031000100010000010b0004d83ac54e";    // Response: Type
//    const char msg_hex[] = "247400000001000000000001016308697076636c6f75640672696e"
//                           "67696403636f6d0000010001000029100000008000000b00080007"
////                           "67696403636f6d0000010002000029100000008000000b00080007"     // Ashraf-Test
//                                                                   "00011800671a70";    // Request: Type A(1) with opts (ecs)
//    const char msg_hex[] = "d9fb000000010000000000010c646e73696d61676573726573067269"
//            "6e67696403636f6d0000010001000029100000008000000b0008000700011800671a70";
//    const char msg_hex[] = "12e1000000010000000000010c646e73696d616765737265730672696"
//            "e67696403636f6d0000010001000029100000008000000b0008000700011800266c5c";        // dnsimagesres.ringid.com
//    const char msg_hex[] = "2dd7000000010000000000000d646e73737469636b65727265730672696e67696403636f6d0000010001";
//    const char msg_hex[] = "57e4840000010002000000000c646e73696d616765737265730672696e67696403636f6d00000100010c"
//                           "646e73696d616765737265730672696e67696403636f6d0000010001000000580004671a73960c646e73"
//                           "696d616765737265730672696e67696403636f6d01250000010001000000580004671a7397";
    const char msg_hex[] = "1bde000000010000000000010d646e73737469636b657272657306726"
            "96e67696403636f6d0000010001000029100000008000000b0008000700011800266c5c";
                            /*  // Ashraf-Test
                                char msg_hex[] = "2474\ -- id
                                        0000\   -- flags
                                        0001\   --  questions
                                        0000\   -- answers
                                        0000\   -- authority rr
                                        0001\   -- additional rr
                                        016308697076636c6f75640672696e67696403636f6d00\ -- testcloud.com
                                        0001\   -- qtype
                                        0001\   -- qclass
                                        00\     -- root name [additinal]
                                        0029\   -- opt type
                                        1000\   -- upd payload size
                                        00\     -- extened rCode
                                        00\     -- endnso version
                                        8000\   -- z
                                        000b\   -- data length
                                        0008\   -- option code (CSUBNET)
                                        0007\   -- option length
                                        0001\   -- Family
                                        18\     -- source netmask
                                        00\     -- scope netmask
                                        671a70";
                            */

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
    for(size_t count = 0; count < strlen(msg_hex)/2; count++) {
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

    for (int i = 0; i < 1; ++i) {
/*********** [TEST] check packet parsing ***********/

        // dummy net_node creation
        net_element *net_node = new(NetElement, app);

        log_context *ctx = new(LogContext, NULL);
        ctx->log_app = app;
        net_node->tlog_info =new(TransactionLog, NULL);
        net_node->tlog_info->ctx = ctx;

        net_node->thd_ctx = tctx;

        struct timeval start;
        gettimeofday(&start, NULL);
        net_node->tlog_info->time_req = start.tv_sec + (start.tv_usec / 1000000.0);

        net_node->id = 0;
        net_node->th_status = 0;
        net_node->sock_fd = -1;
        net_node->rcv_type = 0;
        net_node->peer = malloc(sizeof(struct sockaddr_in));
        net_node->peer->sin_family = AF_INET;
        net_node->peer->sin_port = htons(8888);
        net_node->peer->sin_addr.s_addr = htonl(INADDR_ANY);
        net_node->peer_len = sizeof(struct sockaddr_in);
        net_node->srv_ctx = app->srv_ctx;
        net_node->app_ctx = app->app_ctx;
        char *info = app->get_buffer(MAX_PACKET);
        memcpy(info, buf, sizeof(buf));
        net_node->rcv_buf = info;

        struct msghdr *msg = calloc(1, sizeof(struct msghdr));
        struct iovec *iov = calloc(1, sizeof(struct iovec));

        msg->msg_name           = net_node->peer;
        msg->msg_namelen        = sizeof (struct sockaddr_in);
        msg->msg_iov            = iov;
        msg->msg_iov->iov_base  = net_node->rcv_buf;
        msg->msg_iov->iov_len   = MAX_PACKET;
        msg->msg_iovlen         = 1;

        net_node->versatile = msg;

        net_node->rcv_buf_len = strlen(msg_hex)/2;
        net_node->handle_buf = app->handle_request;

        net_node->handle_buf(net_node);
/***************************************************/
    }

    // server_app de-initialization
    app->destroy(app->app_ctx);

    // server_app's thread_ctx initialization
    app->destroy_thd_ctx(tctx);
    free(tctx);

    // free-up server_app
    free_app((void **)&app);

    return 0;
}
