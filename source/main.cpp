#include <orbis/libkernel.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "savedata.h"
#include "trophy.h"
#include "pkg.h"
#include "init.h"
#include "dir.h"
#include "defs.h"

#define EXAMPLE_SAVE_PATH "/data/utils/upl"
#define EXAMPLE_PKG_PATH "/data/pkg/IV0000-HZHZ00001_00-SAVEDATATS000000.pkg"
#define EXAMPLE_SAVE_NAME "SAVEDATASBGTA50002"
#define EXAMPLE_MOUNT_PATH_SAVE "/data/utils/mount/save"
#define EXAMPLE_MOUNT_PATH_PKG "/data/utils/mount/pkg"
#define EXAMPLE_COPY_PATH "/data/utils/store"
#define EXAMPLE_CREATE_PATH "/data/saves/create"

int main(void) {
    LOG("HELLOOOOOO FROM KERNEL\n\n\n\n\n");

    // initialize cred
    if (initCred() != 0) {
        LOG("Failed to init cred\n");
        for(;;);
    }

    // create devices
    if (initDevices() != 0) {
        LOG("Failed to create devices\n");
        for(;;);
    }

    // load libraries, do once after setting cred
    if (loadSaveDataLib() != 0) {
        LOG("Failed to load savedata lib\n");
        for(;;);
    }
    if (loadTrophyLib() != 0) {
        LOG("Failed to load trophy lib\n");
        for(;;);
    }
    if (loadPkgLib() != 0) {
        LOG("Failed to load pkg lib\n");
        for(;;);
    }

    /*if (setup_cred() != 0) {
        LOG("Failed to setup cred\n");
        for(;;);
    }*/

    // try to mount save
    if (mountSave(EXAMPLE_SAVE_PATH, EXAMPLE_SAVE_NAME, EXAMPLE_MOUNT_PATH_SAVE) != 0) {
        LOG("Failed to mount save\n");
        for(;;);
    }
    if (copydir(EXAMPLE_MOUNT_PATH_SAVE, EXAMPLE_COPY_PATH) != 0) {
        LOG("Failed to copy dir\n");
    }

    // now unmount save
    if (umountSave(EXAMPLE_MOUNT_PATH_SAVE, 0, 0) != 0) {
        LOG("Failed to unmount save\n");
        for(;;);
    }

    // try to mount pkg
    if (mountPkg(EXAMPLE_PKG_PATH, EXAMPLE_MOUNT_PATH_PKG) != 0) {
        LOG("Failed to mount pkg\n");
        for(;;);
    }
    if (copydir(EXAMPLE_MOUNT_PATH_PKG, EXAMPLE_COPY_PATH) != 0) {
        LOG("Failed to copy dir\n");
    }

    // now unmount pkg
    if (umountPkg(EXAMPLE_MOUNT_PATH_PKG, 0, 0) != 0) {
        LOG("Failed to unmount pkg\n");
        for(;;);
    }

    // try to create a save
    if (createSave(EXAMPLE_CREATE_PATH, EXAMPLE_SAVE_NAME, 123) != 0) {
        LOG("Failed to create save\n");
        for(;;);
    }

    // try to create a trophy
    if (createTrophy(EXAMPLE_CREATE_PATH, 123) != 0) {
        LOG("Failed to create trophy\n");
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