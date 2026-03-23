project(LineageCheatingBackend VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认使用 Release 构建
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 头文件（可用于在 Visual Studio IDE 的项目文件树中显示头文件）
file(
    GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)
# 源文件
file(
    GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

set(PROJECT_SOURCES ${HEADERS} ${SRCS})

# 生成
add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

# 配置依赖
list(PREPEND CMAKE_PREFIX_PATH $ENV{MSVC_LIBS})

include(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/onnx_runtime/OnnxRuntimeConfig.cmake)

set(3RDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
set(OpenCV_DIR $ENV{MSVC_LIBS}/x64/vc16/lib)
set(NDI_SDK_DIR "D:/00_Dev/NDI 6 SDK")
set(3RDLIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(OpenCV REQUIRED)
find_library(
    NDI_LIB NAMES Processing.NDI.Lib.x64
    PATHS ${NDI_SDK_DIR}/Lib/x64
)
find_library(
    MSDK_LIB NAMES msdk
    PATHS ${3RDLIBS_DIR}
)
find_library(
    RAPID_OCR_LIB NAMES RapidOcrOnnx
    PATHS ${3RDLIBS_DIR}
)

# 配置头文件路径
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/public
    ${NDI_SDK_DIR}/Include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${3RDPARTY_DIR}/json/include
    ${3RDPARTY_DIR}/rapid_ocr/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    ${OpenCV_LIBS}
    ${NDI_LIB}
    ${MSDK_LIB}
    PRIVATE
    ${OnnxRuntime_LIBS}
    ${RAPID_OCR_LIB}
)

# 定义必需动态库文件
file(GLOB RUNTIME_DEPENDENCIES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bin/*.dll)
set(RUNTIME_DEPENDENCIES ${RUNTIME_DEPENDENCIES} CACHE STRING "Shared libraries of backend dependencies.")

# 定义模型文件夹
set(MODELS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/models CACHE STRING "Models file directory.")
