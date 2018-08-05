
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "strutils.h"
#include "rtypes.h"
#include "utility/log/log.h"
#include "dns_packet.h"
#include "dns_packet_parser.h"

#define AD_ENABLED 1
#define NS_ENABLED 1

// ---------------------------- DNS Parser ----------------------------

// Parse the questions section of the dns protocol.
//  packet - the dns packet.
//  len - length of 'packet'.
//  pos - offset to the start of the questions section.
//  id_pos - offset set to the id field. Needed to decompress dns rdata.
//  count - Number of questions records to expect.
//  root - Pointer to where to store the questions records.
// Return 0 on error, the new 'pos' in the packet otherwise.
uint32_t parse_dns_question(const uint8_t *packet, uint32_t len, uint32_t pos, uint32_t id_pos, uint16_t count,
                            dns_question **root) {

    uint32_t start_pos = pos;
    dns_question *last = NULL;
    dns_question *current;
    uint16_t i;
    *root = NULL;

    for (i = 0; i < count; i++) {
        current = malloc(sizeof(dns_question));
        current->next = NULL;
        current->qname = NULL;

        current->qname = read_rr_name(packet, len, id_pos, &pos);
        if (current->qname == NULL || (pos + 2) >= len) {
            // Handle a bad DNS qname.
            fprintf(stderr, "DNS questions error\n");
            char *buffer = escape_data(packet, start_pos, len);
            const char *msg = "Bad DNS questions: ";
            current->qname = malloc(sizeof(char) * (strlen(buffer) +
                                                    strlen(msg) + 1));
            sprintf(current->qname, "%s%s", msg, buffer);
            free(buffer);
            current->qtype = 0;
            current->qclass = 0;
            if (last == NULL) *root = current;
            else last->next = current;
            return 0;
        }
        current->qtype = (packet[pos] << 8) + packet[pos + 1];
        current->qclass = (packet[pos + 2] << 8) + packet[pos + 3];

        // Add this questions object to the list.
        if (last == NULL) *root = current;
        else last->next = current;
        last = current;
        pos = pos + 4;
    }

    return pos;

}

// Parse an individual resource record, placing the acquired rdata in 'rr'.
// 'packet', 'pos', and 'id_pos' serve the same uses as in parse_rr_set.
// Return 0 on error, the new 'pos' in the packet otherwise.
uint32_t parse_rr(const uint8_t *packet, size_t len, uint32_t pos, uint32_t id_pos, dns_rr *rr) {
    int i;
    uint32_t rr_start = pos;
    rr_parser_container *parser;
    rr_parser_container opts_cont = {0, 0, opts};

    rr->name = NULL;
    rr->rdata = NULL;

    rr->name = read_rr_name(packet, len, id_pos, &pos);
    // Handle a bad rr qname.
    // We still want to print the rest of the escaped rr rdata.
    if (rr->name == NULL) {
        const char *msg = "Bad rr qname: ";
        rr->name = malloc(sizeof(char) * (strlen(msg) + 1));
        sprintf(rr->name, "%s", "Bad rr qname");
        rr->type = 0;
        rr->rr_name = NULL;
        rr->cls = 0;
        rr->ttl.norm_ttl = 0;
        rr->rdata = escape_data(packet, pos, len);
        return 0;
    }

    if ((len - pos) < 10) return 0;

    rr->type = (packet[pos] << 8) + packet[pos + 1];
    rr->rdlength = (packet[pos + 8] << 8) + packet[pos + 9];
    void **rdata;
    // Handle edns opt RR's differently.
    if (rr->type == 41) {
        rr->cls = (packet[pos + 2] << 8) + packet[pos + 3];
        rr->ttl.opt_ttl.extended_rcode = packet[pos + 4];
        rr->ttl.opt_ttl.version = packet[pos + 5];
        rr->ttl.opt_ttl.dnssec_ok = (unsigned) packet[pos + 6] & 0x80;
        rr->ttl.opt_ttl.z = (unsigned) (((packet[pos + 6] & 0x7f) << 8) + packet[pos + 7]);
        rr->rr_name = "OPTS";
        rdata = (void **) &(rr->rdata);
        parser = &opts_cont;
        // We'll leave the parsing of the special EDNS opt fields to
        // our opt rdata parser.
        pos = pos + 10;
    } else {
        // The normal case.
        rr->cls = (packet[pos + 2] << 8) + packet[pos + 3];
        rr->ttl.norm_ttl = 0;
        for (i = 0; i < 4; i++)
            rr->ttl.norm_ttl = (rr->ttl.norm_ttl << 8) + packet[pos + 4 + i];
        // Retrieve the correct parser function.
        parser = find_parser(rr->cls, rr->type);    // TODO optimize find_parser(...), check the loops inside it.
        rr->rr_name = parser->name;
        rdata = (void **) &(rr->rdata);
        pos = pos + 10;
    }

//    if (conf->MISSING_TYPE_WARNINGS && &default_rr_parser == parser)
    if (&default_rr_parser == parser)
        fprintf(stderr, "Missing parser for class %d, qtype %d\n",
                rr->cls, rr->type);

    // Make sure the rdata for the record is actually there.
    // If not, escape and print the raw rdata.
    if (len < (rr_start + 10 + rr->rdlength)) {
        char *buffer;
        const char *msg = "Truncated rr: ";
        rr->rdata = escape_data(packet, rr_start, len);
        buffer = malloc(sizeof(char) * (strlen(rr->rdata) + strlen(msg) + 1));
        _DEBUG printf(buffer, "%s%s", msg, rr->rdata);
        free(rr->rdata);
        rr->rdata = buffer;
        return 0;
    }
    // Parse the resource record rdata/rdata.
    *rdata = parser->parser(packet, pos, id_pos, rr->rdlength, len);

    return pos + rr->rdlength;
}

