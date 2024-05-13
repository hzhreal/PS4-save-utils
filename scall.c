#include "scall.h"

int sys_open(const char *path, int flags, int mode) {
    int result;
    int err;
    
    asm (
        ".intel_syntax;"
        "mov rax, 5;"       // System call number for open: 5
        "syscall;"          // Invoke syscall
        : "=a" (result),    // Output operand: result
          "=@ccc" (err)     // Output operand: err (clobbers condition codes)
    );

    return result;
}