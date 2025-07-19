# Android Syscall Demo

This is a system call wrapper library for the Android platform, implementing file operations by directly using Linux system calls. This project is primarily for educational and research purposes, demonstrating how to use system calls directly on the Android platform.

## Features

- Supports ARM32 and ARM64 architectures
- Provides basic file operations:
    - File open/close
    - File read/write
    - File seeking
    - Get file status
    - Create directory
    - Delete file
- Provides advanced file operations:
    - File copying
    - Check for file existence
    - Get file size
- Supports two logging modes: Android logcat and standard printf

## Architecture Support

This project is specifically optimized for the ARM architecture, with support for:
- ARM32 (armeabi-v7a)
- ARM64 (arm64-v8a)

## Usage Example

The project includes a complete set of test cases demonstrating how to use these system calls:

1.  **Basic File Operations Test:**
    - Create a file
    - Write content to the file
    - Read content from the file
    - Get file information
    - Delete the file

2.  **Advanced Features Test:**
    - Create a directory
    - Copy a file
    - File seeking (lseek)
    - Verification of multiple file operations

## Logging System

The project has a built-in flexible logging system that supports:
- Different log levels (DEBUG, INFO, WARN, ERROR)
- Switchable log output modes (Android logcat or standard printf)

## Compilation and Running

To compile and run this project, ensure you have a C++ development environment for Android. The project uses CMake for building.

## Notes

This project is intended for educational and research purposes to demonstrate the usage of low-level system calls. In actual development, it is recommended to prioritize using the high-level APIs provided by the Android SDK.