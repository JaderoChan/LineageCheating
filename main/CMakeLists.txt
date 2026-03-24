project(LineageCheatingMain VERSION 0.0.1 LANGUAGES CXX)
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

# 重定向目标文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# 生成
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# 配置依赖
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    LineageCheatingBackend
)

# 复制依赖库至目标文件夹下
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${RUNTIME_DEPENDENCIES} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
