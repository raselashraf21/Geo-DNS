
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "log.h"
#include "utility/new.h"

#define MAX_FORMAT_SIZE 100


/********** Object Implementation for type ConfLog(conf_log) **********/
typedef struct _conf_log {
    const void * class; /* must be first */
    char *ident;        // used for syslog
    int option;         // used for syslog
    int facility;       // used for syslog
} conf_log;

static void *conf_log_ctor (void * _self, size_t argc, va_list *argv)
{
    // This is needed for random number generation needed in log_context
    time_t t;
    srand(time(&t));

    conf_log *self = _self;
    int count = argc;

    const char *text = va_arg(*argv, const char *);
    if (text && strlen(text)) {
        self->ident = malloc(strlen(text) + 1);
        assert(self->ident);
        strcpy(self->ident, text);
    }
    else {
        self->ident = NULL;
    }
    count--;
    if (count <= 0) {
        goto SET_DEFAULT_OPTION;
    }

    self->option = va_arg(*argv, int);
    count--;
    if (count <= 0) {
        goto SET_DEFAULT_FACILITY;
    }

    self->facility = va_arg(*argv, int);
    goto EXIT;

    SET_DEFAULT_OPTION: {
        self->option = LOG_CONS | LOG_PID | LOG_NDELAY;
    };

    SET_DEFAULT_FACILITY: {
        self->facility = LOG_LOCAL0;
    };

    EXIT: {
        openlog(self->ident, self->option, self->facility);

        printf("Syslog has been opened with congiguration - (%s, %d, %d)\n", self->ident, self->option, self->facility);
        syslog(LOG_INFO, "Syslog has been opened with congiguration - (%s, %d, %d)", self->ident, self->option, self->facility);

        sleep(1);   // Wait for syslog-ready. If we would not give this, some logs are lost. Why?

        return self;
    };
}

static void *conf_log_dtor (void * _self)
{
    conf_log * self = _self;
    if (self->ident) {
        free(self->ident), self->ident = 0;
    }
    printf("Closing syslog ...");
    syslog(LOG_INFO, "Closing syslog ...");
    closelog();
    return self;
}

static const struct Class _ConfLog = {
        sizeof(conf_log),
        conf_log_ctor,
        conf_log_dtor
};
const void *ConfLog = &_ConfLog;



/********** Public Functions **********/
int log_debug(log_context *ctx, const char *fmt, ...) {
    char format[MAX_FORMAT_SIZE + 1];
    if (ctx) {
        snprintf(format, MAX_FORMAT_SIZE, "%s (%s%s)", fmt, ctx->txn_prefix, ctx->txn_id);
    }
    else {
        snprintf(format, MAX_FORMAT_SIZE, "%s (-)", fmt);
    }

    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, format, ap);
    va_end(ap);
    return 0;
}

int log_info(log_context *ctx, const char *fmt, ...) {
    char format[MAX_FORMAT_SIZE + 1];
    if (ctx) {
        snprintf(format, MAX_FORMAT_SIZE, "%s (%s%s)", fmt, ctx->txn_prefix, ctx->txn_id);
    }
    else {
        snprintf(format, MAX_FORMAT_SIZE, "%s (-)", fmt);
    }

    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_INFO, format, ap);
    va_end(ap);

    return  0;
}

int  log_error(log_context *ctx, const char *fmt, ...) {
    char format[MAX_FORMAT_SIZE + 1];
    if (ctx) {
        snprintf(format, MAX_FORMAT_SIZE, "%s (%s%s)", fmt, ctx->txn_prefix, ctx->txn_id);
    }
    else {
        snprintf(format, MAX_FORMAT_SIZE, "%s (-)", fmt);
    }

    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_ERR, format, ap);
    va_end(ap);

    return 0;
}

int  log_warning(log_context *ctx, const char *fmt, ...) {
    char format[MAX_FORMAT_SIZE + 1];
    if (ctx) {
        snprintf(format, MAX_FORMAT_SIZE, "%s (%s%s)", fmt, ctx->txn_prefix, ctx->txn_id);
    }
    else {
        snprintf(format, MAX_FORMAT_SIZE, "%s (-)", fmt);
    }

    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_WARNING, format, ap);
    va_end(ap);

    return 0;
}

int  log_critical(log_context *ctx, const char *fmt, ...) {
    char format[MAX_FORMAT_SIZE + 1];
    if (ctx) {
        snprintf(format, MAX_FORMAT_SIZE, "%s (%s%s)", fmt, ctx->txn_prefix, ctx->txn_id);
    }
    else {
        snprintf(format, MAX_FORMAT_SIZE, "%s (-)", fmt);
    }

    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_CRIT, format, ap);
    va_end(ap);

    return 0;
}

