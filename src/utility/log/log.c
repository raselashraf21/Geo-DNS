
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "log.h"
#include "utility/new.h"



/********** Private Functions **********/
static void _get_unique_id(char *id) {
    snprintf(id, MAX_TXN_LENGTH, "%d%zu", rand() % 900000 + 100000, time(NULL));
}


/********** Object Implementation for type LogContext(log_context) **********/

static void *log_ctx_ctor (void * _self, size_t argc, va_list *argv)
{
    log_context *self = _self;
    _get_unique_id(self->txn_id);
    self->txn_prefix = "txn";

    return self;
}

static void *log_ctx_dtor (void * _self)
{
    log_context *self = _self;
    // TODO: Add code here for constructor
    return self;
}

static const struct Class _LogContext = {
        sizeof(log_context),
        log_ctx_ctor,
        log_ctx_dtor
};
const void *LogContext = &_LogContext;

