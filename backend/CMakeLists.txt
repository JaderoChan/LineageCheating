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

find_package(OpenCV REQUIRED)
find_library(
    NDI_LIB NAMES Processing.NDI.Lib.x64
    PATHS ${NDI_SDK_LIBS_DIR}
)
find_library(
    MSDK_LIB NAMES msdk
    PATHS ${3RDPARTY_LIBS_DIR}
)

# 配置头文件路径
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${3RDPARTY_DIR}/include
    ${NDI_SDK_INCLUDE_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    ${OpenCV_LIBS}
    ${NDI_LIB}
    ${MSDK_LIB}
)
