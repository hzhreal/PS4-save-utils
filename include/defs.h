#ifndef DEFS_H
#define DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <orbis/libkernel.h>

#define LOG(msg) (sceKernelDebugOutText(0, msg))

#define UNUSED(x) (void)(x)

#define MAX_PATH_LEN 1024

#define PRIVDIR "/system/priv/lib"
#define COMMONDIR "/system/common/lib"

#define LOADMODULE(handle, path) \
    handle = sceKernelLoadStartModule(path, 0, NULL, 0, NULL, NULL);
#define RESOLVESYM(handle, sym, funcptr) \
    sceKernelDlsym(handle, sym, (void **)&funcptr);

#ifdef __cplusplus
}
#endif

#endif // DEFS_H