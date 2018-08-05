#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <maxminddb.h>

#include "utility/def.h"
#include "geoip.h"
#include "dns/dns_app.h"

int __match_proto__(initialize_geoip)
init_geoip(void *ctx) {
    geoip_ctx *gctx = (geoip_ctx *)ctx;
    gctx->geoip_db = malloc(sizeof(MMDB_s));
    return MMDB_open(gctx->geoip_db_path, MMDB_MODE_MMAP, gctx->geoip_db);	// MMDB_MODE_MMAP = 1
}

int __match_proto__(destroy_geoip)
deinit_geoip(void *ctx) {
    geoip_ctx *gctx = (geoip_ctx *)ctx;
    if (gctx->geoip_db) {
        MMDB_close(gctx->geoip_db);
        free(gctx->geoip_db);
        gctx->geoip_db = NULL;
    }

    return 0;
}


char * __match_proto__(geoip_handler)
handle_geoip(char *ref_ip, char *conn_ip, void *ctx, common_ctx *comn_ctx) {
    /**
     *
     * @brief searches entry for ref_ip, if any, and/or conn_ip in mmdb
     * @param ref_ip a pointer to a character array to be malloc'd, can be NULL
     * @param conn_ip a pointer to a character array to be malloc'd, can be NULL
     * @param ctx
     * @return a dynamically allocated string
     */

    char *result_iso = malloc(sizeof(char) * 4);

    /*each time pivot_ip is fed up with ref_ip(if any) or conn_ip*/

    char *pivot_ip = malloc(sizeof(char*) * IP_SIZE);

    if (ref_ip == NULL) {
        if (conn_ip == NULL) return NULL;
        else {
            strcpy(pivot_ip, conn_ip);
        }
    }
    else {
        strcpy(pivot_ip, ref_ip);
    }

    MMDB_s *mmdb = ((geoip_ctx *) ctx)->geoip_db;


    if (mmdb) {
        int gai_error, mmdb_error;
        MMDB_lookup_result_s result = MMDB_lookup_string(mmdb, pivot_ip, &gai_error, &mmdb_error);

        if (gai_error) {
            LM_ERR(comn_ctx->log_level, comn_ctx->lctx, "[GeoLookup] Error from getaddrinfo for %s - %s\n", pivot_ip, gai_strerror(gai_error));
        }

        if (mmdb_error != MMDB_SUCCESS) {
            LM_ERR(comn_ctx->log_level, comn_ctx->lctx,"[GeoLookup] Got an error from libmaxminddb: %s\n", MMDB_strerror(mmdb_error));
        }

        MMDB_entry_data_list_s *entry_data_list = NULL;

        if (result.found_entry) {

            MMDB_entry_data_s entry_data;

            char *lookup[3][3];

            lookup[0][0] = "country";
            lookup[0][1] = "iso_code";
            lookup[0][2] = NULL;

            lookup[1][0] = "registered_country";
            lookup[1][1] = "iso_code";
            lookup[1][2] = NULL;

            lookup[2][0] = "continent";
            lookup[2][1] = "code";
            lookup[2][2] = NULL;

            int count = 0;
            int status = MMDB_LOOKUP_PATH_DOES_NOT_MATCH_DATA_ERROR;
            while (count < 3 && status == MMDB_LOOKUP_PATH_DOES_NOT_MATCH_DATA_ERROR) {
                status = MMDB_aget_value(&result.entry, &entry_data, lookup[count]);
                count++;
            }

            if (status == MMDB_SUCCESS) {
                if (entry_data.offset) {
                    MMDB_entry_s entry = {.mmdb = mmdb, .offset = entry_data.offset};
                    status = MMDB_get_entry_data_list(&entry, &entry_data_list);
                }
                else {
                    LM_CRITICAL(comn_ctx->log_level, comn_ctx->lctx,"[GeoLookup] No data was found at the lookup path you provided\n");
                }
            }
            else {
                LM_ERR(comn_ctx->log_level, comn_ctx->lctx,"[GeoLookup] Got an error looking up the entry data - %s\n", MMDB_strerror(status));
            }

            if (entry_data_list) {
                const char *country_iso = entry_data_list->entry_data.utf8_string;
                result_iso[0] = country_iso[0];                    //truncating utf8_string to ASCII string manually
                result_iso[1] = country_iso[1];                    //needs to find out a converter
                result_iso[2] = '\0';
            }
        }
        else {
            LM_CRITICAL(comn_ctx->log_level, comn_ctx->lctx,"[GeoLookup] No entry for this IP address (%s) was found\n", pivot_ip);
            strcpy(result_iso, "$$");                            // '$$' stands for 'default'
        }
        MMDB_free_entry_data_list(entry_data_list);
    }
    else {
        LM_ERR(comn_ctx->log_level, comn_ctx->lctx,"mmdb error\n");
      }

    free(pivot_ip);

    return result_iso;
}