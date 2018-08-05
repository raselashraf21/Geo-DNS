
#ifndef GEODNS_TEST_H
#define GEODNS_TEST_H

#include "config.h"

#if FLAG_ENABLE_TEST

    #define FLAG_TEST_PARSER    0
    #define FLAG_TEST_PACKET_PARSER    0
    #define FLAG_TEST_LOG       0
    // TODO Give ERROR
    #define FLAG_TEST_PARSER_TP 0
    // TODO GIve Error
    #define FLAG_TEST_LOOKUP    0
    // TODO Check this test
    #define FLAG_TEST_GEOIP     0
    // TODO Check this test
    #define FLAG_UNITTEST       1
    #define FLAG_TEST_FILE_CREATION 0

#endif

extern int test_parse();
extern int test_packet_parse();
extern int test_parse_threadpool();
extern int test_lookup();
extern int test_lookup_geoip();
extern int test_log();
extern int create_iso_file();
extern int test_avl();

int test();

#endif // GEODNS_TEST_H
