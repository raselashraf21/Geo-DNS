
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>     // htons(..) some system provides netinet/in.h
#include <arpa/inet.h>      // htons(..) some system provides arpa/inet.h

#include "config.h"
#include "dns/dns_packet.h"
#include "dns/dns_response.h"

int lookup_zone(char *qname) {
    return 1;   // 1 = Found
}

int recursive_call(char *qname) {
    return 0;
}

// 'header' cannot be NULL. So before calling this function, check 'header' whether it is NULL or not.
int make_header(dns_header *const header, uint16_t ancount, uint16_t nscount, uint16_t arcount) {
    int ret_code = 0;
    if (ancount < 1 && nscount < 1) {
        header->rcode = name_error;
        ret_code = -1;
    }
    else {
        header->aa = 1;
        header->rcode = no_error;
        header->ancount = htons(ancount);
        header->nscount = htons(nscount);
        header->arcount = htons(arcount);
    }

    return ret_code;
}

int compress(dns_rr **rr) {
    return 0;
}

int get_reff_ip(dns_info *query, char *reff_ip) {
    int8_t ar_records, an_counts, indx;
    int ret_code = -1;

    dns_rr *dns_addrecors;
    dns_opt_data *rdata;

    dns_addrecors = query->additional;
//    memset(reff_ip, MEM_RESET, sizeof(char) * IP_SIZE);
    ar_records = ntohs(query->packet.header.arcount);
    an_counts = 0;
    while (an_counts < ar_records) {
        if (dns_addrecors->type == 41 && dns_addrecors->rdlength > 4) {
            rdata = (dns_opt_data *) dns_addrecors->rdata;
            if (rdata->opt_code == 8) {
                dns_ecs_data *opt_data = (dns_ecs_data *) rdata->opt_data;
                for (indx = 0; indx < rdata->opt_length - 4; ++indx) {
                    reff_ip[indx] = *(opt_data->address + indx);
                }

                sprintf(reff_ip, "%u.%u.%u.%u", (unsigned char) reff_ip[0], (unsigned char)reff_ip[1], (unsigned char)reff_ip[2], (unsigned char)reff_ip[3]);
                ret_code = 0;
                break;
            }
        }

        dns_addrecors = dns_addrecors->next;
        ++an_counts;
    }

    return ret_code;
}

int get_dns_response(dns_info *const query, dns_ctx *app_ctx, common_ctx *var_ctx) {
    char reff_ip[IP_SIZE] = {0};
    uint16_t ar_records = ntohs(query->packet.header.arcount);
    int status_reff = -1;
    transaction_log *log = var_ctx->log;
    if (ar_records) {
        status_reff = get_reff_ip(query, reff_ip);
    }

    if (status_reff) {
        memcpy(reff_ip, query->conn_ip, strlen(query->conn_ip));
    }

    char *country_iso = app_ctx->geoip_handle(reff_ip, query->conn_ip, app_ctx->geoip_ctx, var_ctx);     // TODO check conn_ip == NULL, check country_iso
    uint16_t ancount = app_ctx->lookup_handle(query->questions, &query->answers, country_iso, app_ctx->lookup_ctx, var_ctx);

    log->question = strdup(query->questions->qname);
    log->conn_ip = strdup(query->conn_ip);
    log->reff_ip = strdup(reff_ip);
    log->country = strdup(country_iso);
    char *loganswers = log->answers = malloc(ancount * IP_SIZE);

    free(country_iso);

    make_header(&query->packet.header, ancount, 0, 0);

    unsigned char *scan = query->packet.buf;

    // skip header
    scan += 12;

    // skip question section
    while (scan[0]) {
        if (scan[0] & 0xc0) return -1;
        scan += (scan[0] + 1);
    }
    scan += 5;

    // append answer section
    dns_rr *answers = query->answers;
    char *name = NULL;
    while (ancount > 0 && answers != NULL) {
        name = answers->name;
        uint8_t count = 0;
        while (count < 63) {
            if (name[count] == '.') {
                scan[0] = count;
                count++;
                name += count;
                scan += count;
                count = 0;
            }
            else if (name[count] == '\0') {
                scan[0] = count;
                count++;
                scan[count] = 0x00;
                count++;
                scan += count;
                break;

            }
            else {
                scan[count + 1] = (unsigned char) name[count];
                count++;
            }
        }
        // append type
        scan[0] = (unsigned char) (answers->type & 0xff00);
        scan[1] = (unsigned char) (answers->type & 0x00ff);

        // append class
        scan[2] = (unsigned char) (answers->cls & 0xff00);
        scan[3] = (unsigned char) (answers->cls & 0x00ff);

        // append ttl
        scan[4] = (unsigned char) (answers->ttl.norm_ttl & 0xff000000);
        scan[5] = (unsigned char) (answers->ttl.norm_ttl & 0x00ff0000);
        scan[6] = (unsigned char) (answers->ttl.norm_ttl & 0x0000ff00);
        scan[7] = (unsigned char) (answers->ttl.norm_ttl & 0x000000ff);

        // append rdlength
        scan[8] = (unsigned char) (answers->rdlength & 0xff00);
        scan[9] = (unsigned char) (answers->rdlength & 0x00ff);
        scan += 10;

        // append rdata
        count = 0;
        name = answers->rdata;
        while (count < 127 && name[0] != '\0') {
            scan[count] = (unsigned char) atoi(name);
            count++;
            if ((name = strchr(name, '.')) == NULL) break;
            name += 1;
        }

        sprintf(loganswers, "%s", answers->rdata);
        if(ancount != 1){
            int len = strlen(answers->rdata);
            loganswers[len] = '\\';
            loganswers += (len + 1);

        }

        scan += count;
        ancount--;
        answers = answers->next;
    }

    return (int) (scan - query->packet.buf);
}

