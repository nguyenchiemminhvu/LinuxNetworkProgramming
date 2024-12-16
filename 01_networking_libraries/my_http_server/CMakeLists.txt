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
        CURL::libcurl
        Threads::Threads
        ssl
        crypto
)