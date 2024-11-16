.intel_syntax noprefix

.text
.global _syscall

_syscall:
    xor rax, rax
    mov r10, rcx
    syscall
    jb err
    jmp exit
err:
    push rax
    call __error
    pop rcx
    mov [rax], ecx
    mov rax, 0xFFFFFFFFFFFFFFFF
    mov rdx, 0xFFFFFFFFFFFFFFFF
exit:
    ret