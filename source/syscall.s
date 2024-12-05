.intel_syntax noprefix

.text
.globl _syscall

_syscall:
    xor rax, rax
    mov r10, rcx
    syscall
    ret