#ifndef TROPHY_H
#define TROPHY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "sealedkey.h"

int loadTrophyLib(void);
int createTrophy(const char *folder, int blocks);
int mountTrophy(const char *folder, const char *mountPath);
int mountTrophyAny(const char *volumePath, const char *volumeKeyPath, const char *mountPath);
int umountTrophy(const char *mountPath, int handle, bool ignoreErrors);

#ifdef __cplusplus
}
#endif

#endif // TROPHY_H