extern "C" {
    #include "sd.h"
    #include "init.h"
}

#define EXAMPLE_SAVE_PATH "/data/HTOS/uploadencrypted"
#define EXAMPLE_SAVE_NAME "STORM4.S"
#define EXAMPLE_MOUNT_PATH "/data/HTOS/mountfolder/test"

int main() {
    sceKernelDebugOutText(0, "HELLOOOOOO FROM KERNEL\n\n\n\n\n");
    
    // initialize cred
    if (init_cred() != 0) {
        sceKernelDebugOutText(0, "Failed to init cred\n");
        for(;;);
    }

    // create devices
    if (init_devices() != 0) {
        sceKernelDebugOutText(0, "Failed to create devices\n");
        for(;;);
    }

    // try to mount save
    if (mountSave(EXAMPLE_SAVE_PATH, EXAMPLE_SAVE_NAME, EXAMPLE_MOUNT_PATH) != 0) {
        sceKernelDebugOutText(0, "Failed to mount save\n");
        for(;;);
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