#ifndef GEODNS_ERR_CHECKER_H
#define GEODNS_ERR_CHECKER_H


#include "dns_packet.h"

typedef enum _error_list{

    err_null_buffer = -1,
    hdr_no_error,
    err_QR_Flag,
    err_OPT_Flag,
    err_TC_Flag,
    err_Z_FLAG,
    err_No_Question,
    err_QClass,
    err_QType_A,
    err_QType_AAAA,
    err_zone_file,


} error_list;

typedef enum _error_chk_type {
    type_header,
    type_question,
    type_answer,
    type_additional_records
} error_chk_type;

error_list error_check(dns_info *, error_chk_type);

#endif // GEODNS_ERR_CHECKER_H
