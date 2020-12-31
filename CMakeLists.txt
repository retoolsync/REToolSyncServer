# Required for FetchContent
cmake_minimum_required(VERSION 3.14.5)

# Create project
project(REToolSyncServer)

# Target at least C++17
set(CMAKE_CXX_STANDARD 17)

# Add dependencies
include(cmake/CPM.cmake)
include(cmake/CivetWeb.cmake)

# Create project target
add_executable(${PROJECT_NAME} src/REToolSyncServer.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE civetweb)

# Set the Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})