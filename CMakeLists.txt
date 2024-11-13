cmake_minimum_required(VERSION 3.18)
include(FetchContent)

# Host CPU
if(NOT CMAKE_HOST_SYSTEM_PROCESSOR)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CMAKE_HOST_SYSTEM_PROCESSOR "${ARCH}")
endif()
if(NOT CMAKE_HOST_SYSTEM_PROCESSOR)
    message(WARNING "host system processor not detected.")
    set(CMAKE_HOST_SYSTEM_PROCESSOR "x86_64")
endif()
set(CMAKE_HOST_SYSTEM_NAME_LOWER )

# Zig
set(ZIG_VERSION "0.13.0")
set(ZIG_DOWNLOAD "http://ziglang.org/download")
set(ZIG_PATH "${CMAKE_SOURCE_DIR}/vendor/zig/zig")
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
    set(ZIG_DOWNLOAD "${ZIG_DOWNLOAD}/${ZIG_VERSION}/zig-macos-${CMAKE_HOST_SYSTEM_PROCESSOR}-${ZIG_VERSION}.tar.xz")
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    set(ZIG_DOWNLOAD "${ZIG_DOWNLOAD}/${ZIG_VERSION}/zig-linux-${CMAKE_HOST_SYSTEM_PROCESSOR}-${ZIG_VERSION}.tar.xz")
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set(ZIG_DOWNLOAD "${ZIG_DOWNLOAD}/${ZIG_VERSION}/zig-windows-${CMAKE_HOST_SYSTEM_PROCESSOR}-${ZIG_VERSION}.zip")
endif()
if(NOT EXISTS ${ZIG_PATH})
    FetchContent_Populate(
        compiler_zig
        URL "${ZIG_DOWNLOAD}"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendor/zig"
        SUBBUILD_DIR "${CMAKE_BINARY_DIR}/CMakeFiles/_deps/zig-subbuild"
        BINARY_DIR "${CMAKE_BINARY_DIR}/CMakeFiles/_deps/zig-build"
    )
endif()
find_program(ZIGC zig PATHS "${CMAKE_SOURCE_DIR}/vendor/zig" REQUIRED NO_DEFAULT_PATH)

# Compiler
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain.cmake")

# Project
project(player C)
file(GLOB PROJECT_FILES "${CMAKE_SOURCE_DIR}/src/*.c")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/vendor")
include_directories("${CMAKE_SOURCE_DIR}/vendor/ffmpeg")

# Color
set(COLOR_DOWNLOAD "https://github.com/rodrigodornelles/rgb-to-xterm256/archive/refs/heads/main.zip")
if(NOT EXISTS "${LUA_PATH}/color")
    FetchContent_Populate(
        lib_color
        URL "${COLOR_DOWNLOAD}"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendor/color"
        SUBBUILD_DIR "${CMAKE_BINARY_DIR}/CMakeFiles/_deps/rgb_2_xterm256-subbuild"
        BINARY_DIR "${CMAKE_BINARY_DIR}/CMakeFiles/_deps/rgb_2_xterm256-build"
    )
endif()

# FFMPEG
set(FFMPEG_VERSION "7.0.2")
set(FFMPEG_DOWNLOAD "https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n${FFMPEG_VERSION}.tar.gz")
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/vendor/ffmpeg")
if (NOT EXISTS "${FFMPEG_DIR}/libavcodec/libavcodec.a")
    FetchContent_Populate(dep_ffmpeg URL ${FFMPEG_DOWNLOAD} SOURCE_DIR ${FFMPEG_DIR})
    execute_process(
        COMMAND ./configure --prefix=${FFMPEG_DIR}/build --enable-static --disable-shared 
            --disable-zlib --disable-lzma --disable-x86asm --disable-libdrm
        WORKING_DIRECTORY ${FFMPEG_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND make
        WORKING_DIRECTORY ${FFMPEG_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()
FILE(GLOB ffmpeg_files "${FFMPEG_DIR}/*/*.a")

add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE "${ffmpeg_files}")
