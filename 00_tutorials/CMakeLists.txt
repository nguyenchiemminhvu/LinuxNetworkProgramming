cmake_minimum_required(VERSION 3.10...3.28)
project(SocketTutorials)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(compile_executables target_name file_path)
    add_executable(${target_name} ${file_path})

    target_link_libraries(${target_name}
        PRIVATE
            pthread
    )
endfunction()

file(GLOB_RECURSE CPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

foreach(file_path ${CPP_FILES})
    get_filename_component(target_name ${file_path} NAME_WE)
    compile_executables(${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/${file_path})
endforeach()
