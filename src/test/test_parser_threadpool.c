
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif



#include "config.h"
#include "factory/factory.h"
#include "server/server.h"
#include "server/thread_pool.h"

#include <unistd.h>
#include <server/thread_pool.h>

int test_parse_threadpool()
{
//    const char msg_hex[] ="16b50100000100000000000006676f6f676c6503636f6d0000010001";   // Quesion: Type A(1), single
//    const char msg_hex[]="16b50100000200000000000006676f6f676c6503636f6d0000010001"
//                                                 "06676f6f676c6503636f6d0000010001";    // Quesion: Type A(1), double
//    const char msg_hex[]="17758180000100010000000006676f6f676c6503636f6d0000010001"
//                                                 "c00c000100010000007d0004d83ac54e";    // Response: Type A(1), single
//    const char msg_hex[]="a56a0100000100000000000002373803313937023538033231360769"
//                                                 "6e2d61646472046172706100000c0001";    // Question: Type PTR(12)
//    const char msg_hex[]="a55a8180000100010000000002373803313937023538033231360769"
//                         "6e2d61646472046172706100000c0001c00c000c00010000545f001b"
//                           "0f6d616130337332312d696e2d663134053165313030036e657400";    // Response: Type PTR(12)
//    const char msg_hex[]="0be40100000100000000000008636c69656e74733406676f6f676c65"
//                                                               "03636f6d0000010001";    // Request: Type A(1)
//    const char msg_hex[]="0be48180000100020000000008636c69656e74733406676f6f676c65"
//                         "03636f6d0000010001c00c000500010000010b000c07636c69656e74"
//                                       "73016cc015c031000100010000010b0004d83ac54e";    // Response: Type
    const char msg_hex[] = "247400000001000000000001016308697076636c6f75640672696e"
                           "67696403636f6d0000010001000029100000008000000b00080007"
//                           "67696403636f6d0000010001000029100000008000000b00080007"     // Ashraf-Test
                                                                   "00011800671a70";    // Request: Type A(1) with opts (ecs)




    //const char msg_hex[] = "0xc0c07b0002008bd24a7dbe9200000000000000001000000020c17b0000000000a85c4000000000003d0000000000000000000000000000001800671a70";

    /*  // Ashraf-Test
        char msg_hex[] = "2474\ -- id
                0000\   -- flags
                0001\   --  questions
                0000\   -- answers
                0000\   -- authority rr
                0001\   -- additional rr
                016308697076636c6f75640672696e67696403636f6d00\ -- testcloud.com
                0001\   -- qtype
                0001\   -- qclass
                00\     -- root name [additinal]
                0029\   -- opt type
                1000\   -- upd payload size
                00\     -- extened rCode
                00\     -- endnso version
                8000\   -- z
                000b\   -- data length
                0008\   -- option code (CSUBNET)
                0007\   -- option length
                0001\   -- Family
                18\     -- source netmask
                00\     -- scope netmask
                671a70";
    */

    _DEBUG {
        printf("Packet Size: %lu\n", strlen(msg_hex));
        if (strlen(msg_hex) % 2 == 1)
        {
            printf("Odd length of hex-string. Should be even length");
            return 0;
        }
        printf("0x%s\n", msg_hex);
    }

    const char *pos = msg_hex;

    // dummy packet creation
    uint8_t buf[MAX_PACKET];
    memset(buf, 0, sizeof(buf));
    for(size_t count = 0; count < strlen(msg_hex)/2; count++) {
        sscanf(pos, "%2hhx", (buf + count));
        pos += 2;
    }

    // server_app creation
    server_app *app;
    make_app((void **)&app);

    // server_app initialization
    app->initialize(app->srv_ctx, app->app_ctx);

    // server_app's thread_ctx initialization
    thread_ctx *tctx = calloc(1, sizeof(thread_ctx));
    app->initialize_thd_ctx(app->srv_ctx, app->app_ctx, tctx);

/*********** [TEST] check packet parsing with threadpool ***********/
#if FUNCTION_TEST

    int count=0;
    struct timespec tstart={0,0}, tend={0,0};
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    tstart.tv_sec = mts.tv_sec;
    tstart.tv_nsec = mts.tv_nsec;

#else
    clock_gettime(CLOCK_MONOTONIC, &tstart);
#endif

    for(count =0; count < LOOP_COUNT; ++count) {

        // dummy net_node creation
        net_element *net_node = new(NetElement, app);
        net_node->id = 0;
        net_node->th_status = 0;
        net_node->sock_fd = -1;
        net_node->rcv_type = 0;
        net_node->peer.sin_family = AF_INET;
        net_node->peer.sin_port = htons(8888);
        net_node->peer.sin_addr.s_addr = htonl(INADDR_ANY);
        net_node->peer_len = sizeof(net_node->peer);
        net_node->srv_ctx = app->srv_ctx;
        net_node->app_ctx = app->app_ctx;
        char *info = app->get_buffer(MAX_PACKET);
        memcpy(info, buf, sizeof(buf));
        net_node->rcv_buf = (char *)info;
        net_node->rcv_buf_len = strlen(msg_hex)/2;
        net_node->handle_buf = app->handle_request;

        net_node->handle_buf(net_node);
    }

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock2;
    mach_timespec_t mts2;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock2);
    clock_get_time(cclock2, &mts2);
    mach_port_deallocate(mach_task_self(), cclock2);
    tend.tv_sec = mts2.tv_sec;
    tend.tv_nsec = mts2.tv_nsec;