// Parse a set of resource records in the dns protocol in 'packet'
//  packet - the dns packet.
//  len - length of 'packet'.
//  pos - offset to the start of the rr section.
//  id_pos - offset set to the id field. Needed to decompress dns rdata.
//  count - Number of resource records of this qtype to expect.
//  root - Pointer to where to store the parsed list of objects.
// Return 0 on error, the new 'pos' in the packet otherwise.
uint32_t parse_rr_set(const uint8_t *packet, size_t len, uint32_t pos, uint32_t id_pos, uint16_t count,
                      dns_rr **root) {
    dns_rr *last = NULL;
    dns_rr *current;
    uint16_t i;
    *root = NULL;
    for (i = 0; i < count; i++) {
        // Create and clear the rdata in a new dns_rr object.
        current = malloc(sizeof(dns_rr));
        current->next = NULL;
        current->name = NULL;
        current->rdata = NULL;

        pos = parse_rr(packet, len, pos, id_pos, current);
        // If a non-recoverable error occurs when parsing an rr,
        // we can only return what we've got and give up.
        if (pos == 0) {
            if (last == NULL) *root = current;
            else last->next = current;
            return 0;
        }
        if (last == NULL) *root = current;
        else last->next = current;
        last = current;
    }
    return pos;
}

// Parse the dns protocol in 'packet'.
// See RFC1035
// See dns_packet_parser.h for more info.
// 'info' cannot be NULL. So before calling this function, check 'info' whether it is NULL or not.
uint32_t parse_dns_packet(dns_info *const info) {
    int pos = sizeof(info->packet.header);

    uint16_t qdcount = ntohs(info->packet.header.qdcount);
    uint16_t ancount = ntohs(info->packet.header.ancount);
    uint16_t nscount = ntohs(info->packet.header.nscount);
    uint16_t arcount = ntohs(info->packet.header.arcount);

    // Parse each qtype of records in turn.
    size_t id_pos = 0;
    pos = parse_dns_question(info->packet.buf, info->packet_length, pos, id_pos, qdcount, &(info->questions));
    if (pos != 0)
        pos = parse_rr_set(info->packet.buf, info->packet_length, pos, id_pos, ancount, &(info->answers));
    else info->answers = NULL;

    if (pos != 0 && (NS_ENABLED || AD_ENABLED)) { // && (conf->NS_ENABLED || conf->AD_ENABLED || force)) {
        pos = parse_rr_set(info->packet.buf, info->packet_length, pos, id_pos, nscount, &(info->name_servers));
    } else info->name_servers = NULL;

    if (pos != 0 && AD_ENABLED) { // && (conf->AD_ENABLED || force)) {
        pos = parse_rr_set(info->packet.buf, info->packet_length, pos, id_pos, arcount, &(info->additional));
    } else info->additional = NULL;

    return pos;
}
// end (DNS Parser) --------------------------------------------------------

