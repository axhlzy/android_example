#ifndef SYSCALL_WRAPPER_H
#define SYSCALL_WRAPPER_H

#include <fcntl.h>
#include <sys/stat.h>

#define LOG_TAG "syscall-wrapper"

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} log_level_t;

typedef enum {
    LOG_MODE_PRINTF = 0,
    LOG_MODE_ANDROID = 1
} log_mode_t;

void set_log_mode(log_mode_t mode);
log_mode_t get_log_mode(void);
void log_print(log_level_t level, const char* format, ...);

#define LOGD(...) log_print(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOGI(...) log_print(LOG_LEVEL_INFO, __VA_ARGS__)
#define LOGW(...) log_print(LOG_LEVEL_WARN, __VA_ARGS__)
#define LOGE(...) log_print(LOG_LEVEL_ERROR, __VA_ARGS__)

extern "C" long raw_syscall(long __number, ...);

int my_open(const char* pathname, int flags, mode_t mode = 0);
ssize_t my_read(int fd, void* buf, size_t count);
ssize_t my_write(int fd, const void* buf, size_t count);
int my_close(int fd);
off_t my_lseek(int fd, off_t offset, int whence);
int my_stat(const char* pathname, struct stat* statbuf);
int my_mkdir(const char* pathname, mode_t mode);
int my_unlink(const char* pathname);

int my_copy_file(const char* src, const char* dst);
int my_file_exists(const char* pathname);
size_t my_file_size(const char* pathname);


void print_architecture_info();

#endif // SYSCALL_WRAPPER_H