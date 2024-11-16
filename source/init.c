#include <unistd.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <orbis/libkernel.h>
#include <string.h>

#include "../ps4-libjbc/jailbreak.h"
#include "../ps4-libjbc/utils.h"
#include "init.h"
#include "syscall.h"
#include "savedata.h"
#include "trophy.h"
#include "pkg.h"
#include "defs.h"

void (*statfs)(void);

static inline int _mknod(const char *path, mode_t mode, dev_t dev) {
    return (int)_syscall(SYS_mknod, path, mode, dev);
}

// cred must be set to invoke mount call, use before anything
int initCred(void) {
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
int setupCred(void) {
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
int initDevices(void) {
    struct stat s;
    memset(&s, 0, sizeof(struct stat));

    // mount required devices into sandbox
    if (jbc_mount_in_sandbox("/dev/", "rootdev") != 0) {
        return -1;
    }

    // create devices
    if (stat("/rootdev/pfsctldev", &s) == -1) {
        return -2;
    }
    else {
        _mknod("/dev/pfsctldev", S_IFCHR | 0777, s.st_dev);
    }

    memset(&s, 0, sizeof(struct stat));

    if (stat("/rootdev/lvdctl", &s) == -1) {
        return -3;
    }
    else {
        _mknod("/dev/lvdctl", S_IFCHR | 0777, s.st_dev);
    }

    memset(&s, 0, sizeof(struct stat));

    if (stat("/rootdev/sbl_srv", &s) == -1) {
        return -4;
    }
    else {
        _mknod("/dev/sbl_srv", S_IFCHR | 0777, s.st_dev);
    }

    // now unmount devices
    jbc_unmount_in_sandbox("rootdev");

    // get max keyset that can be decrypted
    getMaxKeySet();

    return 0;
}

int initAll(void) {
    if (initCred() != 0) {
        return -1;
    }
    if (initDevices() != 0) {
        return -2;
    }
    if (loadSaveDataLib() != 0) {
        return -3;
    }
    if (loadTrophyLib() != 0) {
        return -4;
    }
    if (loadPkgLib() != 0) {
        return -5;
    }
    return 0;
}

int resolveStatfs(void) {
    int handle;

    if (jbc_mount_in_sandbox(COMMONDIR, "common") != 0) {
        return -1;
    }
    LOADMODULE(handle, "/common/libkernel_sys.sprx");
    jbc_unmount_in_sandbox("common");

    if (handle < 0) {
        return -2;
    }
    RESOLVESYM(handle, "statfs", statfs);

    return 0;
}

bool checkStatfs(void) {
    if (statfs == NULL) {
        return false;
    }
    return true;
}