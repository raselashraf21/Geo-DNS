
#include <stdio.h>
#include <stdlib.h>

#include "utility/new.h"

#include "dns_log.h"


/********** Object Implementation for type LogContext(log_context) **********/

static void *transaction_log_ctor(void *_self, size_t argc, va_list *argv) {
    transaction_log *self = _self;

//    self->fmt = "\"%s\" \"%s\" \"%s\" \"%s\" \"%s\" %f %f";
    self->fmt = "%s %s %s %s %s %f %f";

    return self;
}

static void *transaction_log_dtor(void *_self) {
    transaction_log *self = _self;

    if (self->question) free(self->question);
    if (self->conn_ip) free(self->conn_ip);
    if (self->reff_ip) free(self->reff_ip);
    if (self->country) free(self->country);
    if (self->answers) free(self->answers);

    return self;
}

static const struct Class _TransactionLog = {
        sizeof(transaction_log),
        transaction_log_ctor,
        transaction_log_dtor
};
const void *TransactionLog = &_TransactionLog;


void transaction_info(transaction_log *log) {
    log_info(log->ctx, log->fmt,
             log->question ? log->question : "-",
             log->conn_ip ? log->conn_ip : "-",
             log->reff_ip ? log->reff_ip : "-",
             log->country ? log->country : "-",
             log->answers ? log->answers : "-",
             log->time_req, log->time_processing);
}
