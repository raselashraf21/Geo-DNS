
#ifndef GEODNS_DNS_THREAD_CTX_H
#define GEODNS_DNS_THREAD_CTX_H

#include "server/server.h"

/*!
 * WARNING: Don't manually create 'dns_app_thread_ctx'.
 *          It is created automatically by 'initialize_dns_thread_ctx(...)' function
 *          and assigned to 'thread_ctx'
 *
 */
typedef struct _dns_app_thread_ctx {
    uint32_t *grp_ip_uses;      // Array of group's ip-uses count
} dns_app_thread_ctx;

void initialize_dns_thread_ctx(server_ctx *srv_ctx, void *app_ctx, thread_ctx *thd_ctx);
void destroy_dns_thread_ctx(thread_ctx *thd_ctx);

#endif // GEODNS_DNS_THREAD_CTX_H
