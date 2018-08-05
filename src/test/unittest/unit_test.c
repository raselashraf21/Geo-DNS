

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <utility/new.h>

#include "unit_test.h"
#include "config.h"
#include "dns/dns_packet.h"
#include "dns/dns_packet_parser.h"
#include "server/server.h"
#include "factory/factory.h"

static server_app *_server_app;
thread_ctx *_tctx;

static void _initialize_unittest() {

    // server_app creation
    make_app((void **) &_server_app, NULL);

    // server_app initialization
    _server_app->initialize(_server_app->srv_ctx, _server_app->app_ctx);

    // server_app's thread_ctx initialization
    _tctx = calloc(1, sizeof(thread_ctx));
    _server_app->initialize_thd_ctx(_server_app->srv_ctx, _server_app->app_ctx, _tctx);

}

static void _deinitialize_unittest() {

    // server_app de-initialization
    _server_app->destroy(_server_app->app_ctx);

    // server_app's thread_ctx initialization
    _server_app->destroy_thd_ctx(_tctx);
    free(_tctx);

    // free-up server_app
    free_app((void **)&_server_app);
}

static net_element * _parse_packet(char *msg_hex)
{
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

    // dummy net_node creation
    net_element *net_node = new(NetElement, _server_app);
    net_node->id = 0;
    net_node->th_status = 0;
    net_node->sock_fd = -1;
    net_node->rcv_type = 0;
    net_node->peer->sin_family = AF_INET;
    net_node->peer->sin_port = htons(8888);
    net_node->peer->sin_addr.s_addr = htonl(INADDR_ANY);
    net_node->peer_len = sizeof(net_node->peer);
    net_node->srv_ctx = _server_app->srv_ctx;
    net_node->app_ctx = _server_app->app_ctx;
    char *info = _server_app->get_buffer(MAX_PACKET);
    memcpy(info, buf, sizeof(buf));
    net_node->rcv_buf = info;
    net_node->rcv_buf_len = strlen(msg_hex)/2;
    net_node->handle_buf = _server_app->handle_request;

    // request process
    net_node->handle_buf(net_node);

    return net_node;
}

int unit_test() {

    _initialize_unittest();

    const char message[MAX_PACKET];
    net_element *net_node;
    dns_info *info;

//#if NULL_BUFFER
//    _parse_packet(NULL);
//#endif

#if ALL_ZERO
    memset((char *)message, 0, MAX_PACKET);
    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(info->packet.header.qdcount <= 0)
        printf("ALL_ZERO: FAILED \n");
    else
        printf("ALL_ZERO: PASSED \n");

#endif

#if T_QR
    sprintf((char *)message, "%s","247480000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(info->packet.header.qr == 0 )
        _DEBUG_CHANGE printf("QR : FAILED\n");
    else
        _DEBUG_CHANGE printf("QR : PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif


#if T_OPCODE
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(info->packet.header.opcode != 0 )
           _DEBUG_CHANGE printf("OPCODE: FAILED\n");
    else
           _DEBUG_CHANGE printf("OPCODE: PASSEDD\n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_TC
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(info->packet.header.tc != 0 )
        _DEBUG_CHANGE printf("TC: FAILED \n");
    else
        _DEBUG_CHANGE printf("TC: PASSED \n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_RD
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(info->packet.header.rd != 0 )
        _DEBUG_CHANGE printf("RD: FAILED \n");
    else
        _DEBUG_CHANGE printf("RD: PASSED\n");

    free_dns_packet(&info);
    delete(net_node);
#endif

#if T_QD
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(info->packet.header.qdcount <= 0 )
        _DEBUG_CHANGE printf("QD_COUNT: FAILED \n");
    else
        _DEBUG_CHANGE printf("QD_COUNT: PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_AN
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(info->packet.header.ancount <= 0 )
        _DEBUG_CHANGE printf("AN_COUNT: FAILED \n");
    else
        _DEBUG_CHANGE printf("AN_COUNT: PASSED \n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_AR
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;

    if(info->packet.header.arcount > 0 )
       _DEBUG_CHANGE printf("AR_COUNT: FAILED\n");
    else
       _DEBUG_CHANGE printf("AR_COUNT: PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif


#if T_QTYPE
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;

    if(info->questions->qtype !=1 && info->questions->qtype !=28 )
        _DEBUG_CHANGE printf("QTYPE: FAILED \n");
    else
        _DEBUG_CHANGE printf("QTYPE: PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_QCLASS
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

   net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;

    if(info->questions->qclass !=1)
         _DEBUG_CHANGE printf("QCLASS: FAILED \n");
    else
         _DEBUG_CHANGE printf("QCLASS: PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_ANTYPE
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(!info->answers || info->answers->type != 1)
        _DEBUG_CHANGE printf("AN_TYPE: FAILED \n");
    else
        _DEBUG_CHANGE printf("AN_TYPE PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_ANCLASS
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;

    if(!info->answers || info->answers->cls != 1)
        _DEBUG_CHANGE printf("AN_CLASS: FAILED \n");
    else
        _DEBUG_CHANGE printf("AN_CLASS: PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_RD_LENGTH
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(!info->answers || info->answers->rdlength <= 0)
         _DEBUG_CHANGE printf("RD_LENGTH: FAILED \n");
    else
         _DEBUG_CHANGE  printf("RD_LENGTH: PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif

#if T_RD_DATA
    sprintf((char *)message, "%s","247400000001000000000001016308697076636c6f75640672696e"
            "67696403636f6d0000010001000029100000008000000b00080007"
            "00011800671a70");

    net_node = _parse_packet((char *)message);
    info = (dns_info *)net_node->rcv_buf;
    if(!info->answers || info->answers->rdata == NULL || strlen(info->answers->rdata) < 4)
            _DEBUG_CHANGE printf("RD_DATA: FAILED \n");
    else
            _DEBUG_CHANGE printf("RD_DATA: PASSED\n");

    free_dns_packet(&info);
    delete(net_node);

#endif

    _deinitialize_unittest();

    return 0;
}