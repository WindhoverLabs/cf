
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
)


#add_airliner_app_unit_test(CF-UT
#    UTF 
#    SOURCES
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_app.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_callbacks.c 
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_cmds.c  
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_playback.c  
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/cf_utils.c  
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/aaa.c  
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/callbacks.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/cfdp_lib.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/default_filestore.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/default_indication.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/event.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/machine_list.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/message_class.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/mib.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/misc.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/nak.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/nak_mem.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/pdu_as_string.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/pdu.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/r1.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/r2.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/s1.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/s2.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/timer.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/utils.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/cf_cfgtable.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/cf_testcase.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/cf_testrunner.c
#        
#    INCLUDES
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PRI/
#        ${CMAKE_CURRENT_SOURCE_DIR}/../src/PUB/
#        ${CMAKE_CURRENT_SOURCE_DIR}/../unit_test/
#)

## Generate documentation
#find_package(Doxygen)
#if(DOXYGEN_FOUND)
#    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/user_doxy.in ${CMAKE_CURRENT_BINARY_DIR}/user_doxy @ONLY)
#    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/detail_doxy.in ${CMAKE_CURRENT_BINARY_DIR}/detail_doxy @ONLY)
#    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/codewalk_doxy.in ${CMAKE_CURRENT_BINARY_DIR}/codewalk_doxy @ONLY)
#    add_custom_target(sch-docs
#        COMMENT "Generating API documentation with Doxygen" VERBATIM
#    	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/docs/users_guide/html/${APPTARGET}/
#    	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/docs/users_guide/latex/${APPTARGET}/
#    	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/docs/detailed_design/html/${APPTARGET}/
#    	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/docs/detailed_design/latex/${APPTARGET}/
#    	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/docs/cwt/html/${APPTARGET}/
#    	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/docs/cwt/latex/${APPTARGET}/
#        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/user_doxy
#        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/detail_doxy
#        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/codewalk_doxy
#        #COMMAND make -C ${CMAKE_BINARY_DIR}/docs/users_guide/latex/${APPTARGET}
#        #COMMAND make -C ${CMAKE_BINARY_DIR}/docs/detailed_design/latex/${APPTARGET}
#        #COMMAND make -C ${CMAKE_BINARY_DIR}/docs/cwt/latex/${APPTARGET}
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/
#    )
#endif(DOXYGEN_FOUND)
