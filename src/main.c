
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <server/cmd_server.h>
#include <signal.h>
#include <unistd.h>
#include <factory/factory.h>
#include <lookup/geoip.h>

#include "config.h"
#include "utility/log/log.h"

#if FLAG_ENABLE_TEST
    #include "test/test.h"
#else
    #include "server/server.h"
    #include "factory/factory.h"
#endif  // enable_test


/*usage function*/
void show_usage() {
    printf("\nUsage: <programm_name> [option] [option_value]\n"
                   "\t [-h | --help]\t\t\t: Show help.\n"
                   "\t [-c | --config] <path>\t: Configuration file path (absolute).\n\n");
}

const struct option long_option[] = {
        {"help", 0, NULL, 'h'},
        {"config", 1, NULL, 'c'},
        {NULL, 0, NULL, 0}                  /*must keep an extra one at the end*/
};

volatile server_app *sapp = NULL;
volatile  char *conf_path = NULL;

void run_server_app(){
    server_app *app = NULL;
    make_app((void **) &app, conf_path);
    run_server(app);
    sleep(1);

    if(sapp == NULL){
        #if CMD_SERVER_ENABLE
            init_cmdserver(&app, CMD_SERVER_TYPE, CMD_SERVER_IP, CMD_SERVER_PORT);
        #endif
        sapp = app;
   }
    else{
        sapp->srv_term = 1;
        sleep(2);
        thread_ctx *tctx = sapp->srv_ctx->th_ctx;

        for(int indx =0; indx < tctx->thread_count; ++indx){
            if(tctx->thread_term[indx] == false)
              pthread_cancel(tctx->thread_id[indx]);
        }
        free_app((void **)&sapp);
        sapp = app;
    }
}

void *sig_handler(void *arg) {
    sigset_t *set = arg;
    int s, sig;
    pthread_attr_t attr;
    init_thread_attr(&attr, 1024);

    for (;;) {
        s = sigwait(set, &sig);
        if (s != 0)
            perror("sigwait");
        run_server_app();
    }
}

int main(int argc, char **argv) {
    if(argc < 3) {
        show_usage();
    }

    int option;
    while((option = getopt_long(argc, argv, "hc:", long_option, NULL)) != -1) {
        switch (option) {
            case 'h':
                show_usage();
                break;
            case 'c':
                conf_path = strdup(argv[optind - 1]);
                printf("Configuration file path --> %s\n", conf_path);
                break;
            case '?':
                show_usage();
                break;
            default:
                // ignore
                break;
        }
    }

    if (conf_path == NULL) {
        LM_PRINT(L_ALL, "No configuration file path provided as argument.\n"
                "\t\"%s\" (default) will be used.\n\n", DEFAULT_PATH_CONF_INI);
        conf_path = strdup(DEFAULT_PATH_CONF_INI);
    }

    #if FLAG_ENABLE_TEST
        test();
    #else
        // signal handler thread creation
        pthread_t thread;
        sigset_t set;
        int s;
        sigemptyset(&set);
        sigaddset(&set, SIGHUP);

        s = pthread_sigmask(SIG_BLOCK, &set, NULL);
        if (s != 0)
            LM_PRINT(L_ALL, "Error in pthread sigmask\n");

        s = pthread_create(&thread, NULL, &sig_handler, (void *) &set);
        // send signal to start server for the first time.
        int pid = getpid();
        char cmd[20];
        sprintf(cmd, "kill -1 %d", pid);
        system(cmd);
        pause();
    #endif  // enable_test

    if (conf_path) {
        free(conf_path);
    }
    return 0;
}