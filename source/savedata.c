#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <orbis/libkernel.h>

#include "savedata.h"
#include "../ps4-libjbc/utils.h"
#include "init.h"
#include "defs.h"

typedef struct {
    int blocksize;
    uint8_t reserved[2];
} CreatePfsSaveDataOpt;

typedef struct {
    uint8_t reserved;
    char *budgetid;
} MountSaveDataOpt;

typedef struct {
    uint8_t dummy;
} UmountSaveDataOpt;

int (*sceFsUfsAllocateSaveData)(int fd, uint64_t imageSize, uint64_t imageFlags, int ext);
int (*sceFsInitCreatePfsSaveDataOpt)(CreatePfsSaveDataOpt *opt);
int (*sceFsCreatePfsSaveDataImage)(CreatePfsSaveDataOpt *opt, const char *volumePath, int unk, uint64_t volumeSize, uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN]);
int (*sceFsInitMountSaveDataOpt)(MountSaveDataOpt *opt);
int (*sceFsMountSaveData)(MountSaveDataOpt *opt, const char *volumePath, const char *mountPath, uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN]);
int (*sceFsInitUmountSaveDataOpt)(UmountSaveDataOpt *opt);
int (*sceFsUmountSaveData)(UmountSaveDataOpt *opt, const char *mountPath, int handle, bool ignoreErrors);

// must be loaded upon setup
int loadSaveDataLib(void) {
    int sys;

    if (jbc_mount_in_sandbox(PRIVDIR, "priv") != 0) {
        return -1;
    }

    LOADMODULE(sys, "/priv/libSceFsInternalForVsh.sprx");
    jbc_unmount_in_sandbox("priv");

    if (sys >= 0) {
        RESOLVESYM(sys, "sceFsInitCreatePfsSaveDataOpt", sceFsInitCreatePfsSaveDataOpt);
        RESOLVESYM(sys, "sceFsCreatePfsSaveDataImage", sceFsCreatePfsSaveDataImage);
        RESOLVESYM(sys, "sceFsUfsAllocateSaveData", sceFsUfsAllocateSaveData);
        RESOLVESYM(sys, "sceFsInitMountSaveDataOpt", sceFsInitMountSaveDataOpt);
        RESOLVESYM(sys, "sceFsMountSaveData", sceFsMountSaveData);
        RESOLVESYM(sys, "sceFsInitUmountSaveDataOpt", sceFsInitUmountSaveDataOpt);
        RESOLVESYM(sys, "sceFsUmountSaveData", sceFsUmountSaveData);
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

int createSave(const char *folder, const char *saveName, int blocks) {
    uint8_t sealedKey[ENC_SEALEDKEY_LEN] = {0};
    uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN] = {0};
    uint64_t volumeSize;
    char volumePath[MAX_PATH_LEN] = {0};
    char volumeKeyPath[MAX_PATH_LEN] = {0};
    int fd;
    CreatePfsSaveDataOpt opt;
    
    // generate a key
    if (generateSealedKey(sealedKey) != 0) {
        return -1;
    }

    // decrypt the generated key
    if (decryptSealedKey(sealedKey, decryptedSealedKey) != 0) {
        return -2;
    }

    snprintf(volumePath, sizeof(volumePath), "%s/%s", folder, saveName);
    snprintf(volumeKeyPath, sizeof(volumeKeyPath), "%s/%s.bin", folder, saveName);

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

    if (sceFsUfsAllocateSaveData(fd, volumeSize, 0 << 7, 0) < 0) {
        sceKernelClose(fd);
        return -6;
    }
    sceKernelClose(fd);

    if (sceFsInitCreatePfsSaveDataOpt(&opt) < 0) {
        return -7;
    }

    if (sceFsCreatePfsSaveDataImage(&opt, volumePath, 0, volumeSize, decryptedSealedKey) < 0) {
        return -8;
    }

    // finalize
    fd = sceKernelOpen(volumePath, O_RDONLY, 0);
    sceKernelFsync(fd);
    sceKernelClose(fd);
    
    return 0;
}

int mountSave(const char *folder, const char *saveName, const char *mountPath) {
    char volumeKeyPath[MAX_PATH_LEN] = {0};
    char volumePath[MAX_PATH_LEN] = {0};
    uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN] = {0};
    MountSaveDataOpt opt;
    int ret;

    snprintf(volumePath, sizeof(volumePath), "%s/%s", folder, saveName);
    snprintf(volumeKeyPath, sizeof(volumeKeyPath), "%s/%s.bin", folder, saveName);

    ret = decryptSealedKeyAtPath(volumeKeyPath, decryptedSealedKey);
    if (ret < 0) {
        return ret;
    }

    sceFsInitMountSaveDataOpt(&opt);
    opt.budgetid = "system";

    ret = sceFsMountSaveData(&opt, volumePath, mountPath, decryptedSealedKey);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int mountSaveAny(const char *volumePath, const char *volumeKeyPath, const char *mountPath) {
    uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN] = {0};
    MountSaveDataOpt opt;

    int ret = decryptSealedKeyAtPath(volumeKeyPath, decryptedSealedKey);
    if (ret < 0) {
        return ret;
    }

    sceFsInitMountSaveDataOpt(&opt);
    opt.budgetid = "system";

    ret = sceFsMountSaveData(&opt, volumePath, mountPath, decryptedSealedKey);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int umountSave(const char *mountPath, int handle, bool ignoreErrors) {
    UmountSaveDataOpt opt;
    sceFsInitUmountSaveDataOpt(&opt);
    return sceFsUmountSaveData(&opt, mountPath, handle, ignoreErrors);
}