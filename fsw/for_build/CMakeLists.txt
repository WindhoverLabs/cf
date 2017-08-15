
add_airliner_app_def(cf
    FILE CF
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_callbacks.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_cmds.c  
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_playback.c  
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_utils.c  
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/aaa.c  
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/callbacks.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/cfdp_lib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/default_filestore.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/default_indication.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/event.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/machine_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/message_class.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/mib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/misc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/nak.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/nak_mem.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/pdu_as_string.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/pdu.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/r1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/r2.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/s1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/s2.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/timer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/utils.c
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PUB/
    USER_DOCS   ${CMAKE_CURRENT_SOURCE_DIR}/user_doxy.in
)


add_airliner_app_unit_test(cf-ut
    UTASSERT 
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_callbacks.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_cmds.c  
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_playback.c  
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_utils.c  
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/aaa.c  
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/callbacks.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/cfdp_lib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/default_filestore.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/default_indication.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/event.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/machine_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/message_class.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/mib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/misc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/nak.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/nak_mem.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/pdu_as_string.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/pdu.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/r1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/r2.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/s1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/s2.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/timer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/cf_cfgtable.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/cf_testcase.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/cf_testrunner.c
        
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/../mission_inc/
        ${CMAKE_CURRENT_SOURCE_DIR}/../platform_inc/
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PUB/
        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/
)


