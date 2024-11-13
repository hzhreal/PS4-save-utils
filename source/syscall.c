#include <stdarg.h>

#include "syscall.h"

long _syscall(long nr, ...) {
    long result;
    va_list args;

    va_start(args, nr);
    register long rax asm("rax")   = nr;
    register long a1  asm("rdi")   = va_arg(args, long);
    register long a2  asm("rsi")   = va_arg(args, long);
    register long a3  asm("rdx")   = va_arg(args, long);
    register long a4  asm("r10")   = va_arg(args, long);
    register long a5  asm("r8" )   = va_arg(args, long);
    register long a6  asm("r9" )   = va_arg(args, long);
    va_end(args);

    asm volatile(
        ".intel_syntax noprefix;"
        "syscall;"
        "jb err;"
        "jmp exit;"
    "err:"
        "push rax;"
        "call __error;"
        "pop rcx;"
        "mov [rax], ecx;"
        "mov rax, 0xFFFFFFFFFFFFFFFF;"
        "mov rdx, 0xFFFFFFFFFFFFFFFF;"
    "exit:"
        : "=a" (result)
        : "a" (rax), "D" (a1), "S" (a2), "d" (a3), "r" (a4), "r" (a5), "r" (a6)
        : "rcx", "r11", "memory"
    );

    return result;
}