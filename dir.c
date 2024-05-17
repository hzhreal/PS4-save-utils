#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <orbis/libkernel.h>
#include <stdint.h>

#include "dir.h"
#include "scall.h"

int copyfile(const char *src, const char *dst) {
    int fd_src = -1;
    int fd_dst = -1;
    int ret = 0;
    uint8_t buf[65536]; // 64KB chunk size
    ssize_t bytesRead, bytesWritten;

    // open source file
    fd_src = open(src, O_RDONLY, 0);
    if (fd_src == -1) {
        ret = -1;
        goto clean;  
    }

    // open destination file
    fd_dst = open(dst, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if (fd_dst == -1) {
        ret = -2;
        goto clean;
    }

    // write to other file in chunks until end
    while ((bytesRead = read(fd_src, buf, sizeof(buf))) > 0) {
        bytesWritten = write(fd_dst, buf, bytesRead);
        if (bytesWritten != bytesRead) {
            ret = -3;
            goto clean;
        }
    }
    
    if (bytesRead == -1) {
        ret = -4;
    }

    clean:
        if (fd_src != -1) {
            close(fd_src);
        }
        if (fd_dst != -1) {
            close(fd_dst);
        }
        return ret;
}

int copydir(const char *src, const char *dst) {
    DIR *dir;
    struct dirent *entry = NULL;
    char basename[SINGLE_MAX_PATH_LEN];
    char new_dst[MAX_PATH_LEN];

    // find folder name
    char *last_part = strrchr(src, '/');
    if (last_part) {
        strcpy(basename, last_part + 1);
    }
    else {
        // '/' not found, copy entire path
        strcpy(basename, src);
    }

    mkdir(dst, 0777);

    dir = opendir(src);
    if (!dir) {
        return -1;
    }

    while ((entry = readdir(dir))) {
        // skip "." and ".." to avoid infinite loops
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        struct stat s;
        char path[MAX_PATH_LEN];

        memset(&s, 0, sizeof(struct stat));
        snprintf(path, sizeof(path), "%s/%s", src, entry->d_name);

        // check if dir
        if (entry->d_type == DT_DIR) {
            // check if exists in src
            if (stat(path, &s) == -1) {
                continue;
            }
            snprintf(new_dst, sizeof(new_dst), "%s/%s", dst, entry->d_name);
            copydir(path, new_dst);        
        }
        // check if file
        else if (entry->d_type == DT_REG) {
            // check if exists in src
            if (stat(path, &s) == -1) {
                continue;
            }

            // treat files specially
            if (strcmp(entry->d_name, "memory.dat") == 0 || strcmp(basename, "sce_sys") == 0) {
                setuid(0);
            }
            else {
                setuid(1);
            }
            snprintf(new_dst, sizeof(new_dst), "%s/%s", dst, entry->d_name);
            if (copyfile(path, new_dst) != 0) {
                return -2;
            }
        }
    }

    setuid(0);
    closedir(dir);
    return 0;
}