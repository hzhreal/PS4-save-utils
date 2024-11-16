#ifndef SAVEDATA_H
#define SAVEDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "sealedkey.h"

int loadSaveDataLib(void);
int createSave(const char *folder, const char *saveName, int blocks);
int mountSave(const char *folder, const char *saveName, const char *mountPath);
int mountSaveAny(const char *volumePath, const char *volumeKeyPath, const char *mountPath);
int umountSave(const char *mountPath, int handle, bool ignoreErrors);

#ifdef __cplusplus
}
#endif

#endif // SAVEDATA_H