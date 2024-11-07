#ifndef DEFS_H
#define DEFS_H

#include <orbis/libkernel.h>

#define LOG(msg) (sceKernelDebugOutText(0, msg))
#define UNUSED(x) (void)(x)
#define MAX_PATH_LEN 1024

#endif // DEFS_H