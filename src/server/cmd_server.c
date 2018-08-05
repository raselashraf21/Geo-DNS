
#include <stdlib.h>
#include <ctype.h>
#include <config.h>
#include <unistd.h>

#include "cmd_server.h"
#include "server.h"
#include "string.h"
#include "stdio.h"

#define MAX_CONN 65535
#define MAX_FRAME_LEN 1024


typedef struct _cmp_options{

    char *long_name;
    int num_of_arg;
    char short_name;
}cmd_options;

 cmd_options cmd_opts[] = {
         {"help", 0, 'h'},
         {"reload", 0, 'r'},
         {"exit", 0, 'q'},
         {"q", 0, 'q'},
         {NULL, 0, 0}
 };

int str2lower(char *src){

    int indx;
    for(indx = 0; src[indx]; ++indx)
        src[indx] = tolower(src[indx]);

}

void *cmdrx_udp(void *srv_ptr){

    server_ctx *sctx = (server_ctx *) srv_ptr;

    int sock_fd;
    int ret;
    struct sockaddr_in server_addr;

    ret = init_socket(&sock_fd, sctx, SOCK_STREAM, tcp, &server_addr);

    if (ret != SUCCESS) {
        LM_PRINT(sctx->log_level , "%s", "[ERROR] scoket creation failed\n");
        return NULL;
    }

}

int process_cmdopts(char *argv, int argc, char *optarg){

    char *token = NULL;
    int cmd_optsize = sizeof(cmd_opts)/ sizeof(cmd_options);

    token = strtok(argv, " ");
    int indx ;
    for(indx =0; indx < cmd_optsize - 1; ++indx){
     //   printf("cmdName: %s \n", cmd_opts[indx].long_name);
        if(strcmp(token, cmd_opts[indx].long_name)==0){
            optarg = argv;
            return indx;
        }


    }


}

void show_message(char *msg){


        sprintf(msg, "%s", "\nUsage: <programm_name> [option] [option_value]\n"
                       "\t [-h | --help]\t\t\t: Show help.\n"
                       "\t [-c | --config] <path>\t: Configuration file path (absolute).\n\n");

}


void *cmdrx_tcp(void *srv_ptr){

    server_ctx *sctx = (server_ctx *) srv_ptr;
    server_app **app = (server_app **)sctx->log_conf;

    int sock_fd, client_fd;
    int ret, recv_len = 0;
    struct sockaddr_in server_addr, peer;
    int peer_len = sizeof(peer);

    ret = init_socket(&sock_fd, sctx, SOCK_STREAM, tcp, &server_addr);

    if (ret != SUCCESS) {
        LM_PRINT(sctx->log_level , "%s", "[ERROR] scoket creation failed\n");
        return NULL;
    }

    listen(sock_fd, MAX_CONN);

    char recv_buf[MAX_FRAME_LEN];

    char shell_prompt[] = "geodns->";
    int shell_prompt_len = strlen(shell_prompt);
    int cmd_indx;
    char *message;
    int pid;
    char pid_cmd[20];
    while(1){

        client_fd = accept(sock_fd, (struct sockaddr *) &(peer), &(peer_len));

        do{

            ret = send(client_fd, shell_prompt, shell_prompt_len,0);


            if((recv_len = recv(client_fd, recv_buf, MAX_FRAME_LEN, 0)) <=0){
                close(client_fd);
                break;
            }
             recv_buf[recv_len-2] = '\0';
             cmd_indx = process_cmdopts(recv_buf, recv_len, optarg);

            switch(cmd_opts[cmd_indx].short_name){

                case 'h':
                    message = malloc(sizeof(char) * MAX_FRAME_LEN);
                    show_message(message);
                    send(client_fd, message, strlen(message),0);
                    free(message);
                    break;

                case 'r':
                    pid = getpid();
                    sprintf(pid_cmd, "kill -1 %d",pid);
                    system(pid_cmd);
                    break;

                case 'q':
                        close(client_fd);
                         break;
                default:
                    message = malloc(sizeof(char) * MAX_FRAME_LEN);
                    sprintf(message, "%s", "commands are invalid \n");
                    send(client_fd, message, strlen(message),0);
                    free(message);
                    break;

            }

        }while(recv_len > 0);


        usleep(10);
    }


}

void init_cmdserver(server_app **app, char *proto_type, char *ip, uint16_t port){

    server_ctx *srv_ctx = (server_ctx *)malloc(sizeof(server_ctx));

    char *protocol = malloc(sizeof(char) * (strlen(proto_type) + 1));
    sprintf(protocol, "%s", proto_type);
    str2lower(protocol);

    srv_ctx->srv_type = cmd_server;
    srv_ctx->protocol = strcmp(protocol, "tcp") == 0?tcp:udp;
    srv_ctx->ip = ip;
    srv_ctx->port = port;
    srv_ctx->log_level = (*app)->srv_ctx->log_level;
    srv_ctx->log_conf = app;


    pthread_t tid;
    pthread_attr_t attr;

    init_thread_attr(&attr, 1024);

    if(srv_ctx->protocol == tcp)
        pthread_create(&tid, &attr, &cmdrx_tcp, srv_ctx);
    else
        pthread_create(&tid, &attr, &cmdrx_udp, srv_ctx);


}