cmake_minimum_required(VERSION 3.26)

project(LineageCheating VERSION 0.0.1)

if(NOT MSVC)
    message(FATAL_ERROR "This project only be compile by MSVC compiler.")
endif()

if(MSVC)
    add_compile_options(/source-charset:utf-8 /execution-charset:utf-8)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/backend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/main)
