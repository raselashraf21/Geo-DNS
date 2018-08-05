
#include <stdlib.h>

#include "dns_thread_ctx.h"
#include "utility/def.h"
#include "dns/dns_app.h"
#include "lookup/loookup.h"

void __match_proto__(initialize_thread_ctx)
initialize_dns_thread_ctx(server_ctx *srv_ctx, void *app_ctx, thread_ctx *thd_ctx) {
    lookup_ctx *lctx = (lookup_ctx *)((dns_ctx *)app_ctx)->lookup_ctx;

    if (thd_ctx && lctx->group_count > 0) {
        dns_app_thread_ctx *th_app_ctx = calloc(1, sizeof(dns_app_thread_ctx));
        th_app_ctx->grp_ip_uses = calloc((size_t)lctx->group_count, sizeof(uint32_t));
        thd_ctx->thread_app_ctx = th_app_ctx;
    }
}

void __match_proto__(destroy_thread_ctx)
destroy_dns_thread_ctx(thread_ctx *thd_ctx) {

    if (thd_ctx && thd_ctx->thread_app_ctx) {
        free(thd_ctx->thread_app_ctx);
    }
}
