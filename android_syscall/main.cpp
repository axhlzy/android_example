#include <iostream>
#include <cstring>          // strlen
#include <stdarg.h>         // va_list, va_start, va_end
#include "syscall_wrapper.h"

static log_mode_t g_log_mode = (log_mode_t)USE_ANDROID_LOG_DEFAULT;

void set_log_mode(log_mode_t mode) {
    g_log_mode = mode;
    const char* mode_str = (mode == LOG_MODE_ANDROID) ? "Android logcat" : "printf";
    log_print(LOG_LEVEL_INFO, "Log mode switched to: %s", mode_str);
}

log_mode_t get_log_mode(void) {
    return g_log_mode;
}

void log_print(log_level_t level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    if (g_log_mode == LOG_MODE_ANDROID) {
        int android_level;
        switch (level) {
            case LOG_LEVEL_DEBUG: android_level = ANDROID_LOG_DEBUG; break;
            case LOG_LEVEL_INFO:  android_level = ANDROID_LOG_INFO; break;
            case LOG_LEVEL_WARN:  android_level = ANDROID_LOG_WARN; break;
            case LOG_LEVEL_ERROR: android_level = ANDROID_LOG_ERROR; break;
            default: android_level = ANDROID_LOG_INFO; break;
        }
        __android_log_vprint(android_level, LOG_TAG, format, args);
    } else {
        const char* level_str;
        FILE* output_stream;
        
        switch (level) {
            case LOG_LEVEL_DEBUG: level_str = "DEBUG"; output_stream = stdout; break;
            case LOG_LEVEL_INFO:  level_str = "INFO";  output_stream = stdout; break;
            case LOG_LEVEL_WARN:  level_str = "WARN";  output_stream = stdout; break;
            case LOG_LEVEL_ERROR: level_str = "ERROR"; output_stream = stderr; break;
            default: level_str = "INFO"; output_stream = stdout; break;
        }
        
        fprintf(output_stream, "[%s][%s] ", level_str, LOG_TAG);
        vfprintf(output_stream, format, args);
        fprintf(output_stream, "\n");
        fflush(output_stream);
    }
    
    va_end(args);
}

// ARM32 and ARM64 raw_syscall impl
#if defined(__aarch64__)
// ARM64 (AArch64)
__attribute__((naked)) long raw_syscall(long __number, ...) {
    __asm__ __volatile__(
            "mov    x8, x0\n"          // 系统调用号存入x8
            "mov    x0, x1\n"          // 第1个参数
            "mov    x1, x2\n"          // 第2个参数  
            "mov    x2, x3\n"          // 第3个参数
            "mov    x3, x4\n"          // 第4个参数
            "mov    x4, x5\n"          // 第5个参数
            "mov    x5, x6\n"          // 第6个参数
            "svc    #0\n"              // 系统调用
            "ret\n"                    // 返回
            :
            :
            : "memory"
    );
}
#elif defined(__arm__)
// ARM32 版本
__attribute__((naked)) long raw_syscall(long __number, ...) {
    __asm__ __volatile__(
            "push   {r4-r7, lr}\n"     // 保存寄存器
            "mov    r7, r0\n"          // 系统调用号存入r7
            "mov    r0, r1\n"          // 第1个参数
            "mov    r1, r2\n"          // 第2个参数
            "mov    r2, r3\n"          // 第3个参数
            "ldr    r3, [sp, #20]\n"   // 第4个参数从栈中获取
            "ldr    r4, [sp, #24]\n"   // 第5个参数从栈中获取
            "ldr    r5, [sp, #28]\n"   // 第6个参数从栈中获取
            "ldr    r6, [sp, #32]\n"   // 第7个参数从栈中获取
            "svc    #0\n"              // 系统调用
            "pop    {r4-r7, pc}\n"     // 恢复寄存器并返回
            :
            :
            : "memory"
    );
}
#else
#error "Unsupported architecture. Only ARM32 and ARM64 are supported."
#endif

