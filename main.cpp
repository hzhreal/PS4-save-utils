#include <time.h>
int __clock_gettime(clockid_t clock_id, struct timespec *tp) {
    return clock_gettime(clock_id, tp);
}
#include <orbis/libkernel.h>

#include "sd.h"
#include "init.h"
#include "dir.h"

#define EXAMPLE_SAVE_PATH "/data/HTOS/uploadencrypted"
#define EXAMPLE_SAVE_NAME "SAVEDATAPROFILE"
#define EXAMPLE_MOUNT_PATH "/data/HTOS/mountfolder/test"
#define EXAMPLE_COPY_PATH "/data/HTOS/example"

int main() {
    sceKernelDebugOutText(0, "HELLOOOOOO FROM KERNEL\n\n\n\n\n");

    // initialize cred
    if (init_cred() != 0) {
        sceKernelDebugOutText(0, "Failed to init cred\n");
        for(;;);
    }

    // load private libraries, do once after setting cred
    if (loadPrivLibs() != 0) {
        sceKernelDebugOutText(0, "Failed to load priv libs\n");
        for(;;);
    }

    // create devices
    if (init_devices() != 0) {
        sceKernelDebugOutText(0, "Failed to create devices\n");
        for(;;);
    }

    /*if (setup_cred() != 0) {
        sceKernelDebugOutText(0, "Failed to setup cred\n");
        for(;;);
    }*/

    // try to mount save
    if (mountSave(EXAMPLE_SAVE_PATH, EXAMPLE_SAVE_NAME, EXAMPLE_MOUNT_PATH) != 0) {
        sceKernelDebugOutText(0, "Failed to mount save\n");
        for(;;);
    }
    
    if (copydir(EXAMPLE_MOUNT_PATH, EXAMPLE_COPY_PATH) != 0) {
        sceKernelDebugOutText(0, "Failed to copy dir\n");
    }
    sceKernelSleep(20);

    // now unmount save
    if (umountSave(EXAMPLE_MOUNT_PATH, 0, 0) != 0) {
        sceKernelDebugOutText(0, "Failed to unmount save\n");
        for(;;);
    }

    sceKernelDebugOutText(0, "GOODBYE FROM KERNEL\n\n\n\n\n");
    for(;;);

    return 0;
}