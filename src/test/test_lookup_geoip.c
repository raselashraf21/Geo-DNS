
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server/server.h"
#include "dns/dns_app.h"
#include "factory/factory.h"

int test_lookup_geoip() {

    // server_app creation
    server_app *app;
    make_app((void **) &app, NULL);

    // server_app initialization
    app->initialize(app->srv_ctx, app->app_ctx);

    // server_app's thread_ctx initialization
    thread_ctx *tctx = calloc(1, sizeof(thread_ctx));
    app->initialize_thd_ctx(app->srv_ctx, app->app_ctx, tctx);

    dns_ctx *dctx = (dns_ctx *)app->app_ctx;



/**************  [TEST] check geoip  **************/
    char *ref_ip = malloc(sizeof(char) * 16);
    char *con_ip = malloc(sizeof(char) * 16);
    char *result_iso;

    /*TEST CASE-1:- ref_ip = NULL and conn_ip = NULL*/
    //result_iso = dctx->geoip_handle(NULL, NULL, dctx->geoip_ctx);

    /*TEST CASE-2:- ref_ip = NULL and conn_ip = some_ip_string*/
    /*strcpy(con_ip, "1.0.4.0");
    result_iso = dctx->geoip_handle(NULL, con_ip, dctx->geoip_ctx);*/


    /*TEST CASE-3:- ref_ip = some_ip_string and conn_ip = NULL*/
    /*strcpy(ref_ip, "103.26.112.17");
    result_iso = dctx->geoip_handle(ref_ip, NULL, dctx->geoip_ctx);*/

    /*TEST CASE-4:- ref_ip = some_ip_string and conn_ip = some_ip_string*/
    strcpy(ref_ip, "103.26.112.17");      //regular IP
//    strcpy(ref_ip, "2.22.234.0");           //continent 'EU'
//    strcpy(ref_ip, "5.56.16.0");            //Anonymous Proxy A1
//    strcpy(ref_ip, "2.111.70.28");             //Anonymous Proxy A1
//    strcpy(ref_ip, "5.11.17.0");          //Satellite Provider A2


//    strcpy(ref_ip, "255.255.255.255");
    strcpy(con_ip, "2.16.0.0");
    result_iso = dctx->geoip_handle(ref_ip, con_ip, dctx->geoip_ctx, NULL);


    if(result_iso == NULL) {
        printf("no value for ref_ip or conn_ip was found\n");
    }
    else {
        printf("[-----------------here '$$' means no entry found for that ip-----------------]\n");
        printf("country iso --> %s\n", result_iso);
    }
    free(ref_ip);
    free(con_ip);
    free(result_iso);
/***************************************************/



    // server_app de-initialization
    app->destroy(app->app_ctx);

    // server_app's thread_ctx initialization
    app->destroy_thd_ctx(tctx);
    free(tctx);

    // free-up server_app
    free_app((void **)&app);

    return 0;
}