#else
    clock_gettime(CLOCK_MONOTONIC, &tend);
#endif

#elif THREAD_TEST

    int count=0;
    struct timespec tstart={0,0}, tend={0,0};

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    tstart.tv_sec = mts.tv_sec;
    tstart.tv_nsec = mts.tv_nsec;

#else
    clock_gettime(CLOCK_MONOTONIC, &tstart);
#endif

    _thread_pool *thread_pool = create_thread_pool(THREAD_NUMBER, QUEUE_SIZE);
    if(thread_pool == NULL)
        printf("error in creating thread pool\n");

    for(count =0; count < LOOP_COUNT; ++count) {

        // dummy net_node creation
        net_element *net_node = new(NetElement, app);
        net_node->id = 0;
        net_node->th_status = 0;
        net_node->sock_fd = -1;
        net_node->rcv_type = 0;
        net_node->peer.sin_family = AF_INET;
        net_node->peer.sin_port = htons(8888);
        net_node->peer.sin_addr.s_addr = htonl(INADDR_ANY);
        net_node->peer_len = sizeof(net_node->peer);
        net_node->srv_ctx = app->srv_ctx;
        net_node->app_ctx = app->app_ctx;
        char *info = app->get_buffer(MAX_PACKET);
        memcpy(info, buf, sizeof(buf));
        net_node->rcv_buf = (char *)info;
        net_node->rcv_buf_len = strlen(msg_hex)/2;
        net_node->handle_buf = app->handle_request;

        append_job(thread_pool, (void *) net_node, NULL);
    }

    while(thread_pool->queue_node->qsize != 0)
        ;
    clock_gettime(CLOCK_MONOTONIC, &tend);

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock2;
    mach_timespec_t mts2;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock2);
    clock_get_time(cclock2, &mts2);
    mach_port_deallocate(mach_task_self(), cclock2);
    tend.tv_sec = mts2.tv_sec;
    tend.tv_nsec = mts2.tv_nsec;

#else
    clock_gettime(CLOCK_MONOTONIC, &tend);
#endif

#endif
/*******************************************************************/

    // server_app de-initialization
    app->destroy(app->app_ctx);

    // server_app's thread_ctx initialization
    app->destroy_thd_ctx(tctx);
    free(tctx);

    // free-up server_app
    free_app((void **)&app);

    printf("time taken  about %.5f seconds\n",
           ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
           ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));


    return 0;
}