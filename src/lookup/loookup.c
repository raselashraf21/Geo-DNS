
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <dns/dns_app.h>

#include "utility/def.h"
#include "dns/dns_packet.h"
#include "lookup/loookup.h"
#include "utility/sqlite_db_backup.h"
#include "utility/new.h"
#include "iso_groups.h"

typedef struct _ip_info{
    char *ip;
    char *type;
    int priority;
    uint32_t ttl;
} ip_info;

typedef struct _ip_info_list{
    ip_info *entry[MAX_IP_ENTRY];
    int count;
} ip_info_list;

typedef struct _host_groups {
    int groups[MAX_HOST_GROUP];
    int len;
} host_groups;


static int search_ip_entry(ip_info_list *list, int start, int end, int value){
    if(start >= end) {
        return 0;
    }
    if(list->entry[start]->priority > value){
        return start;
    } else if(list->entry[end]->priority > value && list->entry[end-1]->priority <= value){
        return end;
    }

    int mid = (start + end) >> 1;
    if (value >= list->entry[mid]->priority){ //search right
        return search_ip_entry(list, mid, end, value);
    }
    else { //search left
        return search_ip_entry(list, start, mid, value);
    }
}

static int get_answer(char *host, ip_info_list *list, dns_rr **answers, uint32_t *ip_ucount) {
    dns_rr *counter = NULL;
    dns_rr *head = NULL;
    /*
     * for the first time: the result will ignore first set of answer
     * later the round-robin executes correctly
     * */
    *ip_ucount = (++(*ip_ucount)) % (list->entry[list->count-1]->priority);
    int index = search_ip_entry(list, 0, list->count-1, *ip_ucount);
    int loop = 0;
    while (loop < MAX_ANSWER){
        if(index == list->count){
            index = 0;
        }
        dns_rr *tmp = malloc(sizeof(dns_rr));
        tmp->name = strdup(host);
        tmp->cls = 1;
        tmp->type = 1;
        tmp->rr_name = strdup(list->entry[index]->type);
        tmp->ttl.norm_ttl = list->entry[index]->ttl;
        tmp->rdlength = 4;
        tmp->rdata = strdup(list->entry[index]->ip);
        tmp->next = NULL;
        if (head == NULL) {
            head = tmp;
            counter = tmp;
        } else {
            counter->next = tmp;
            counter = counter->next;
        }
        index++;
        loop++;
    }

    *answers = head;
    return loop;
}

static int callback_host(void *data, int argc, char **argv, char **az_col_name) {
    host_groups *groups = (host_groups *)data;
    groups->groups[groups->len] = atoi(argv[0]);
    groups->len++;
    return 0;
}

