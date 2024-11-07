#ifndef MKNOD_H
#define MKNOD_H

#include <sys/stat.h>
#include <syscall.h>

#include "defs.h"

int sys_mknod(const char *path, mode_t mode, dev_t dev) {
    long result;

    asm volatile(
        "movq %1, %%rax;" // syscall value
        "movq %2, %%rdi;" // path
        "movl %3, %%esi;" // mode
        "movl %4, %%edx;" // dev
        "syscall;" // invoke syscall
        "movq %%rax, %0;" // store result
        : "=r" (result)
        : "i" (SYS_mknod), "r" (path), "r" (mode), "r" (dev)
        : "rax", "rdi", "rsi", "rdx"
    );

    if (result < 0) {
        return -1;
    }

    return (int)result;
}

#endif // MKNOD_H