
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <server/socklib.h>

#include "dns/dns_packet.h"
#include "dns/dns_packet_parser.h"
#include "dns/dns_response.h"
#include "dns/err_checker.h"
#include "server/socklib.h"

int test_parse()
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
//                           "67696403636f6d0000010002000029100000008000000b00080007"     // Ashraf-Test
                                                                   "00011800671a70";    // Request: Type A(1) with opts (ecs)
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

////    printf("size %lu\n", strlen(msg_hex));
//    if (strlen(msg_hex) % 2 == 1)
//    {
//        printf("Odd length of hex-string. Should be even length");
//        return 0;
//    }
//    printf("0x%s\n", msg_hex);

    const char *pos = msg_hex;

    // dummy packet creation
    uint8_t buf[MAX_PACKET];
    memset(buf, 0, sizeof(buf));
    for(size_t count = 0; count < strlen(msg_hex)/2; count++) {
        sscanf(pos, "%2hhx", (buf + count));
        pos += 2;
    }


#if FUNCTION_TEST

    int count=0;
    struct timespec tstart={0,0}, tend={0,0};

    clock_gettime(CLOCK_MONOTONIC, &tstart);


    for(count =0; count < LOOP_COUNT; ++count) {

        _net_element *net_node = (_net_element*)malloc(sizeof(_net_element));
        net_node->info = (dns_info *)malloc(sizeof(dns_info));
        memcpy(net_node->info->packet.buf, buf, sizeof(buf));
        net_node->rcv_buf_len = 61;
        net_node->peer.sin_family = AF_INET;
        net_node->peer.sin_port = htons(8888);
        net_node->peer.sin_addr.s_addr = htonl(INADDR_ANY);
        net_node->rcv_func = handle_request;
        printf("size of msg_hex: %d\n", sizeof(buf));

        hndl_dns_pkt(net_node);
    }

    clock_gettime(CLOCK_MONOTONIC, &tend);

#elif THREAD_TEST

    pthread_attr_t attr;
    int count=0;
    pthread_t tid[LOOP_COUNT];
    struct timespec tstart={0,0}, tend={0,0};

    clock_gettime(CLOCK_MONOTONIC, &tstart);

    init_thread_attr(&attr, 2048);

    for(count =0; count < LOOP_COUNT; ++count) {

        _net_element *net_node = malloc(sizeof(_net_element));
        net_node->info = (dns_info *)malloc(sizeof(dns_info));
        memcpy(net_node->info->packet.buf, buf, sizeof(buf));
        net_node->rcv_buf_len = 61;
        net_node->peer.sin_family = AF_INET;
        net_node->peer.sin_port = htons(8888);
        net_node->peer.sin_addr.s_addr = htonl(INADDR_ANY);
        net_node->rcv_func = handle_request;
        printf("size of msg_hex: %d\n", sizeof(buf));

        pthread_create(&tid[count], NULL, &hndl_dns_pkt, net_node);

    }


    for (count = 0; count < LOOP_COUNT; ++count)
        pthread_join(tid[count], NULL);

    clock_gettime(CLOCK_MONOTONIC, &tend);

#endif

    printf("time taken  about %.5f seconds\n",
           ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
           ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));


    while(1);


    return 0;
}
