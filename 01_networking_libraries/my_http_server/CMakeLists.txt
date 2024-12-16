cmake_minimum_required(VERSION 3.11...3.20)
project(HTTPServer)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_DEBUG ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_DEBUG_LIBS OFF)
set(Boost_USE_STATIC_LIBS OFF)

find_package(Threads)
find_package(CURL REQUIRED)
find_package(Boost 1.65 REQUIRED
    COMPONENTS
        system
        filesystem
)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(HTTPServer main.cpp ${SOURCES})

target_include_directories(HTTPServer
    PRIVATE
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(HTTPServer
    PRIVATE
        CURL::libcurl
        Threads::Threads
        ssl
        crypto
        ${Boost_LIBRARIES}
)

add_custom_target(deploy_http_root ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/http_root
    ${CMAKE_BINARY_DIR}/http_root
    COMMENT "Deploying HTTP root folder to build directory"
)

add_dependencies(HTTPServer deploy_http_root)
