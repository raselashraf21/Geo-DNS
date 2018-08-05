
#ifndef GEODNS_DNS_LOG_H
#define GEODNS_DNS_LOG_H

#include <utility/log/log.h>

typedef struct _transaction_log {
    const void *class;  /* Must be first, must not use it. */
    char *fmt;          /* It will be set by constructor, so don't change this manually. */

    /*****************************************************/
    /*!
     * Below members should be assigned malloc-ed string,
     * because they will be freed by destructor
     * */
    char *question;
    char *conn_ip;
    char *reff_ip;
    char *country;
    char *answers;
    /*****************************************************/

    /* time since the Epoch, 1970-01-01 00:00:00 +0000 (UTC). */
    double time_req;
    double time_processing;

    log_context *ctx;       // not owner
} transaction_log;
extern const void *TransactionLog;

void transaction_info(transaction_log *log);

#endif // GEODNS_DNS_LOG_H
