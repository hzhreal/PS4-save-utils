#include <stdio.h>
#include <unistd.h>
extern "C" {
    #include "sd.h"
    #include "scall.h"
    #include "jailbreak.h"
}

int main() {
    sceKernelDebugOutText(0, "HELLOOOOOO FROM KERNEL\n\n\n\n\n");
    struct stat s;
    jbc_cred old_cred;
    jbc_cred cred;

    memset(&old_cred, 0, sizeof(jbc_cred));
    memset(&cred, 0, sizeof(jbc_cred));

    if (jbc_get_cred(&old_cred) != 0) {
        sceKernelDebugOutText(0, "Failed to get cred\n");
        for(;;);
    }
    old_cred.sonyCred = old_cred.sonyCred | 0x4000000000000000ULL;

    if (jbc_get_cred(&cred) != 0) {
        sceKernelDebugOutText(0, "Failed to get cred\n");
        for(;;);
    }
    cred.sonyCred = cred.sonyCred | 0x4000000000000000ULL;
    cred.sceProcType = 0x3801000000000013ULL;
    jbc_set_cred(&cred);
    setuid(0);

    // Load private libs
    if (loadPrivLibs() != 0) {
        sceKernelDebugOutText(0, "Failed to load priv libs\n");
        for(;;);
    }

    // Mount required devices into sandbox
    if (jbc_mount_in_sandbox("/dev/", "rootdev") != 0) {
        sceKernelDebugOutText(0, "Failed to mount devices\n");
        for(;;);
    }

    if (stat("/rootdev/pfsctldev", &s) == 0) {
        if (mknod("/dev/pfsctldev", S_IFCHR | 0777, s.st_dev) != 0) {
            sceKernelDebugOutText(0, "mknod error\n");
            for(;;);
        }
    }

    if (jbc_unmount_in_sandbox("rootdev") != 0) {
        sceKernelDebugOutText(0, "Failed to unmount rootdev\n");
        for(;;);
    }

    // Get max keyset that can be decrypted
    getMaxKeySet();

    jbc_set_cred(&old_cred);

    if (mountSave("/data/HTOS/uploadencrypted", "SAVEDATASGTA50000", "/data/HTOS/mount/test") != 0) {
        sceKernelDebugOutText(0, "Failed to mount save\n");
        for(;;);
    }
    for(;;);

    return 0;
}