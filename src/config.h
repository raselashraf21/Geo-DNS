
#ifndef GEODNS_CONFIG_H
#define GEODNS_CONFIG_H

#include "utility/def.h"

// SOURCE : http://stackoverflow.com/a/7464257
//          (http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing)
// USAGE  : To print diagnostics to the error log
// WARNING: Don't use like this
//          1) D continue; or
//          2) D break;
#ifdef DEBUG				// enable through gcc -DDEBUG
	#define _DEBUG
    #define FLAG_ENABLE_TEST    0
#else
    #define FLAG_ENABLE_TEST    0
	#define _DEBUG for(;0;)
#endif

#define _DEBUG_CHANGE

#define MULTI_THREAD 1

#define RECURSION_ENABLE 0

#define MAX_PACKET      512     // udp payload
#define MAX_ANSWER      3
#define MAX_IP_ENTRY    10      // Maximum IP-entry in table_ip_info(db) against individual host-group.
#define MAX_HOST_GROUP  10      // Maximum group-entry in table_host_group(db) against individual host.

#define DEFAULT_PATH_DATA   "/home/ashraf/geodns/DB_File"                              // test-himu

//#define DEFAULT_PATH_DATA    "/etc/geodns"

#define DEFAULT_PATH_CONF_INI   DEFAULT_PATH_DATA"/geodns.ini"

#define DEFAULT_PATH_COUNTRY_ISO    DEFAULT_PATH_DATA"/country_iso.txt"
#define DEFAULT_PATH_READ_ISO       DEFAULT_PATH_DATA"/iso.txt"
#define DEFAULT_PATH_INPUT_ISO      DEFAULT_PATH_DATA"/iso_input_file.txt"

#define DNS_HDR_SIZE 12
#define IP_SIZE 20
#define MEM_RESET 0

/* for testing purpose */
#define THREAD_TEST 0
#define FUNCTION_TEST 1
#define LOOP_COUNT 2

/* for thread pool*/
#define THREAD_POOL_ON 1
#define THREAD_NUMBER 100
#define QUEUE_SIZE 500


// Server Configuration
#define SERVER_IP	"38.108.92.202"
#define SERVER_PORT	53

//command server

#define CMD_SERVER_ENABLE 1
#define CMD_SERVER_IP "127.0.0.1"
#define CMD_SERVER_PORT 50700
#define CMD_SERVER_TYPE "TcP"



#define EXIT_CODE_DNS   101             // exit from 'dns_initializer'
#define EXIT_CODE_ISO1  102             // exit from 'iso_groups_inilizer'
#define EXIT_CODE_ISO2  103             // exit from 'iso_groups_inilizer'
#define EXIT_CODE_INI   104             // exit from 'make_app' for wrong configuration(ini) file

// Log Level
#define L_VOID  0
#define L_INFO  2
#define L_DEBUG 4
#define L_CRIT  8
#define L_ERR   16
#define STD_ERR 128
#define L_ALL  255

#endif // GEODNS_CONFIG_H
