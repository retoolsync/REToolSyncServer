# CivetWeb v1.13
# Licensed under MIT (https://github.com/civetweb/civetweb/blob/v1.13/LICENSE.md)

CPMAddPackage(
    NAME civetweb
    GITHUB_REPOSITORY civetweb/civetweb
    VERSION 1.13
    DOWNLOAD_ONLY ON
)

if(civetweb_ADDED)
    add_library(civetweb STATIC
        ${civetweb_SOURCE_DIR}/src/civetweb.c
        ${civetweb_SOURCE_DIR}/src/CivetServer.cpp
        ${civetweb_SOURCE_DIR}/include/civetweb.h
        ${civetweb_SOURCE_DIR}/include/CivetServer.h
    )
    target_include_directories(civetweb PUBLIC ${civetweb_SOURCE_DIR}/include)
    
    # See https://github.com/civetweb/civetweb/blob/19f31ba8dd8443e86c7028a4b4c37f4b299aa68c/docs/api/mg_check_feature.md#description
    target_compile_definitions(civetweb PRIVATE NO_SSL USE_WEBSOCKET)
    if(WIN32)
	    target_link_libraries(civetweb PUBLIC ws2_32)
    endif()

    # We need threading
    find_package(Threads REQUIRED)
    target_link_libraries(civetweb PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()