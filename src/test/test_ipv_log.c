
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <sys/time.h>

#include "utility/new.h"
#include "utility/log/log.h"
#include "utility/dns_log.h"

int test_log(){
//    void *conf = new(ConfLog, NULL);
    void *conf = new(ConfLog, "geodns", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL2);

    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    log_context *ctx = new(LogContext, NULL);
    ctx->log_app = conf;
    log_debug(ctx, "hello %s", "mello");
    log_info(ctx, "hello %s", "mello");
    log_error(ctx, "hello %s", "mello");
    log_warning(ctx, "hello %s", "mello");
    log_critical(ctx, "hello %s", "mello");

    transaction_log *logg = new(TransactionLog, NULL);
    logg->ctx = ctx;
    logg->question = strdup("testcloud.com");
    logg->conn_ip = strdup("8.8.8.8");
    logg->reff_ip = strdup("192.168.8.86");
    logg->country = strdup("BD");
    logg->answers = strdup("38.108.192.100/38.108.192.101");
    logg->time_req = start.tv_sec + (start.tv_usec / 1000000.0);

    gettimeofday(&end, NULL);

    logg->time_processing = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);
    transaction_info(logg);
    delete(logg);


    delete(ctx);

//    ctx = new(LogContext, NULL);
//    ctx->log_app = conf;
//    log_debug(ctx, "hello1 %s", "mello");
//    log_info(ctx, "hello1 %s", "mello");
//    log_error(ctx, "hello1 %s", "mello");
//    log_warning(ctx, "hello1 %s", "mello");
//    log_critical(ctx, "hello1 %s", "mello");
//    delete(ctx);
//
//    ctx = new(LogContext, NULL);
//    ctx->log_app = conf;
//    log_debug(ctx, "hello2 %s", "mello");
//    log_info(ctx, "hello2 %s", "mello");
//    log_error(ctx, "hello2 %s", "mello");
//    log_warning(ctx, "hello2 %s", "mello");
//    log_critical(ctx, "hello2 %s", "mello");
//    delete(ctx);
//
//    ctx = new(LogContext, NULL);
//    ctx->log_app = conf;
//    log_debug(ctx, "hello3 %s", "mello");
//    log_info(ctx, "hello3 %s", "mello");
//    log_error(ctx, "hello3 %s", "mello");
//    log_warning(ctx, "hello3 %s", "mello");
//    log_critical(ctx, "hello3 %s", "mello");
//    delete(ctx);

    delete(conf);

    return 0;
}

