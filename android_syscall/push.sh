#!/bin/bash
#conan install . --profile android_arm64 --build=missing -s build_type=Release
#cmake -DCMAKE_BUILD_TYPE=Release --preset conan-release -S . -B build/Release
cmake --build build/Release
adb push /home/kali/CLionProjects/android_syscall/build/Release/android_syscall /data/local/tmp
adb shell su -c "chmod +x /data/local/tmp/android_syscall"
adb shell su -c ./data/local/tmp/android_syscall