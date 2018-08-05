
#ifndef GEODNS_GEOIP_H
#define GEODNS_GEOIP_H

#include <maxminddb.h>
#include <dns/dns_app.h>

typedef struct _geoip_ctx {
    char *geoip_db_path;   // should be string, not malloced
    MMDB_s *geoip_db;
} geoip_ctx;

int init_geoip(void *ctx);
int deinit_geoip(void *ctx);
char *handle_geoip(char *reff_ip, char *conn_ip, void *ctx, common_ctx *);

#endif // GEODNS_GEOIP_H
