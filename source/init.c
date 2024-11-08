#include <unistd.h>
#include <sys/stat.h>
#include <orbis/libkernel.h>
#include <string.h>

#include "../ps4-libjbc/jailbreak.h"
#include "../ps4-libjbc/utils.h"
#include "init.h"
#include "savedata.h"
#include "defs.h"
#include "mknod.h"

// cred must be set to invoke mount call, use before anything
int init_cred(void) {
    struct jbc_cred old_cred;
    struct jbc_cred cred;

    memset(&old_cred, 0, sizeof(struct jbc_cred));
    memset(&cred, 0, sizeof(struct jbc_cred));

    if (jbc_get_cred(&old_cred) != 0) {
        return -1;
    }
    old_cred.sonyCred = old_cred.sonyCred | 0x4000000000000000ULL;

    if (jbc_get_cred(&cred) != 0) {
        return -2;
    }
    cred.sonyCred = cred.sonyCred | 0x4000000000000000ULL;
    cred.sceProcType = 0x3801000000000013ULL;

    if (jbc_set_cred(&cred) != 0) {
        return -3;
    }
    setuid(0);

    return 0;
}

// can use before mount call after initializing everything
int setup_cred(void) {
    struct jbc_cred cred;
    memset(&cred, 0, sizeof(struct jbc_cred));

    if (jbc_get_cred(&cred) != 0) {
        return -1;
    }
    cred.sonyCred = cred.sonyCred | 0x4000000000000000ULL;
    cred.sceProcType = 0x3801000000000013ULL;

    if (jbc_set_cred(&cred) != 0) {
        return -2;
    }
    setuid(0);

    return 0;
}

// create devices, do once after setting cred and loading priv libs
int init_devices(void) {
    struct stat s;
    memset(&s, 0, sizeof(struct stat));

    // mount required devices into sandbox
    if (jbc_mount_in_sandbox("/dev/", "rootdev") != 0) {
        LOG("Failed to mount devices\n");
        return -1;
    }

    // create devices
    if (stat("/rootdev/pfsctldev", &s) == -1) {
        LOG("err stat pfsctldev\n");
        return -2;
    }
    else {
        if (sys_mknod("/dev/pfsctldev", S_IFCHR | 0777, s.st_dev) == -1) {
            LOG("err mknod pfsctldev\n");
            return -2;
        }
    }

    memset(&s, 0, sizeof(struct stat));

    if (stat("/rootdev/lvdctl", &s) == -1) {
        LOG("err stat lvdctl\n");
        return -3;
    }
    else {
        if (sys_mknod("/dev/lvdctl", S_IFCHR | 0777, s.st_dev) == -1) {
            LOG("err mknod lvdctl\n");
            return -3;
        }
    }

    memset(&s, 0, sizeof(struct stat));

    if (stat("/rootdev/sbl_srv", &s) == -1) {
        LOG("err stat sbl_srv\n");
        return -4;
    }
    else {
        if (sys_mknod("/dev/sbl_srv", S_IFCHR | 0777, s.st_dev) == -1) {
            LOG("err mknod sbl_srv\n");
            return -4;
        }
    }

    // now unmount devices
    if (jbc_unmount_in_sandbox("rootdev") != 0) {
        LOG("Failed to unmount rootdev\n");
    }

    // get max keyset that can be decrypted
    getMaxKeySet();

    return 0;
}