// -------------- init/free functions --------------
dns_info *create_info() {
    dns_info *info = malloc(sizeof(dns_info));
    if (info) {
        info->packet_length = 0;
        info->questions = NULL;
        info->answers = NULL;
        info->name_servers = NULL;
        info->additional = NULL;
        info->conn_ip = NULL;
    }

    return info;
}

// TODO : this function should be a MACRO
void free_pointer(void **p) {
    if (*p) {
        free(*p);
        *p = NULL;
    }
}

uint8_t free_rr(dns_rr **p, uint16_t count) {
    dns_rr *current = *p, *next = NULL;
    uint8_t error_no = 0;
    for (int i = 0; i < count; ++i) {
        if (current == NULL) {
            error_no = -1;   // TODO(PTR_FREE_ERROR) indicates count mismatches to rr-count.
            break;
        }
        free_pointer((void **) &(current->name));
        if (current->type == 41 && current->rdlength >= 4) {    // opt type
            dns_opt_data *rdata = current->rdata;
            if (rdata->opt_code == 8 && rdata->opt_data) {
                dns_ecs_data *ecs = (dns_ecs_data *) rdata->opt_data;
                if (ecs->address) {
                    free_pointer((void **) &(ecs->address));
                }
                free_pointer((void **) &(rdata->opt_data));
            }
            else {
                free_pointer((void **) &(rdata->opt_data));
            }
            free_pointer((void **) &(current->rdata));
        }
        free_pointer((void **) &(current->rdata));
        next = current->next;
        free_pointer((void **) &current);
        current = next;
    }
    *p = NULL;

    return error_no;
}

uint8_t free_dns_packet(dns_info **info) {
    uint8_t error_no = 0;
    if (*info) {
        // freeing conn_ip pointer
        free_pointer((void **) &((*info)->conn_ip));

        // freeing question section.
        dns_question *qdcurrent = (*info)->questions, *qdnext = NULL;
        for (int qdcount = 0; qdcount < ntohs((*info)->packet.header.qdcount); ++qdcount) {
            if (qdcurrent == NULL) {
                error_no = -1;   // TODO(PTR_FREE_ERROR) indicates qdcount mismatches to question records.
                break;
            }
            free_pointer((void **) &(qdcurrent->qname));
            qdnext = qdcurrent->next;
            free_pointer((void **) &qdcurrent);
            qdcurrent = qdnext;
        }

        // TODO Check error_no for proper log.
        error_no = free_rr(&((*info)->answers), ntohs((*info)->packet.header.ancount));
        error_no = free_rr(&((*info)->name_servers), ntohs((*info)->packet.header.nscount));
        error_no = free_rr(&((*info)->additional), ntohs((*info)->packet.header.arcount));

        free_pointer((void **) info);
    }

    return error_no;
}
// end (init/free functions) ----------------------------

// -------------- Print Functions --------------
uint8_t print_dns_header(const dns_header *header) {
    if (!header)
        return -1;

    printf("\tid\t\t:0x%02x\n", (unsigned) ntohs(header->id));
    printf("\tqr\t\t:%u\n", (unsigned) header->qr);
    printf("\topcode\t:%u\n", (unsigned) header->opcode);
    printf("\taa\t\t:%u\n", (unsigned) header->aa);
    printf("\ttc\t\t:%u\n", (unsigned) header->tc);
    printf("\trd\t\t:%u\n", (unsigned) header->rd);
    printf("\tra\t\t:%u\n", (unsigned) header->ra);
    printf("\tz\t\t:%u\n", (unsigned) header->z);
    printf("\trcode\t:%u\n", (unsigned) header->rcode);
    printf("\tqdcount\t:%u\n", (unsigned) ntohs(header->qdcount));
    printf("\tancount\t:%u\n", (unsigned) ntohs(header->ancount));
    printf("\tnscount\t:%u\n", (unsigned) ntohs(header->nscount));
    printf("\tarcount\t:%u\n", (unsigned) ntohs(header->arcount));
    return 0;
}

