#ifndef INIT_H
#define INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

int initCred(void);
int setupCred(void);
int initDevices(void);
int initAll(void);

int resolveStatfs(void);
bool checkStatfs(void);

#ifdef __cplusplus
}
#endif

#endif // INIT_H