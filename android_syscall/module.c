#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <common.h>
#include <kputils.h>
#include <linux/string.h>

KPM_NAME("kpm-hook-read-write");
KPM_VERSION("1.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("axhlzy");
KPM_DESCRIPTION("KernelPatch Module");

static uid_t target_uid = (uid_t)-1;

#include <asm/current.h>
#include <linux/sched.h>
#include <linux/cred.h>

uid_t current_uid()
{
    struct cred *cred = *(struct cred **)((uintptr_t)current + task_struct_offset.cred_offset);
    uid_t uid = *(uid_t *)((uintptr_t)cred + cred_offset.uid_offset);
    return uid;
}

#include "syscall.h"

unsigned long **sys_call_table;

#include <uapi/asm-generic/unistd.h>

static void before_read(void *fdata_args)
{
    if (target_uid != (uid_t)-1 && current_uid() != target_uid) {
        return;
    }

    long fd = (long)syscall_argn(fdata_args, 0);
    size_t count = (size_t)syscall_argn(fdata_args, 2);

    pr_info("kpm: [UID %u] read(fd=%ld, count=%zu)\n",
            current_uid(), fd, count);
}

static void after_read(void *fdata_args)
{
    if (target_uid != (uid_t)-1 && current_uid() != target_uid) {
        return;
    }

    long fd = (long)syscall_argn(fdata_args, 0);
    long ret = (long)syscall_argn(fdata_args, 0);

    pr_info("kpm: [UID %u] read(fd=%ld) ret %ld\n",
            current_uid(), fd, ret);
}

static void before_write(void *fdata_args)
{
    if (target_uid != (uid_t)-1 && current_uid() != target_uid) {
        return;
    }

    long fd = (long)syscall_argn(fdata_args, 0);
    size_t count = (size_t)syscall_argn(fdata_args, 2);

    pr_info("kpm: [UID %u] write(fd=%ld, count=%zu)\n",
            current_uid(), fd, count);
}

static void after_write(void *fdata_args)
{
    if (target_uid != (uid_t)-1 && current_uid() != target_uid) {
        return;
    }

    long fd = (long)syscall_argn(fdata_args, 0);
    long ret = (long)syscall_argn(fdata_args, 0);

    pr_info("kpm: [UID %u] write(fd=%ld) ret %ld\n",
            current_uid(), fd, ret);
}

static long hello_init(const char *args, const char *event, void *__user reserved)
{
    pr_info("kpm init, event: %s, args: %s, uid:%lu\n", event, args, current_uid());
    pr_info("kpm ver: %x\n", kpver);

    if (hook_syscalln(__NR_read, 3, before_read, after_read, NULL) == 0) {
        pr_info("kpm: Hooked read\n");
    } else {
        pr_err("kpm: Failed to hook read\n");
    }

    if (hook_syscalln(__NR_write, 3, before_write, after_write, NULL) == 0) {
        pr_info("kpm: Hooked write\n");
    } else {
        pr_err("kpm: Failed to hook write\n");
    }

    return 0;
}

static long hello_control0(const char *args, char *__user out_msg, int outlen)
{
    pr_info("kpm control0, args: %s\n", args);
    char echo[64] = "echo: ";
    strncat(echo, args, 48);
    compat_copy_to_user(out_msg, echo, sizeof(echo));

    unsigned long long val;
    int ret = kstrtoull(args, 10, &val);
    if (ret) {
        pr_info("kpm kstrtoull failed\n");
    } else {
        target_uid = (uid_t)val;
        pr_info("kpm new_uid %u\n", target_uid);
    }

    return 0;
}

static long hello_control1(void *a1, void *a2, void *a3)
{
    pr_info("kpm control1, a1: %llx, a2: %llx, a3: %llx\n", (unsigned long long)a1, (unsigned long long)a2, (unsigned long long)a3);
    return 0;
}

static long hello_exit(void *__user reserved)
{
    pr_info("kpm exit\n");

    unhook_syscalln(__NR_read, before_read, after_read);
    pr_info("kpm: Unhooked read\n");

    unhook_syscalln(__NR_write, before_write, after_write);
    pr_info("kpm: Unhooked write\n");

    return 0;
}

KPM_INIT(hello_init);
KPM_CTL0(hello_control0);
KPM_CTL1(hello_control1);
KPM_EXIT(hello_exit);
