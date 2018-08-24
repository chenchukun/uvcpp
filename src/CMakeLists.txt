aux_source_directory(. LIB_SRC_LIST)

if(BUILD_SHARED_LIBS STREQUAL "ON")
    add_library(uvcpp_shared SHARED ${LIB_SRC_LIST})
    target_link_libraries(uvcpp_shared pthread tcmalloc)
    set_target_properties(uvcpp_shared PROPERTIES OUTPUT_NAME "uvcpp")
    set_target_properties(uvcpp_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    set_target_properties(uvcpp_shared PROPERTIES VERSION 0.1 SOVERSION 1)
endif()

if(BUILD_STATIC_LIBS STREQUAL "ON")
    add_library(uvcpp_static STATIC ${LIB_SRC_LIST})
    target_link_libraries(uvcpp_static pthread tcmalloc)
    set_target_properties(uvcpp_static PROPERTIES OUTPUT_NAME "uvcpp")
    set_target_properties(uvcpp_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
endif()

add_subdirectory(test)