uint8_t inspect_dns_header(const dns_header *header,  log_context *ctx){

    char msg[MAX_PACKET] = {0};
    snprintf(msg, MAX_PACKET, "DNS HEADR: \n\tid\t\t:0x%02x\n\tqr\t\t:%u\n \topcode\t:%u\n\taa\t\t:%u\n \ttc\t\t:%u\n\trd\t\t:%u\n\tra\t\t:%u\n\tz\t\t:%u\n\trcode\t:%u\n\tqdcount\t:%u\n\tancount\t:%u\n\tnscount\t:%u\n\tarcount\t:%u\n ",
             (unsigned) ntohs(header->id),
             (unsigned) header->qr,
             (unsigned) header->opcode,
             (unsigned) header->aa,
             (unsigned) header->tc,
             (unsigned) header->rd,
             (unsigned) header->ra,
             (unsigned) header->z,
             (unsigned) header->rcode,
             (unsigned) ntohs(header->qdcount),
             (unsigned) ntohs(header->ancount),
             (unsigned) ntohs(header->nscount),
             (unsigned) ntohs(header->arcount)
    );

    log_debug(ctx, "%s %s %d\n\t Header: %s", __FILE__, __FUNCTION__, __LINE__, msg);

    return 0;
}

uint8_t print_dns_question(const dns_question *question) {
    if (!question)
        return -1;

    printf("\tqname\t:%s\n", question->qname);
    printf("\tqtype\t:%u\n", (unsigned) question->qtype);
    printf("\tclass\t:%u\n", (unsigned) question->qclass);
    printf("\n");

    print_dns_question(question->next);

    return 0;
}


uint8_t inspect_dns_question(const dns_question *question, log_context *ctx ){
    if (!question)
        return -1;


    char msg[MAX_PACKET] = {0};
    static int8_t qcount = 0;

    sprintf(msg , "DNS QUESTION_%d: \tqname\t:%s\n \tqtype\t:%u\n \tclass\t:%u\n", ++qcount, question->qname,(unsigned) question->qtype,(unsigned) question->qclass );
    log_debug(ctx, "%s %s %d\n\t Header: %s", __FILE__, __FUNCTION__, __LINE__, msg);

    inspect_dns_question(question->next, ctx);

    return 0;
}

uint8_t print_dns_rr(const dns_rr *rr) {
    if (!rr)
        return -1;

    printf("\tqname\t\t:%s\n", rr->name);
    printf("\tqtype\t\t:%u\n", (unsigned) rr->type);
    if (rr->type == 41) {
        dns_opt_data *rdata = (dns_opt_data *) rr->rdata;
        printf("\tclass\t\t:%u (requestor's UDP payload size)\n", (unsigned) rr->cls);
        printf("\trr_name\t\t:%s\n", rr->rr_name);
        printf("\textended_rcode :%u\n", (unsigned) rr->ttl.opt_ttl.extended_rcode);
        printf("\tversion\t\t:%u\n", (unsigned) rr->ttl.opt_ttl.version);
        printf("\tdnssec_ok\t:%u\n", (unsigned) rr->ttl.opt_ttl.dnssec_ok);
        printf("\tz\t\t\t:%u\n", (unsigned) rr->ttl.opt_ttl.z);
        printf("\trdlength\t:%u\n", (unsigned) rr->rdlength);
        if (rr->rdlength >= 4 && rr->type == 41) {
            printf("\topt_code\t:%u\n", (unsigned) rdata->opt_code);
            printf("\topt_length\t:%u\n", (unsigned) rdata->opt_length);
            if (rdata->opt_length >= 4 && rdata->opt_code == 8) {
                printf("\topt_data\t:\n");
                dns_ecs_data *opt_data = (dns_ecs_data *) rdata->opt_data;
                printf("\t\tecs_family\t\t:%u\n", opt_data->family);
                printf("\t\tsource_pre_len\t:%u\n", opt_data->source_pre_len);
                printf("\t\tscope_pre_len\t:%u\n", opt_data->scope_pre_len);
                printf("\t\taddress\t\t\t: ");
                for (int i = 0; i < rdata->opt_length - 4; ++i) {
                    printf("%d ", (unsigned char )*(opt_data->address + i));
                }
                printf("\n");
            }
            else {
                printf("\topt_data\t:%s\n", (char *) rdata->opt_data);
            }
        }
        else {
            printf("\trcode\t:%s\n", (char *) rr->rdata);
        }
    }
    else {
        printf("\tclass\t\t:%u\n", (unsigned) rr->cls);
        printf("\trr_name\t\t:%s\n", rr->rr_name);
        printf("\tttl\t\t\t:%u\n", (unsigned) rr->ttl.norm_ttl);
        printf("\trdlength\t:%u\n", (unsigned) rr->rdlength);
        printf("\trdata\t\t:%s\n", (char *) rr->rdata);
    }
    printf("\n");

    print_dns_rr(rr->next);

    return 0;
}


