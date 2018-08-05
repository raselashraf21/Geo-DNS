
#include <stdio.h>
#include "server.h"
#include "utility/new.h"
#include "config.h"


/********** Object Implementation for type LogContext(log_context) **********/

static void *net_element_ctor (void * _self, size_t argc, va_list *argv)
{
    net_element *net_node = _self;

    net_node->tlog_info = new(TransactionLog, NULL);
    net_node->tlog_info->ctx = new(LogContext, NULL);

    if (argc > 0) {
        server_app *app =  va_arg(*argv, server_app *);;
        net_node->srv_ctx = app->srv_ctx;
        net_node->app_ctx = app->app_ctx;
        net_node->rcv_buf = app->get_buffer(MAX_PACKET);    // TODO: Null Check
        net_node->tlog_info->ctx->log_app = app->srv_ctx->log_conf;
    }

    net_node->th_status = 0;
    net_node->rcv_type = 0;
    net_node->peer_len = sizeof(struct sockaddr_in);

    return net_node;
}

static void *net_element_dtor (void * _self)
{
    net_element *net_node = _self;

    if (net_node->tlog_info->ctx) delete(net_node->tlog_info->ctx);
    if (net_node->tlog_info) delete(net_node->tlog_info);

    return net_node;
}

static const struct Class _NetElement = {
        sizeof(net_element),
        net_element_ctor,
        net_element_dtor
};
const void *NetElement = &_NetElement;





