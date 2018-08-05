
#ifndef GEODNS_LOOOKUP_H
#define GEODNS_LOOOKUP_H

#include <sqlite3.h>

typedef struct _lookup_ctx {
    char *lookup_db_path;   // should be string, not malloced
    sqlite3 *lookup_db;

    char *iso_group_path;
    int iso_count;
    int group_count;

    void *varsetile;
} lookup_ctx;

int init_lookup(void *ctx);
int deinit_lookup(void *ctx);
int handle_lookup(dns_question * const question, dns_rr **rr, void *extras, void *ctx, common_ctx *);

#endif // GEODNS_LOOOKUP_H
