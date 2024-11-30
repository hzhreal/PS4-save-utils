#include <orbis/libkernel.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

#include "savedata.h"
#include "trophy.h"
#include "pkg.h"
#include "init.h"
#include "dir.h"
#include "defs.h"

#define PORT 1234
#define MAX_CONNECTIONS 16
#define BUF_LEN 1024
#define RANDSTR_LEN 10

const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static void genRandomStr(char *str, int len) {
    for (int i = 0; i < len; i++) {
        str[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    str[len] = '\0'; 
}

static int strToInt(const char *str, int base, int *val) {
    char *end = NULL;
    long l;

    errno = 0;
    l = strtol(str, &end, base);

    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX)) {
        return -1;
    }
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN)) {
        return -2;
    }
    if (*end != '\0') {
        return -3;
    }

    *val = (int)l;

    return 0;
}

static void removeTrailingWhitespace(char *str) {
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
}

static void cmdDumpSave(const char *saveFolder, const char *saveName, const char *mountFolder, char res[BUF_LEN]) {
    char tempMountPath[MAX_PATH_LEN] = {0};
    char randstr[RANDSTR_LEN + 1] = {0};
    genRandomStr(randstr, RANDSTR_LEN);
    snprintf(tempMountPath, sizeof(tempMountPath), "/data/%s", randstr);
    rmdir(tempMountPath);
    mkdir(tempMountPath, 0777);

    if (mountSave(saveFolder, saveName, tempMountPath) < 0) {
        snprintf(res, BUF_LEN, "Failed to mount: %s\n\n", strerror(errno));
        return;
    }
    if (copydir(tempMountPath, mountFolder) < 0) {
        snprintf(res, BUF_LEN, "Failed to copy dir\n\n");
    }
    umountSave(tempMountPath, 0, 0);
    rmdir(tempMountPath);
}

static void cmdUpdateSave(const char *saveFolder, const char *saveName, const char *sourceFolder, char res[BUF_LEN]) {
    char tempMountPath[MAX_PATH_LEN] = {0};
    char randstr[RANDSTR_LEN + 1] = {0};
    genRandomStr(randstr, RANDSTR_LEN);
    snprintf(tempMountPath, sizeof(tempMountPath), "/data/%s", randstr);
    rmdir(tempMountPath);
    mkdir(tempMountPath, 0777);

    if (mountSave(saveFolder, saveName, tempMountPath) < 0) {
        snprintf(res, BUF_LEN, "Failed to mount: %s\n\n", strerror(errno));
        return;
    }
    if (copydir(sourceFolder, tempMountPath) < 0) {
        snprintf(res, BUF_LEN, "Failed to copy dir\n\n");
    }
    umountSave(tempMountPath, 0, 0);
    rmdir(tempMountPath);
}

static void cmdDumpTrophy(const char *folder, const char *mountFolder, char res[BUF_LEN]) {
    char tempMountPath[MAX_PATH_LEN] = {0};
    char randstr[RANDSTR_LEN + 1] = {0};
    genRandomStr(randstr, RANDSTR_LEN);
    snprintf(tempMountPath, sizeof(tempMountPath), "/data/%s", randstr);
    rmdir(tempMountPath);
    mkdir(tempMountPath, 0777);

    if (mountTrophy(folder, tempMountPath) < 0) {
        snprintf(res, BUF_LEN, "Failed to mount: %s\n\n", strerror(errno));
        return;
    }
    if (copydir(tempMountPath, mountFolder) < 0) {
        snprintf(res, BUF_LEN, "Failed to copy dir\n\n");
    }
    umountTrophy(tempMountPath, 0, 0);
    rmdir(tempMountPath);
}

static void cmdUpdateTrophy(const char *folder, const char *sourceFolder, char res[BUF_LEN]) {
    char tempMountPath[MAX_PATH_LEN] = {0};
    char randstr[RANDSTR_LEN + 1] = {0};
    genRandomStr(randstr, RANDSTR_LEN);
    snprintf(tempMountPath, sizeof(tempMountPath), "/data/%s", randstr);
    rmdir(tempMountPath);
    mkdir(tempMountPath, 0777);

    if (mountTrophy(folder, tempMountPath) < 0) {
        snprintf(res, BUF_LEN, "Failed to mount: %s\n\n", strerror(errno));
        return;
    }
    if (copydir(sourceFolder, tempMountPath) < 0) {
        snprintf(res, BUF_LEN, "Failed to copy dir\n\n");
    }
    umountTrophy(tempMountPath, 0, 0);
    rmdir(tempMountPath);
}

