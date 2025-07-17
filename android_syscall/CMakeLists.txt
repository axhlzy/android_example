cmake_minimum_required(VERSION 3.31)
project(android_syscall)

set(CMAKE_CXX_STANDARD 17)

# Log mode option
option(USE_PRINTF_LOG "Use printf for logging instead of Android logcat" OFF)

# Add header file directory
include_directories(.)

# Add executable
add_executable(android_syscall main.cpp)

# Configure log mode
if(USE_PRINTF_LOG)
    target_compile_definitions(android_syscall PRIVATE USE_ANDROID_LOG_DEFAULT=0)
    message(STATUS "Log mode: printf")
else()
    target_compile_definitions(android_syscall PRIVATE USE_ANDROID_LOG_DEFAULT=1)
    message(STATUS "Log mode: Android logcat")
endif()

# Architecture-specific settings
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message(STATUS "Building for ARM64 (AArch64)")
    target_compile_definitions(android_syscall PRIVATE __aarch64__)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    message(STATUS "Building for ARM32")
    target_compile_definitions(android_syscall PRIVATE __arm__)
else()
    message(WARNING "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Link log library for Android
if(ANDROID)
    target_link_libraries(android_syscall log)
endif()

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(android_syscall PRIVATE -Wall -Wextra)
endif()