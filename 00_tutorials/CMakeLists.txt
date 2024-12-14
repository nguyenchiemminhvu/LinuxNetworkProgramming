cmake_minimum_required(VERSION 3.10...3.28)
project(SocketTutorials)

set(CMAKE_BUILD_TYPE Debug)

function(compile_executables target_name file_path)
    add_executable(${target_name} ${file_path})

    target_link_libraries(${target_name}
        PRIVATE
            pthread
    )
endfunction()

file(GLOB_RECURSE C_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c")

foreach(file_path ${C_FILES})
    get_filename_component(target_name ${file_path} NAME_WE)
    compile_executables(${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/${file_path})
endforeach()
