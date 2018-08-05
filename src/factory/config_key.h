
#ifndef GEODNS_CONFIG_KEY_H
#define GEODNS_CONFIG_KEY_H

// [lookup]
#define KEY_SECTION_LOOKUP      "lookup"
#define KEY_PATH_DB_SQLITE3     "db_sqlite3_path"
#define KEY_PATH_COUNTRY_ISO    "country_iso_path"
#define KEY_ISO_COUNT           "iso_count"
#define KEY_GROUP_COUNT         "group_count"

// [geoip]
#define KEY_SECTION_GEOIP       "geoip"
#define KEY_PATH_DB_MAXMIND     "db_maxmind_path"

// [server-udp]
#define KEY_SECTION_SERVER_UDP  "server-udp"
#define KEY_SERVER_IP           "server_ip"
#define KEY_SERVER_PORT         "server_port"
#define KEY_SERVER_TYPE         "server_type"
#define KEY_THREAD_COUNT        "thread_count"

// [log]
#define KEY_SECTION_LOG         "log"
#define KEY_LOG_NAME            "log_name"
#define KEY_LOG_PRIORITY        "log_priority"
#define KEY_LOG_LEVEL           "log_level"

#endif // GEODNS_CONFIG_KEY_H
