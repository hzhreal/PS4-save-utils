#include <orbis/libkernel.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "savedata.h"
#include "init.h"
#include "dir.h"
#include "defs.h"

#define EXAMPLE_SAVE_PATH "/data/saves/upl"
#define EXAMPLE_SAVE_NAME "SAVEDATASBGTA50002"
#define EXAMPLE_MOUNT_PATH "/data/saves/mount"
#define EXAMPLE_COPY_PATH "/data/saves/store"
#define EXAMPLE_CREATE_PATH "/data/saves/create"

int main(void) {
    LOG("HELLOOOOOO FROM KERNEL\n\n\n\n\n");

    // initialize cred
    if (init_cred() != 0) {
        LOG("Failed to init cred\n");
        for(;;);
    }

    // load private libraries, do once after setting cred
    if (loadPrivLibs() != 0) {
        LOG("Failed to load priv libs\n");
        for(;;);
    }

    // create devices
    if (init_devices() != 0) {
        LOG("Failed to create devices\n");
        for(;;);
    }

    /*if (setup_cred() != 0) {
        LOG("Failed to setup cred\n");
        for(;;);
    }*/

    // try to mount save
    if (mountSave(EXAMPLE_SAVE_PATH, EXAMPLE_SAVE_NAME, EXAMPLE_MOUNT_PATH) != 0) {
        LOG("Failed to mount save\n");
        for(;;);
    }
    
    if (copydir(EXAMPLE_MOUNT_PATH, EXAMPLE_COPY_PATH) != 0) {
        LOG("Failed to copy dir\n");
    }

    sceKernelSleep(20);

    // now unmount save
    if (umountSave(EXAMPLE_MOUNT_PATH, 0, 0) != 0) {
        LOG("Failed to unmount save\n");
        for(;;);
    }

    // try to create a save
    if (createSave(EXAMPLE_CREATE_PATH, EXAMPLE_SAVE_NAME, 123) != 0) {
        LOG("Failed to create save\n");
        for(;;);
    }

    uint16_t maxKeyset = getMaxKeySet();
    
    OrbisNotificationRequest req;
    memset(&req, '\0', sizeof(req));
    req.reqId = NotificationRequest;
    req.unk3 = 0;
    req.useIconImageUri = 0;
    req.targetId = -1;
    snprintf(req.message, sizeof(req.message), "Success!\nKeyset: %" PRIu16 "\n", maxKeyset);
    sceKernelSendNotificationRequest(NotificationRequest, &req, sizeof(req), 0);

    LOG("GOODBYE FROM KERNEL\n\n\n\n\n");
    for(;;);

    return 0;
}