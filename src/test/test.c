
#include "test.h"

#if FLAG_UNITTEST
    #include "test/unittest/unit_test.h"
#endif

#if FLAG_TEST_FILE_CREATION
        #include "utility/create_iso_file.h"
#endif

int test() {

    #if FLAG_UNITTEST
        unit_test();
    #endif

    #if FLAG_TEST_PARSER
        test_parse();
    #endif  // FLAG_TEST_PARSER

    #if FLAG_TEST_PACKET_PARSER
        test_packet_parse();
    #endif  // FLAG_TEST_PACKET_PARSER

    #if FLAG_TEST_LOG
        test_log();
    #endif  // FLAG_TEST_LOG

    #if FLAG_TEST_PARSER_TP
        test_parse_threadpool();
    #endif  // FLAG_TEST_PARSER_TP

    #if FLAG_TEST_LOOKUP
        test_lookup();
    #endif  // test_lookup

    #if FLAG_TEST_GEOIP
        test_lookup_geoip();
    #endif  // test_geoip

    #if FLAG_TEST_FILE_CREATION
        create_iso_file();
    #endif
    #if FLAG_TEST_AVL
        test_avl();
    #endif
}

