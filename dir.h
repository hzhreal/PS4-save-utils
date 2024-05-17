#ifndef DIR_H
#define DIR_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PATH_LEN 1024
#define SINGLE_MAX_PATH_LEN 256

int copyfile(const char *src, const char *dst);
int copydir(const char *src, const char *dst);

#ifdef __cplusplus
}
#endif

#endif // DIR_H