int my_open(const char* pathname, int flags, mode_t mode) {
    long result = raw_syscall(__NR_openat, AT_FDCWD, pathname, flags, mode);
    if (result < 0) {
        LOGI("open failed: %s, error: %ld", pathname, -result);
        return -1;
    }
    LOGI("open success: %s, fd: %ld", pathname, result);
    return (int)result;
}

ssize_t my_read(int fd, void* buf, size_t count) {
    long result = raw_syscall(__NR_read, fd, buf, count);
    if (result < 0) {
        LOGI("read failed: fd=%d, error: %ld", fd, -result);
        return -1;
    }
    LOGI("read success: fd=%d, bytes: %ld", fd, result);
    return (ssize_t)result;
}

ssize_t my_write(int fd, const void* buf, size_t count) {
    long result = raw_syscall(__NR_write, fd, buf, count);
    if (result < 0) {
        LOGI("write failed: fd=%d, error: %ld", fd, -result);
        return -1;
    }
    LOGI("write success: fd=%d, bytes: %ld", fd, result);
    return (ssize_t)result;
}

int my_close(int fd) {
    long result = raw_syscall(__NR_close, fd);
    if (result < 0) {
        LOGI("close failed: fd=%d, error: %ld", fd, -result);
        return -1;
    }
    LOGI("close success: fd=%d", fd);
    return (int)result;
}

off_t my_lseek(int fd, off_t offset, int whence) {
    long result = raw_syscall(__NR_lseek, fd, offset, whence);
    if (result < 0) {
        LOGI("lseek failed: fd=%d, error: %ld", fd, -result);
        return -1;
    }
    LOGI("lseek success: fd=%d, offset: %ld", fd, result);
    return (off_t)result;
}

int my_stat(const char* pathname, struct stat* statbuf) {
    long result = raw_syscall(__NR_newfstatat, AT_FDCWD, pathname, statbuf, 0);
    if (result < 0) {
        LOGI("stat failed: %s, error: %ld", pathname, -result);
        return -1;
    }
    LOGI("stat success: %s", pathname);
    return (int)result;
}

int my_mkdir(const char* pathname, mode_t mode) {
    long result = raw_syscall(__NR_mkdirat, AT_FDCWD, pathname, mode);
    if (result < 0) {
        LOGI("mkdir failed: %s, error: %ld", pathname, -result);
        return -1;
    }
    LOGI("mkdir success: %s", pathname);
    return (int)result;
}

int my_unlink(const char* pathname) {
    long result = raw_syscall(__NR_unlinkat, AT_FDCWD, pathname, 0);
    if (result < 0) {
        LOGI("unlink failed: %s, error: %ld", pathname, -result);
        return -1;
    }
    LOGI("unlink success: %s", pathname);
    return (int)result;
}

