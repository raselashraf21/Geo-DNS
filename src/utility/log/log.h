
#ifndef GEODNS_LOG_H
#define GEODNS_LOG_H
#include "p99_id.h"

#define MAX_TXN_LENGTH 20

/*!
 * INTERFACE: Should be implemented.
 *            Example implementation can be found in 'log_syslog.c'
 * NOTE:      'ConfLog'(log_app in log_context type) should be allocated before
 *            calling any log function, e.g., 'log_debug', 'log_info' etc, and
 *            same 'log_app' should be assigned in each newly created log_context.
 * USAGE:
 *            void *log_app = new(ConfLog, NULL);
 *            ...
 *            ...
 *            log_context *ctx = new(LogContext, NULL);
 *            ctx->log_app = log_app;
 *            ...
 *            log_info(ctx, "Hello %s", "Bangladesh!");
 *            log_info(ctx, "Hello %s", "NoText!");
 *            ...
 *            delete(ctx)
 *            ...
 *            ...
 *            delete(log_app)
 *
 */
extern const void *ConfLog;


/*!
 * WARNING: Don't manually create 'log_context'.
 * USAGE:
 *          log_context *ctx = new(LogContext, NULL);
 *          ...
 *          ...
 *          delete(ctx)
 *
 */
typedef struct _log_context {
    const void * class;             /* Must be first, must not use it */
    char txn_id[MAX_TXN_LENGTH];    /* It will be set by constructor everytime for newly created 'log_context',
                                     * so don't change this manually.
                                     */
    char *txn_prefix;               /* Cannot be changed now. Default is 'txn' */

    void *log_app;
} log_context;
extern const void *LogContext;


#define FORMAT "%s %s  %d"
#define arguments __FILE__,__FUNCTION__, __LINE__


#define LM_ERR(  level, log_contxt,fmt, ...) \
			do { \
			    if(level & L_ERR)\
                    log_error(log_contxt, FORMAT fmt,arguments, ##__VA_ARGS__);\
            }while(0)

#define LM_DEBUG(  level, log_contxt,  fmt, ...) \
			do { \
			    if(level & L_ERR)\
                    log_debug(log_contxt, FORMAT  fmt, arguments, ##__VA_ARGS__);\
            }while(0)

#define LM_INFO(  level, log_contxt,  fmt, ...) \
			do { \
			    if(level & L_ERR)\
                    log_info(log_contxt, FORMAT fmt,arguments, ##__VA_ARGS__);\
            }while(0)

#define LM_WARNING(  level, log_contxt,  fmt, ...) \
			do { \
			    if(level & L_ERR)\
                    log_warnings(log_contxt, FORMAT  fmt, arguments,##__VA_ARGS__);\
            }while(0)

#define LM_CRITICAL(  level, log_contxt,fmt, ...) \
			do { \
			    if(level & L_ERR)\
                    log_critical(log_contxt, FORMAT fmt, arguments, ##__VA_ARGS__);\
            }while(0)

#define LM_PRINT( level, fmt, ...) \
			do { \
			    if(level & L_ERR)\
                    fprintf(stdout, fmt, ##__VA_ARGS__);\
            }while(0)


/*!
 * INTERFACE: Should be implemented.
 *            Example implementation can be found in 'log_syslog.c'
 */
int log_debug(log_context *ctx, const char *fmt, ...);
int log_info(log_context *ctx, const char *fmt, ...);
int log_error(log_context *ctx, const char *fmt, ...);
int log_warning(log_context *ctx, const char *fmt, ...);
int log_critical(log_context *ctx, const char *fmt, ...);

#endif // GEODNS_LOG_H
