
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

#include "factory/factory.h"
#include "server/server.h"
#include "dns/dns_app.h"
#include "dns/dns_thread_ctx.h"
#include "lookup/loookup.h"
#include "lookup/geoip.h"
#include "utility/new.h"
#include "utility/ini_parser.h"
#include "config_key.h"

#ifndef EXIT_CODE_INI
    #define EXIT_CODE_INI 104
#endif

int _get_log_priority(char *priority_str) {
    if (priority_str == NULL) {
        return LOG_LOCAL0;
    }
    else if (!strcmp(priority_str, "local0")) {
        return LOG_LOCAL0;
    }
    else if (!strcmp(priority_str, "local1")) {
        return LOG_LOCAL1;
    }
    else if (!strcmp(priority_str, "local2")) {
        return LOG_LOCAL2;
    }
    else if (!strcmp(priority_str, "local3")) {
        return LOG_LOCAL3;
    }
    else if (!strcmp(priority_str, "local4")) {
        return LOG_LOCAL4;
    }
    else if (!strcmp(priority_str, "local5")) {
        return LOG_LOCAL5;
    }
    else if (!strcmp(priority_str, "local6")) {
        return LOG_LOCAL6;
    }
    else {
        return LOG_LOCAL0;
    }
}

// 'factory' implementation
void make_app(void **srv_app, char *config_path) {

    int status = 0;
    char *conf_path = DEFAULT_PATH_CONF_INI;
    if (config_path) {
        conf_path = config_path;
    }

    cfg_param_t *cfg_t = malloc(sizeof(cfg_param_t));
    status = ini_parse(conf_path, cfg_t);
    if (status) {
        LM_PRINT(L_ALL, "[ERROR] Cannot open configuration(ini) file. (path - %s, status %d)\n", conf_path, status);
        exit(EXIT_CODE_INI);
    }

    // create initial log context
    char *log_name = NULL;
    char *log_priority_str = NULL;
    int log_level = L_ALL;
    get_param_value(cfg_t, KEY_LOG_NAME, &log_name);
    get_param_value(cfg_t, KEY_LOG_PRIORITY, &log_priority_str);
    get_int(cfg_t, KEY_LOG_LEVEL, &log_level);
    int log_priority = _get_log_priority(log_priority_str);
    void *log_app = new(ConfLog, log_name, LOG_CONS | LOG_PID | LOG_NDELAY, log_priority);
    if (log_name)
        free(log_name);
    if (log_priority_str)
        free(log_priority_str);
    LM_PRINT(L_ALL, "Log level has been set to %d\n", log_level);


    // create initial lookup context
    lookup_ctx *lctx = malloc(sizeof(lookup_ctx));

    // TODO: Give Default Value or give exception
    get_param_value(cfg_t, KEY_PATH_DB_SQLITE3, &lctx->lookup_db_path);
    get_param_value(cfg_t, KEY_PATH_COUNTRY_ISO, &lctx->iso_group_path);
    get_int(cfg_t, KEY_ISO_COUNT, &lctx->iso_count);
    get_int(cfg_t, KEY_GROUP_COUNT, &lctx->group_count);

    // create initial geoip context
    geoip_ctx *gctx = malloc(sizeof(geoip_ctx));
    // TODO: Give Default Value or give exception
    get_param_value(cfg_t, KEY_PATH_DB_MAXMIND, &gctx->geoip_db_path);

    // create initial app context
    dns_ctx *app_ctx = malloc(sizeof(dns_ctx));
    app_ctx->lookup_ctx = lctx;
    app_ctx->lookup_initialize = init_lookup;
    app_ctx->lookup_destroy = deinit_lookup;
    app_ctx->lookup_handle = handle_lookup;
    app_ctx->geoip_ctx = gctx;
    app_ctx->geoip_initialize = init_geoip;
    app_ctx->geoip_destroy = deinit_geoip;
    app_ctx->geoip_handle = handle_geoip;
    app_ctx->log_conf = log_app;
    app_ctx->log_level = log_level;

    // create server context

    thread_ctx *th_ctx = (thread_ctx *) malloc(sizeof(thread_ctx));
    get_int(cfg_t, KEY_THREAD_COUNT, &th_ctx->thread_count);
    th_ctx->thread_id = malloc(sizeof(pthread_t) * th_ctx->thread_count);
    th_ctx->thread_term = malloc(sizeof(int) * th_ctx->thread_count);

    server_ctx *srv_ctx = (server_ctx *) malloc(sizeof(server_ctx));
    srv_ctx->protocol = udp;
    srv_ctx->srv_type = dns_server;

    // TODO: Give Default Value or give exception
    get_param_value(cfg_t, KEY_SERVER_IP, &srv_ctx->ip);
    get_int(cfg_t, KEY_SERVER_PORT, &srv_ctx->port);
    srv_ctx->th_ctx = th_ctx;
    srv_ctx->log_conf = log_app;
    srv_ctx->log_level = app_ctx->log_level;

    // create server app
    server_app *app = (server_app *) malloc(sizeof(server_app));
    app->srv_term = 0;
    app->srv_ctx = srv_ctx;
    app->app_ctx = app_ctx;
    app->initialize = initialize_dns_app;
    app->destroy = destroy_dns_app;
    app->get_buffer = get_dns_buffer;
    app->handle_request = request_dns_handler;
    app->initialize_thd_ctx = initialize_dns_thread_ctx;
    app->destroy_thd_ctx = destroy_dns_thread_ctx;

    *srv_app = app;

    free(cfg_t->cfg);
    free(cfg_t);
}

void free_app(void **app) {
    if (*app) {
        server_app *sapp = *app;
        if (sapp->app_ctx) {
            sapp->destroy(sapp->app_ctx);
            free(sapp->app_ctx);
        }

        if (sapp->srv_ctx) {

            if(sapp->srv_ctx->th_ctx){
                 thread_ctx *th_dctx = sapp->srv_ctx->th_ctx;
                 free(th_dctx->thread_id);
                 free(th_dctx->thread_term);
            }

            if (sapp->srv_ctx->log_conf) {
                delete(sapp->srv_ctx->log_conf);
            }

            if (sapp->srv_ctx->ip) {
                free(sapp->srv_ctx->ip);
            }
        }

        free(*app);
        app = NULL;
    }
}

