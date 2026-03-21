cmake_minimum_required(VERSION 3.17)

project(LineageCheating VERSION 0.0.1)

if(NOT MSVC)
    message(FATAL_ERROR "This project only be compile by MSVC compiler.")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/backend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/main)
