cmake_minimum_required(VERSION 3.6)
project(REToolSyncServer)

set(CMAKE_CXX_STANDARD 14)

add_executable(REToolSyncServer main.cpp cpp-httplib/httplib.h json.hpp)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT REToolSyncServer)

if (WIN32)
    target_link_libraries(REToolSyncServer PRIVATE ws2_32)
endif ()