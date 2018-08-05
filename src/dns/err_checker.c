#include <stdio.h>
#include "dns_packet.h"
#include "err_checker.h"
#include "dns_response.h"

#define TC_FLAG 0
#define Z_FLAG 0
#define QR_FLAG 0
#define OPT_FLAG 0

#define QTYPE_A 1
#define QTYPE_AAAA 28
#define QCLASS 1
#define QD_COUNT 1
#define ZONE_FOUND 1

// 'info' cannot be NULL. So before calling this function, check 'info' whether it is NULL or not.
error_list error_check(dns_info *info, error_chk_type hdr_type) {

    error_list ret = hdr_no_error;
    switch (hdr_type) {
        case type_header: {
            if (info->packet.header.qr != QR_FLAG) {
                info->packet.header.rcode = format_error;
                ret = err_QR_Flag;
            }
            else if (info->packet.header.opcode != OPT_FLAG) {
                info->packet.header.rcode = not_implemented;
                ret = err_OPT_Flag;
            }
            else if (info->packet.header.tc != TC_FLAG) {
                info->packet.header.rcode = format_error;
                ret = err_TC_Flag;
            }
            else if (info->packet.header.z != Z_FLAG) {
                info->packet.header.rcode = format_error;
                ret = err_Z_FLAG;
            }
            else if (info->packet.header.qdcount < QD_COUNT) {
                info->packet.header.rcode = format_error;
                ret = err_No_Question;
            }

            break;
        }
        case type_question: {
            if (info->questions->qtype != QTYPE_A && info->questions->qtype != QTYPE_AAAA) {
                info->packet.header.rcode = format_error;
                ret = err_QType_A;
            }
            else if (info->questions->qclass != QCLASS) {
                info->packet.header.rcode = format_error;
                ret = err_QClass;
            }
            else if (lookup_zone(info->questions->qname) != ZONE_FOUND) {
                info->packet.header.rcode = server_failure;
                ret = err_zone_file;
            }

            break;
        }
        case type_answer: {
            break;
        }
        case type_additional_records: {
            break;
        }
        default: {
            break;
        }
    }

    return ret;
}