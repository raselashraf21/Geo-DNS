
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utility/new.h"
#include "server/server.h"
#include "dns/dns_app.h"
#include "factory/factory.h"
#include "dns/dns_packet.h"
#include "dns/dns_packet_parser.h"
#include "lookup/loookup.h"

int test_lookup() {
    const char msg_hex[] = "247400000001000000000001016308697076636c6f75640672696e"
                           "67696403636f6d0000010001000029100000008000000b00080007"
                           "00011800671a70";                                            // Request: Type A(1) with opts (ecs)

    dns_info *info = malloc(sizeof(dns_info));
    info->packet_length = MAX_PACKET;
    const char *pos = msg_hex;

    for(size_t count = 0; count < strlen(msg_hex)/2; count++) {
        sscanf(pos, "%2hhx", (info->packet.buf + count));
        pos += 2;
    }

//    printf("\n\n----------------------- Parsing Question -----------------------\n");
    parse_dns_packet(info);
//    print_dns_packet(info);

    // server_app creation
    server_app *app;
    make_app((void **) &app, NULL);

    // server_app initialization
    app->initialize(app->srv_ctx, app->app_ctx);

    // server_app's thread_ctx initialization
    thread_ctx *tctx = calloc(1, sizeof(thread_ctx));
    app->initialize_thd_ctx(app->srv_ctx, app->app_ctx, tctx);

    dns_ctx *dctx = (dns_ctx *)app->app_ctx;

    transaction_log *tlog_info = new(TransactionLog, NULL);
    tlog_info->ctx = new(LogContext, NULL);
    tlog_info->ctx->log_app = NULL;     // no need for syslog
    common_ctx *vctx = malloc(sizeof(common_ctx));
    vctx->log = tlog_info;
    vctx->thd_ctx = tctx->thread_app_ctx;



/**************  [TEST] check lookup  **************/

//    //dns_question *questions;
//    dns_rr *answers = NULL;
//    char referrer_ip[20] = "103.26.112.17";
//    char connection_ip[20] = "1.0.4.0";
//    char *result_iso = dctx->geoip_handle(referrer_ip, connection_ip, dctx->geoip_ctx, vctx);
//    result_iso = strdup("CA");
//    clock_t begin = clock();
//    int ans_count = dctx->lookup_handle(info->questions, &answers, result_iso, dctx->lookup_ctx, vctx);
//    clock_t end = clock();
//    printf("\n\nanswers : %d\n", ans_count);
//    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
//    printf("Time difference: %lf\n\n", time_spent);
//
////    free(result_iso);
//    free_rr(&answers, ans_count);




    for(int loop=0; loop < 3; loop++){
        puts("----test-lookup-----");
        //dns_question *questions;
        dns_rr *answers = NULL;
        char referrer_ip[20] = "103.26.112.17";
        char connection_ip[20] = "1.0.4.0";
        char *result_iso = dctx->geoip_handle(referrer_ip, connection_ip, dctx->geoip_ctx, vctx);
        result_iso = strdup("BD");
        //clock_t begin = clock();
        int ans_count = dctx->lookup_handle(info->questions, &answers, result_iso, dctx->lookup_ctx, vctx);
        //clock_t end = clock();
//        printf("\n\nanswers : %d\n", ans_count);
        //double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        //printf("Time difference: %lf\n\n", time_spent);

//        print_dns_rr(answers);
//    free(result_iso);
        free_rr(&answers, ans_count);

    }

/***************************************************/



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