static void commandHandler(char buf[BUF_LEN], char res[BUF_LEN]) {
    char *cmd = strtok(buf, " ");
    if (cmd == NULL) {
        snprintf(res, BUF_LEN, "Failed to parse cmd: %s\n\n", cmd);
        return;
    }
    removeTrailingWhitespace(cmd);

    if (strcmp(cmd, "dump_save") == 0) {
        char *saveFolder, *saveName, *mountFolder;

        saveFolder = strtok(NULL, " ");
        saveName = strtok(NULL, " ");
        mountFolder = strtok(NULL, " ");
        if (saveFolder == NULL || saveName == NULL | mountFolder == NULL) {
            snprintf(res, BUF_LEN, "dump_save <savefolder> <savename> <mountfolder>\n\n");
            return;
        }
        removeTrailingWhitespace(mountFolder);

        cmdDumpSave(saveFolder, saveName, mountFolder, res);
    }
    else if (strcmp(cmd, "update_save") == 0) {
        char *saveFolder, *saveName, *sourceFolder;

        saveFolder = strtok(NULL, " ");
        saveName = strtok(NULL, " ");
        sourceFolder = strtok(NULL, " ");
        if (saveFolder == NULL || saveName == NULL || sourceFolder == NULL) {
            snprintf(res, BUF_LEN, "update_save <savefolder> <savename> <sourcefolder>\n\n");
            return;
        }
        removeTrailingWhitespace(sourceFolder);

        cmdUpdateSave(saveFolder, saveName, sourceFolder, res);
    }
    else if (strcmp(cmd, "create_save") == 0) {
        char *targetFolder, *saveName, *blocks;
        int saveblocks;

        targetFolder = strtok(NULL, " ");
        saveName = strtok(NULL, " ");
        blocks = strtok(NULL, " ");
        if (targetFolder == NULL || saveName == NULL || blocks == NULL) {
            snprintf(res, BUF_LEN, "create_save <targetfolder> <savename> <blocks>\n\n");
            return;
        }
        removeTrailingWhitespace(blocks);
        if (strToInt(blocks, 10, &saveblocks) < 0) {
            snprintf(res, BUF_LEN, "Parse error: %s\n\n", blocks);
            return;
        }

        if (createSave(targetFolder, saveName, saveblocks) < 0) {
            snprintf(res, BUF_LEN, "Failed to create: %s\n\n", strerror(errno));
        }
    }
    else if (strcmp(cmd, "dump_trophy") == 0) {
        char *folder, *mountFolder;

        folder = strtok(NULL, " ");
        mountFolder = strtok(NULL, " ");
        if (folder == NULL || mountFolder == NULL) {
            snprintf(res, BUF_LEN, "dump_trophy <folder> <mountfolder>\n\n");
            return;
        }
        removeTrailingWhitespace(mountFolder);

        cmdDumpTrophy(folder, mountFolder, res);
    }
    else if (strcmp(cmd, "update_trophy") == 0) {
        char *folder, *sourceFolder;

        folder = strtok(NULL, " ");
        sourceFolder = strtok(NULL, " ");
        if (folder == NULL || sourceFolder == NULL) {
            snprintf(res, BUF_LEN, "update_trophy <folder> <sourcefolder>\n\n");
            return;
        }
        removeTrailingWhitespace(sourceFolder);

        cmdUpdateTrophy(folder, sourceFolder, res);
    }
    else if (strcmp(cmd, "create_trophy") == 0) {
        char *targetFolder, *blocks;
        int trophyblocks;

        targetFolder = strtok(NULL, " ");
        blocks = strtok(NULL, " ");
        if (targetFolder == NULL || blocks == NULL) {
            snprintf(res, BUF_LEN, "create_save <targetfolder> <savename> <blocks>\n\n");
            return;
        }
        removeTrailingWhitespace(blocks);
        if (strToInt(blocks, 10, &trophyblocks) < 0) {
            snprintf(res, BUF_LEN, "Parse error: %s\n\n", blocks);
            return;
        }

        if (createTrophy(targetFolder, trophyblocks) < 0) {
            snprintf(res, BUF_LEN, "Failed to create: %s\n\n", strerror(errno));
        }
    }
    else if (strcmp(cmd, "keyset") == 0) {
        uint16_t keyset = getMaxKeySet();
        snprintf(res, BUF_LEN, "Keyset: %" PRIu16 "\n\n", keyset);
    }
    else {
        snprintf(res, BUF_LEN, "Unknown cmd: %s\n\n", cmd);
    }
}

int main(void) {
    if (initAll() != 0) {
        LOG("Failed to init");
        for(;;);
    }
    srand(time(NULL));

    int sockfd;
    int connfd;
    socklen_t addrLen;
    
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    fd_set readfds;
    int maxsd;
    int sd;
    int clientSocket[MAX_CONNECTIONS] = {0};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG("Failed to create socket");
        for(;;);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
        LOG("Failed to bind socket");
        for(;;);
    }

    addrLen = sizeof(clientAddr);

    if (listen(sockfd, MAX_CONNECTIONS) != 0) {
        LOG("Failed to listen");
        for(;;);
    }

    for (;;) {
        char buf[BUF_LEN] = {0};
        char res[BUF_LEN] = {0};
        int bytesRead;
        int i;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        maxsd = sockfd;

        for (i = 0; i < MAX_CONNECTIONS; i++) {
            sd = clientSocket[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > maxsd) {
                maxsd = sd;
            }
        }

        if (select(maxsd + 1, &readfds, NULL, NULL, NULL) < 0) {
            LOG("Select error");
            for(;;);
        }

        if (FD_ISSET(sockfd, &readfds)) {
            connfd = accept(sockfd, (struct sockaddr *)&clientAddr, &addrLen);
            if (connfd < 0) {
                LOG("Failed to accept client");
                continue;
            }

            for (i = 0; i < MAX_CONNECTIONS; i++) {
                if (clientSocket[i] == 0) {
                    clientSocket[i] = connfd;
                    break;
                }
            }

            if (i == MAX_CONNECTIONS) {
                close(connfd);
            }
        }

        for (i = 0; i < MAX_CONNECTIONS; i++) {
            sd = clientSocket[i];
            if (FD_ISSET(sd, &readfds)) {
                bytesRead = read(sd, buf, sizeof(buf));
                if (bytesRead <= 0) {
                    close(sd);
                    clientSocket[i] = 0;
                    continue;
                }
                
                buf[bytesRead] = '\0';
                res[bytesRead] = '\0';
                res[0] = '\n';
                res[1] = '\n';
                commandHandler(buf, res);
                write(sd, res, sizeof(res));
            }
        }
    }

    return 0;
}