
#ifndef  GEODNS_DNS_APP_H
#define GEODNS_DNS_APP_H

#include "server/server.h"
#include "dns/dns_packet.h"
#include "dns_thread_ctx.h"

typedef struct _common_ctx common_ctx;

typedef int initialize_lookup(void *ctx);
typedef int destroy_lookup(void *ctx);
typedef int lookup_handler(dns_question * const question, dns_rr **rr, void *extras, void *ctx, common_ctx *);

typedef int initialize_geoip(void *ctx);
typedef int destroy_geoip(void *ctx);
typedef char *geoip_handler(char *reff_ip, char *conn_ip, void *ctx , common_ctx *comn_ctx);

typedef struct _dns_ctx {
    void *lookup_ctx;
    initialize_lookup *lookup_initialize;
    destroy_lookup *lookup_destroy;
    lookup_handler *lookup_handle;

    void *geoip_ctx;
    initialize_geoip *geoip_initialize;
    destroy_geoip *geoip_destroy;
    geoip_handler *geoip_handle;

    void *log_conf;     // not owner
    int log_level;
} dns_ctx;

 struct _common_ctx {
    transaction_log *log;
    log_context *lctx;
    int log_level;

     dns_app_thread_ctx *thd_ctx;
} ;

void initialize_dns_app(server_ctx *srv_ctx, void *app_ctx);
void destroy_dns_app(void *app_ctx);
char *get_dns_buffer(int len);
void *request_dns_handler(void *srv_ctx);

#endif // GEODNS_DNS_APP_H
