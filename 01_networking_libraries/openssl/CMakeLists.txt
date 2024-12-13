cmake_minimum_required(VERSION 3.11...3.20)
project(OpenSSLExamples)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads)
find_package(CURL REQUIRED)

function(compile_executables target_name file_path)
    add_executable(${target_name} ${file_path})
    target_link_libraries(${target_name}
        PRIVATE
            CURL::libcurl
            Threads::Threads
            ssl
            crypto
    )
endfunction()

add_custom_target(copy_certificates ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/certificate
    ${CMAKE_BINARY_DIR}/
    COMMENT "Copying certificate files to build directory"
)

compile_executables(https_client ${CMAKE_CURRENT_SOURCE_DIR}/src/https_client.cpp)

compile_executables(ssl_client_server ${CMAKE_CURRENT_SOURCE_DIR}/src/ssl_client_server.cpp)
add_dependencies(ssl_client_server copy_certificates)