cmake_minimum_required(VERSION 3.5)
#cmake_minimum_required(VERSION 2.8.11)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeProjectVariables.cmake)

#message(STATUS "${BoldGreen}Started cmake configuring ...${ColourReset}")




                                        #################################
                                        #   CMake: cmake build system   #
                                        #################################
project(${PROJECT_NAME})

# The version number.
set(${PROJECT_NAME}_VERSION_MAJOR ${VERSION_MAJOR})
set(${PROJECT_NAME}_VERSION_MINOR ${VERSION_MINOR})

if (CMAKE_BUILD_TYPE STREQUAL "Release")        # "cmake -DCMAKE_BUILD_TYPE=Release .."
    message(STATUS "${BoldGreen}CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}${ColourReset}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -v -Wall -std=gnu11 -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -v -std=gnu++11")
else ()
    message(STATUS "${BoldGreen}CMAKE_BUILD_TYPE: Debug${ColourReset}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -v -DDEBUG -Wall -std=gnu11 -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -v -DDEBUG -Wall -std=gnu++11")
endif ()

# Custom cmake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")



# Include source files
set(SOURCE_FILES
#        edit_iso_file/create_iso_group.c

        src/dns/dns_app.c
        src/dns/dns_app.h
        src/dns/dns_packet.h
        src/dns/dns_packet_parser.c
        src/dns/dns_packet_parser.h
        src/dns/dns_response.c
        src/dns/dns_response.h
        src/dns/rtypes.c
        src/dns/rtypes.h
        src/dns/strutils.c
        src/dns/strutils.h
        src/dns/err_checker.c
        src/dns/err_checker.h
        src/dns/dns_thread_ctx.c
        src/dns/dns_thread_ctx.h

        src/lookup/loookup.c
        src/lookup/loookup.h
        src/lookup/geoip.c
        src/lookup/geoip.h
#        src/lookup/iso_groups_linear.c
        src/lookup/iso_groups_avl.c
        src/lookup/iso_groups.h

        src/server/server.h
        src/server/server.c
        src/server/server_isymped.c
#        src/server/server_threadpool.c
#        src/server/thread_pool.c
#        src/server/thread_pool.h
        src/server/cmd_server.c
        src/server/cmd_server.h

        src/factory/factory.h
        src/factory/factory_server.c
        src/factory/config_key.h

        src/utility/def.h
        src/utility/__assert.h
        src/utility/sqlite_db_backup.c
        src/utility/sqlite_db_backup.h
        src/utility/new.c
        src/utility/new.h
        src/utility/dns_log.c
        src/utility/dns_log.h
        src/utility/create_iso_group.c
        src/utility/create_iso_file.h
        src/utility/avl.c
        src/utility/avl.h
        src/utility/ini_parser.c
        src/utility/ini_parser.h

        src/utility/log/log.h
        src/utility/log/log.c
        src/utility/log/log_syslog.c

        src/test/test.h
        src/test/test.c
        src/test/test_parser.c
        src/test/test_packet_parse.c
#        src/test/test_parser_threadpool.c
        src/test/test_lookup.c
        src/test/test_lookup_geoip.c
        src/test/test_ipv_assert.c
        src/test/test_ipv_log.c
        src/test/unittest/unit_test.h
        src/test/unittest/unit_test.c

        src/config.h
        src/main.c
        src/geodns_error_code.h
        )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories(src)        # should be only 'include' directories.
include_directories(libs/p99)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

find_package(SQLite3 REQUIRED)
#include_directories(${SQLite3_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${SQLite3_LIBRARIES})

find_package(LibMaxmindDB REQUIRED)
#include_directories(${LibMaxmindDB_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${LibMaxmindDB_LIBRARIES})

target_link_libraries(${PROJECT_NAME} sqlite3 maxminddb)
########################################################################################################################




                                        #################################
                                        # CPack: rpm build related data #
                                        #################################
include(InstallRequiredSystemLibraries)

message(STATUS "${BoldGreen}XCPACK_PACKAGING_INSTALL_PATH: ${XCPACK_PACKAGING_INSTALL_PATH}${ColourReset}")

# Generate congfig.h file
#configure_file("${CMAKE_CURRENT_SOURCE_DIR}/templates/config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h" @ONLY IMMEDIATE)

set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")

configure_file("templates/systemd.service.in" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.service" @ONLY IMMEDIATE)
configure_file("templates/rpm.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/rpm.spec" @ONLY IMMEDIATE)
set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/rpm.spec")

install(TARGETS ${PROJECT_NAME} DESTINATION ${XCPACK_PACKAGING_INSTALL_PATH})

set(DATA_FILES
        DB_File/country_iso.txt
        DB_File/GeoLite2-Country.mmdb
        DB_File/lookup.db

        DB_File/iso.txt
        DB_File/iso_input_file.txt

        DB_File/geodns.ini
        )
install(FILES ${DATA_FILES} DESTINATION ${XCPACK_PACKAGING_DATA_PATH})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.service DESTINATION ${XCPACK_PACKAGING_SERVICE_PATH})
include(CPack)
########################################################################################################################




#message(STATUS "${BoldGreen}Finished cmake tasks.${ColourReset}")