static int get_host_groups(char *host_name, host_groups *groups, sqlite3 *db, common_ctx *comn_ctx) {

    int  status;
    char *zErrMsg = 0;

    char *sql = "SELECT ip_group FROM table_host_group WHERE host LIKE '%s'";
    int length = snprintf(NULL, 0, sql, host_name);
    length++; // For '\0' termination.
    char *newSql = (char *)malloc(sizeof(char) * length);
    snprintf(newSql, length, sql, host_name);

    /* Execute SQL statement */
    status = sqlite3_exec(db, newSql, callback_host, groups, &zErrMsg);
    if (status != SQLITE_OK) {
        LM_ERR(comn_ctx->log_level, comn_ctx->lctx,"SQL error (host query) %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    free(newSql);

    return status;
}

static int callback_ip_info(void *data, int argc, char **argv, char **az_col_name) {
    ip_info_list *list = (ip_info_list *)data;

    ip_info *ipt = malloc(sizeof(ip_info));
    ipt->ip = strdup(argv[0]);
    ipt->type = strdup(argv[1]);
    ipt->priority = atoi(argv[2]);
    ipt->ttl = atoi(argv[3]);

    list->entry[list->count] = ipt;
    list->count++;
    return 0;
}

static int get_ip_info(char *group, ip_info_list *iptype, sqlite3 *db, common_ctx *comn_ctx) {
    int  status;
    char *zErrMsg = 0;
    char *sql = "SELECT ip, type, priority, ttl FROM table_ip_info WHERE ip_group = %s";
    int length = snprintf(NULL, 0, sql, group);
    length++; // For '\0' termination.
    char *newSql = (char *)malloc(sizeof(char) * length);
    snprintf(newSql, length, sql, group);

    /* Execute SQL statement */
    status = sqlite3_exec(db, newSql, callback_ip_info, iptype, &zErrMsg);
    if (status != SQLITE_OK) {
        LM_ERR(comn_ctx->log_level, comn_ctx->lctx,"SQL error get ip type: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    free(newSql);

    return status;
}

int __match_proto__(initialize_lookup)
init_lookup(void *ctx) {
    // positive number > 0
    // 0 - successfull
    lookup_ctx *lctx = (lookup_ctx *)ctx;

    lctx->varsetile = new(IsoGroupsCtx, lctx->iso_group_path);
    sqlite3_open(":memory:", &(lctx->lookup_db));

    return loadOrSaveDb(lctx->lookup_db, lctx->lookup_db_path, 0);
}

int __match_proto__(destroy_lookup)
deinit_lookup(void *ctx) {
    lookup_ctx *lctx = (lookup_ctx *)ctx;

    // Freeing 'iso_groups_ctx'
    delete(lctx->varsetile);
    lctx->varsetile = NULL;

    return sqlite3_close(((lookup_ctx *)ctx)->lookup_db);       // TODO : Change here
}

int __match_proto__(lookup_handler)
handle_lookup(dns_question * const question, dns_rr **rr, void *extras, void *ctx, common_ctx *comn_ctx) {
    int status;
    int found_group;                    // first entry of (host -> groups) in (country -> groups)
    int anscount = 0;
    char *question_name;
    char *country =  extras;
    host_groups *hgroups = NULL;        // host -> groups
    ip_info_list *ip_info_entry = NULL; // group
    uint32_t *grp_ip_ucount = comn_ctx->thd_ctx->grp_ip_uses;

    if (!question || !rr || !extras || !ctx) {
        LM_ERR(comn_ctx->log_level, comn_ctx->lctx, "[ERROER] Null given (question/rr/extras/ctx)");
        goto exit;
    }

    lookup_ctx *lctx = (lookup_ctx *)ctx;
    sqlite3 *lookup_db = lctx->lookup_db;
    question_name = question->qname;
    //question_name = "dnsstickerres.ringid.com";
    hgroups = malloc(sizeof(host_groups));
    hgroups->len = 0;
    status = get_host_groups(question_name, hgroups, lookup_db, comn_ctx);

    if (status != 0 || hgroups->len == 0) {

        LM_ERR(comn_ctx->log_level, comn_ctx->lctx, "Host not found\n");
        goto exit;
    }

    if (strcmp(country, "$$") != 0) {
        found_group = get_iso_host_group(hgroups->groups, hgroups->len, country, lctx->varsetile);
    }
    else {
        found_group = hgroups->groups[0];
    }

    ip_info_entry = malloc(sizeof(ip_info_list));
    ip_info_entry->count = 0;
    char buff[20];
    sprintf(buff, "%d", found_group);
    status = get_ip_info(buff, ip_info_entry, lookup_db, comn_ctx);
    if (status != 0){

        LM_ERR(comn_ctx->log_level, comn_ctx->lctx, "sql error\n");
        goto exit;
    }
    
    anscount =  get_answer(question_name, ip_info_entry, rr, &grp_ip_ucount[found_group]);

exit:
    if (hgroups) {
        free(hgroups);
    }

    if (ip_info_entry) {
        int count = ip_info_entry->count;
        for (int i = 0; i < count; ++i) {
            ip_info *entry = ip_info_entry->entry[i];
            free(entry->ip);
            free(entry->type);
            free(ip_info_entry->entry[i]);
        }
        free(ip_info_entry);
    }

    return anscount;
}