uint8_t inspect_dns_answer(const dns_rr *rr, log_context *ctx) {
    if (!rr)
        return -1;

    char msg[4000];

    sprintf(msg, "\tqname\t\t:%s\n \tqtype\t\t:%u\n", rr->name,(unsigned) rr->type );

    if (rr->type == 41) {
        dns_opt_data *rdata = (dns_opt_data *) rr->rdata;
       sprintf(msg, "%s\tclass\t\t:%u (requestor's UDP payload size)\n\trr_name\t\t:%s\n\textended_rcode :%u\n\tversion\t\t:%u\n\tdnssec_ok\t:%u\n\tz\t\t\t:%u\n\trdlength\t:%u\n", msg,
                (unsigned) rr->cls,
                rr->rr_name,
                (unsigned) rr->ttl.opt_ttl.extended_rcode,
                (unsigned) rr->ttl.opt_ttl.version,
                (unsigned) rr->ttl.opt_ttl.dnssec_ok,
                (unsigned) rr->ttl.opt_ttl.z,
                (unsigned) rr->rdlength
        );

        if (rr->rdlength >= 4 && rr->type == 41) {
             sprintf(msg, "%s\topt_code\t:%u\n\topt_length\t:%u\n", msg,(unsigned) rdata->opt_code,(unsigned) rdata->opt_length);

            if (rdata->opt_length >= 4 && rdata->opt_code == 8) {
                sprintf(msg, "%s\topt_data\t:\n", msg);
                dns_ecs_data *opt_data = (dns_ecs_data *) rdata->opt_data;

                sprintf(msg, "%s\t\tecs_family\t\t:%u\n\t\tsource_pre_len\t:%u\n\t\tscope_pre_len\t:%u\n\t\taddress\t\t\t: ", msg,opt_data->family,opt_data->source_pre_len,opt_data->scope_pre_len );

                for (int i = 0; i < rdata->opt_length - 4; ++i) {
                    sprintf(msg, "%s %d", msg, *(opt_data->address + i));
                }

            }
            else {
                sprintf(msg,"%s\topt_data\t:%s\n", msg, (char *) rdata->opt_data );
            }
        }
        else {
            sprintf(msg, "%s\trcode\t:%s\n", msg, (char *) rr->rdata);

        }
    }
    else {

        sprintf(msg, "%s\tclass\t\t:%u\n\trr_name\t\t:%s\n\tttl\t\t\t:%u\n\trdlength\t:%u\n\trdata\t\t:%s\n", msg, (unsigned) rr->cls, rr->rr_name, (unsigned) rr->ttl.norm_ttl, (unsigned) rr->rdlength, (char *) rr->rdata);

    }

    log_debug(ctx, "%s %s %d\n\t Header: %s", __FILE__, __FUNCTION__, __LINE__, msg);


    inspect_dns_answer(rr->next, ctx);

    return 0;
}

uint8_t print_dns_packet(const dns_info *info) {
    if (!info)
        return -1;

    printf("\nDNS Header\n");
    print_dns_header(&info->packet.header);
    printf("\nDNS Questions\n");
    print_dns_question(info->questions);
    printf("\nDNS Answers\n");
    print_dns_rr(info->answers);
    printf("\nDNS NS\n");
    print_dns_rr(info->name_servers);
    printf("\nDNS Additionals\n");
    print_dns_rr(info->additional);

    return 0;
}

uint8_t inspect_dns_packet(const dns_info *info, log_context *ctx) {

    inspect_dns_header(&info->packet.header, ctx);
    inspect_dns_question(info->questions, ctx);
    inspect_dns_answer(info->answers, ctx);
    inspect_dns_answer(info->name_servers, ctx);
    inspect_dns_answer(info->additional, ctx);

    return 0;
}
// end (Print Funtions) ----------------------------

