#include <stdlib.h>
#include <immintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../ps4-libjbc/utils.h"
#include "sealedkey.h"
#include "init.h"
#include "defs.h"
#include "pkg.h"

typedef struct {
    const char *pfsImgFile;
    uint8_t cache[64];
    char *budgetid;
} MountGamePkgOpt;

typedef struct {
    uint32_t reserved;
} UmountGamePkgOpt;

typedef struct {
    uint64_t unk1;
    uint64_t unk2;
    uint64_t volumeSize;
} i_pConfig;

int (*sceFsInitMountGamePkgOpt)(MountGamePkgOpt *opt);
int (*sceFsMountGamePkg)(MountGamePkgOpt *opt, const char *mountPath, uint8_t mountKey[DEC_SEALEDKEY_LEN], int unk1, int unk2, int unk3);
int (*sceFsInitUmountGamePkgOpt)(UmountGamePkgOpt *opt);
int (*sceFsUmountGamePkg)(UmountGamePkgOpt *opt, const char *mountPath, int handle, bool ignoreErrors);

int loadPkgLib(void) {
    int sys;

    if (jbc_mount_in_sandbox(PRIVDIR, "priv") != 0) {
        return -1;
    }

    LOADMODULE(sys, "/priv/libSceFsInternalForVsh.sprx");
    jbc_unmount_in_sandbox("priv");

    if (sys >= 0) {
        RESOLVESYM(sys, "sceFsInitMountGamePkgOpt", sceFsInitMountGamePkgOpt);
        RESOLVESYM(sys, "sceFsMountGamePkg", sceFsMountGamePkg);
        RESOLVESYM(sys, "sceFsInitUmountGamePkgOpt", sceFsInitUmountGamePkgOpt);
        RESOLVESYM(sys, "sceFsUmountGamePkg", sceFsUmountGamePkg);
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

int mountPkg(const char *volumePath, const char *mountPath) {
    uint8_t sealedKey[ENC_SEALEDKEY_LEN] = {0};
    uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN] = {0};
    MountGamePkgOpt opt;

    // generate a key
    if (generateSealedKey(sealedKey) != 0) {
        return -1;
    }

    // decrypt the generated key
    if (decryptSealedKey(sealedKey, decryptedSealedKey) != 0) {
        return -2;
    }

    sceFsInitMountGamePkgOpt(&opt);
    opt.pfsImgFile = volumePath;
    opt.budgetid = "system";
    LOG("\n\n\n\n\n\n\n\n");

    int ret = sceFsMountGamePkg(&opt, mountPath, decryptedSealedKey, 0, 0, 0);
    LOG("\n\n\n\n\n\n\n\n");
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int umountPkg(const char *mountPath, int handle, bool ignoreErrors) {
    UmountGamePkgOpt opt;
    sceFsInitUmountGamePkgOpt(&opt);
    return sceFsUmountGamePkg(&opt, mountPath, handle, ignoreErrors);
}