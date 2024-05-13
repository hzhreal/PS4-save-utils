#ifndef SCALL_H
#define SCALL_H

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int sys_open(const char *path, int flags, int mode);

#endif // SCALL_H