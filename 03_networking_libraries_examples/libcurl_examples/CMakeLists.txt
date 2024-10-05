cmake_minimum_required(VERSION 3.11...3.20)
project(CurlExamples)

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
    )
endfunction()

compile_executables(basic_curl ${CMAKE_CURRENT_SOURCE_DIR}/src/basic_curl.cpp)
compile_executables(curl_multithreaded ${CMAKE_CURRENT_SOURCE_DIR}/src/curl_multithreaded.cpp)
compile_executables(curl_multi_handle ${CMAKE_CURRENT_SOURCE_DIR}/src/curl_multi_handle.cpp)
