cmake_minimum_required(VERSION 3.26)

project(LineageCheating VERSION 0.0.1)

if(NOT MSVC)
    message(FATAL_ERROR "This project only be compile by MSVC compiler.")
endif()

if(MSVC)
    add_compile_options(/utf-8)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(3RDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
set(3RDPARTY_INCLUDE_DIR ${3RDPARTY_DIR}/include)
set(3RDPARTY_LIBS_DIR ${3RDPARTY_DIR}/lib)
set(3RDPARTY_BINS_DIR ${3RDPARTY_DIR}/bin)

set(OpenCV_DIR $ENV{MSVC_LIBS}/x64/vc16/lib)

set(NDI_SDK_DIR "D:/00_Dev/NDI 6 SDK")
set(NDI_SDK_INCLUDE_DIR ${NDI_SDK_DIR}/Include)
set(NDI_SDK_LIBS_DIR ${NDI_SDK_DIR}/Lib/x64)

file(GLOB RUNTIME_DEPENDENCIES CONFIGURE_DEPENDS ${3RDPARTY_BINS_DIR}/*.dll)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/backend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/frontend)

set(TEST ON)
if(${TEST})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/main)
endif()
