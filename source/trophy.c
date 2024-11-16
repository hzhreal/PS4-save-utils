#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>

#include "../ps4-libjbc/utils.h"
#include "defs.h"
#include "sealedkey.h"
#include "init.h"
#include "trophy.h"

typedef struct {
    int blocksize;
    uint8_t reserved[2];
} CreatePfsTrophyDataOpt;

typedef struct {
    uint8_t reserved;
    char *budgetid;
} MountTrophyDataOpt;

typedef struct {
    uint8_t dummy;
} UmountTrophyDataOpt;

int (*sceFsUfsAllocateTrophyData)(int fd, uint64_t imageSize, uint64_t imageFlags, int ext);
int (*sceFsInitCreatePfsTrophyDataOpt)(CreatePfsTrophyDataOpt *opt);
int (*sceFsCreatePfsTrophyDataImage)(CreatePfsTrophyDataOpt *opt, const char *volumePath, int, uint64_t volumeSize, uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN]);
int (*sceFsInitMountTrophyDataOpt)(MountTrophyDataOpt *opt);
int (*sceFsMountTrophyData)(MountTrophyDataOpt *opt, const char *volumePath, const char *mountPath, uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN]);
int (*sceFsInitUmountTrophyDataOpt)(UmountTrophyDataOpt *opt);
int (*sceFsUmountTrophyData)(UmountTrophyDataOpt *opt, const char *mountPath, int handle, bool ignoreErrors);

int loadTrophyLib(void) {
    int sys;

    if (jbc_mount_in_sandbox(PRIVDIR, "priv") != 0) {
        return -1;
    }

    LOADMODULE(sys, "/priv/libSceFsInternalForVsh.sprx");
    jbc_unmount_in_sandbox("priv");

    if (sys >= 0) {
        RESOLVESYM(sys, "sceFsInitCreatePfsTrophyDataOpt", sceFsInitCreatePfsTrophyDataOpt);
        RESOLVESYM(sys, "sceFsCreatePfsTrophyDataImage", sceFsCreatePfsTrophyDataImage);
        RESOLVESYM(sys, "sceFsUfsAllocateTrophyData", sceFsUfsAllocateTrophyData);
        RESOLVESYM(sys, "sceFsInitMountTrophyDataOpt", sceFsInitMountTrophyDataOpt);
        RESOLVESYM(sys, "sceFsMountTrophyData", sceFsMountTrophyData);
        RESOLVESYM(sys, "sceFsInitUmountTrophyDataOpt", sceFsInitUmountTrophyDataOpt);
        RESOLVESYM(sys, "sceFsUmountTrophyData", sceFsUmountTrophyData);
    }
    else {
        return -2;
    }

    if (checkStatfs() == false) {
        if (resolveStatfs() != 0) {
            return -3;
        }
    }

    return 0;
}

int createTrophy(const char *folder, int blocks) {
    uint8_t sealedKey[ENC_SEALEDKEY_LEN] = {0};
    uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN] = {0};
    uint64_t volumeSize;
    char volumePath[MAX_PATH_LEN] = {0};
    char volumeKeyPath[MAX_PATH_LEN] = {0};
    int fd;
    CreatePfsTrophyDataOpt opt;
    
    // generate a key
    if (generateSealedKey(sealedKey) != 0) {
        return -1;
    }

    // decrypt the generated key
    if (decryptSealedKey(sealedKey, decryptedSealedKey) != 0) {
        return -2;
    }

    snprintf(volumePath, sizeof(volumePath), "%s/trophy.img", folder);
    snprintf(volumeKeyPath, sizeof(volumeKeyPath), "%s/sealedkey", folder);

    fd = sceKernelOpen(volumeKeyPath, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd == -1) {
        return -3;
    }

    // write sealed key
    if (sceKernelWrite(fd, sealedKey, sizeof(sealedKey)) != sizeof(sealedKey)) {
        sceKernelClose(fd);
        return -4;
    }
    sceKernelClose(fd);

    fd = sceKernelOpen(volumePath, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd == -1) {
        return -5;
    }

    volumeSize = blocks << 15;

    if (sceFsUfsAllocateTrophyData(fd, volumeSize, 0 << 7, 0) < 0) {
        sceKernelClose(fd);
        return -6;
    }
    sceKernelClose(fd);

    if (sceFsInitCreatePfsTrophyDataOpt(&opt) < 0) {
        return -7;
    }

    if (sceFsCreatePfsTrophyDataImage(&opt, volumePath, 0, volumeSize, decryptedSealedKey) < 0) {
        return -8;
    }

    // finalize
    fd = sceKernelOpen(volumePath, O_RDONLY, 0);
    sceKernelFsync(fd);
    sceKernelClose(fd);
    
    return 0;
}

int mountTrophy(const char *folder, const char *mountPath) {
    char volumeKeyPath[MAX_PATH_LEN] = {0};
    char volumePath[MAX_PATH_LEN] = {0};
    uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN] = {0};
    MountTrophyDataOpt opt;
    int ret;

    snprintf(volumePath, sizeof(volumePath), "%s/trophy.img", folder);
    snprintf(volumeKeyPath, sizeof(volumeKeyPath), "%s/sealedkey", folder);

    ret = decryptSealedKeyAtPath(volumeKeyPath, decryptedSealedKey);
    if (ret < 0) {
        return ret;
    }

    sceFsInitMountTrophyDataOpt(&opt);
    opt.budgetid = "system";

    ret = sceFsMountTrophyData(&opt, volumePath, mountPath, decryptedSealedKey);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int mountTrophyAny(const char *volumePath, const char *volumeKeyPath, const char *mountPath) {
    uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN] = {0};
    MountTrophyDataOpt opt;

    int ret = decryptSealedKeyAtPath(volumeKeyPath, decryptedSealedKey);
    if (ret < 0) {
        return ret;
    }

    sceFsInitMountTrophyDataOpt(&opt);
    opt.budgetid = "system";

    ret = sceFsMountTrophyData(&opt, volumePath, mountPath, decryptedSealedKey);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int umountTrophy(const char *mountPath, int handle, bool ignoreErrors) {
    UmountTrophyDataOpt opt;
    sceFsInitUmountTrophyDataOpt(&opt);
    return sceFsUmountTrophyData(&opt, mountPath, handle, ignoreErrors);
}