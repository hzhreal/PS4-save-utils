#ifndef _SYSCALL_H
#define _SYSCALL_H

#ifdef __cplusplus
extern "C" {
#endif

extern long _syscall(long nr, ...);

#ifdef __cplusplus
}
#endif

#endif // _SYSCALL_H