int my_copy_file(const char* src, const char* dst) {
    char buffer[4096];
    int src_fd = my_open(src, O_RDONLY);
    if (src_fd < 0) {
        LOGE("Failed to open source file: %s", src);
        return -1;
    }
    
    int dst_fd = my_open(dst, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (dst_fd < 0) {
        LOGE("Failed to open destination file: %s", dst);
        my_close(src_fd);
        return -1;
    }
    
    ssize_t bytes_read;
    while ((bytes_read = my_read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (my_write(dst_fd, buffer, bytes_read) != bytes_read) {
            LOGE("Failed to write to destination file: %s", dst);
            my_close(src_fd);
            my_close(dst_fd);
            return -1;
        }
    }
    
    my_close(src_fd);
    my_close(dst_fd);
    
    if (bytes_read < 0) {
        LOGE("Failed to read from source file: %s", src);
        return -1;
    }
    
    LOGI("File copied successfully: %s -> %s", src, dst);
    return 0;
}

int my_file_exists(const char* pathname) {
    struct stat statbuf;
    return my_stat(pathname, &statbuf) == 0;
}

size_t my_file_size(const char* pathname) {
    struct stat statbuf;
    if (my_stat(pathname, &statbuf) == 0) {
        return statbuf.st_size;
    }
    return 0;
}

void test_file_operations() {
    const char* test_file = "/data/local/tmp/test_syscall.txt";
    const char* test_content = "Hello, raw syscall world!\n";
    char read_buffer[256] = {0};
    
    LOGI("Starting file operations test...");

    int fd = my_open(test_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd >= 0) {
        my_write(fd, test_content, strlen(test_content));
        my_close(fd);
    }

    fd = my_open(test_file, O_RDONLY);
    if (fd >= 0) {
        ssize_t bytes_read = my_read(fd, read_buffer, sizeof(read_buffer) - 1);
        if (bytes_read > 0) {
            read_buffer[bytes_read] = '\0';
            LOGI("Read content: %s", read_buffer);
        }
        my_close(fd);
    }

    struct stat file_stat;
    if (my_stat(test_file, &file_stat) == 0) {
        LOGI("File size: %ld bytes", file_stat.st_size);
        LOGI("File mode: %o", file_stat.st_mode & 0777);
    }

    my_unlink(test_file);
    
    LOGI("File operations test completed.");
}

void test_advanced_operations() {
    const char* test_dir = "/data/local/tmp/syscall_test_dir";
    const char* src_file = "/data/local/tmp/syscall_src.txt";
    const char* dst_file = "/data/local/tmp/syscall_dst.txt";
    const char* test_content = "This is a test file for advanced operations.\nLine 2\nLine 3\n";
    
    LOGI("Starting advanced operations test...");

    if (my_mkdir(test_dir, 0755) == 0) {
        LOGI("Directory created successfully: %s", test_dir);
    }

    int fd = my_open(src_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd >= 0) {
        my_write(fd, test_content, strlen(test_content));
        my_close(fd);
        LOGI("Source file created: %s", src_file);
    }

    if (my_file_exists(src_file)) {
        LOGI("File exists: %s", src_file);
        size_t size = my_file_size(src_file);
        LOGI("File size: %zu bytes", size);
    }

    if (my_copy_file(src_file, dst_file) == 0) {
        LOGI("File copied successfully");

        if (my_file_exists(dst_file)) {
            size_t dst_size = my_file_size(dst_file);
            LOGI("Destination file size: %zu bytes", dst_size);
        }
    }

    fd = my_open(dst_file, O_RDONLY);
    if (fd >= 0) {
        char buffer[64] = {0};

        my_read(fd, buffer, 10);
        buffer[10] = '\0';
        LOGI("First 10 bytes: %s", buffer);

        my_lseek(fd, 0, SEEK_SET);

        memset(buffer, 0, sizeof(buffer));
        my_read(fd, buffer, 20);
        buffer[20] = '\0';
        LOGI("First 20 bytes after seek: %s", buffer);
        
        my_close(fd);
    }

    my_unlink(src_file);
    my_unlink(dst_file);
    
    LOGI("Advanced operations test completed.");
}

void print_architecture_info() {
#if defined(__aarch64__)
    LOGI("Architecture: ARM64 (AArch64)");
    LOGI("Pointer size: %zu bytes", sizeof(void*));
    LOGI("Long size: %zu bytes", sizeof(long));
#elif defined(__arm__)
    LOGI("Architecture: ARM32");
    LOGI("Pointer size: %zu bytes", sizeof(void*));
    LOGI("Long size: %zu bytes", sizeof(long));
#else
    LOGI("Architecture: Unknown");
#endif
}

int main() {
    LOGI("Android Raw Syscall Demo Started");

    set_log_mode(LOG_MODE_PRINTF);

    print_architecture_info();

    test_file_operations();

    test_advanced_operations();
    
    LOGI("Android Raw Syscall Demo Finished");
    return 0;
}