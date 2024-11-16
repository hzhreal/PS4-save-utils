#ifndef PKG_H
#define PKG_H

#ifdef __cplusplus
extern "C" {
#endif

int loadPkgLib(void);
int mountPkg(const char *volumePath, const char *mountPath);
int umountPkg(const char *mountPath, int handle, bool ignoreErrors);

#ifdef __cplusplus
}
#endif

#endif // PKG_H