cmake_minimum_required(VERSION 3.11...3.20)
project(HTTPServer)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads)
find_package(CURL REQUIRED)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(HTTPServer main.cpp ${SOURCES})

target_link_libraries(HTTPServer
    PRIVATE
        Threads::Threads
)

add_custom_target(deploy_http_root ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/http_root
    ${CMAKE_BINARY_DIR}/http_root
    COMMENT "Deploying HTTP root folder to build directory"
)

add_dependencies(HTTPServer deploy_http_root)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    target_link_libraries(HTTPServer PRIVATE stdc++fs)
endif()