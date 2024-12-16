#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include "../ps4-libjbc/utils.h"
#include "defs.h"
#include "sealedkey.h"

int generateSealedKey(uint8_t data[ENC_SEALEDKEY_LEN]) {
    uint8_t dummy[0x30]; UNUSED(dummy);
    uint8_t sealedKey[ENC_SEALEDKEY_LEN] = {0};

    int fd = open("/dev/sbl_srv", O_RDWR);
    if (fd == -1) {
        return -1;
    }

    if (ioctl(fd, 0x40845303, sealedKey) == -1) {
        close(fd);
        return -2;
    }

    memcpy(data, sealedKey, sizeof(sealedKey));
    close(fd);

    return 0;
}

int decryptSealedKey(uint8_t enc_key[ENC_SEALEDKEY_LEN], uint8_t dec_key[DEC_SEALEDKEY_LEN]) {
    uint8_t dummy[0x10]; UNUSED(dummy);
    uint8_t data[ENC_SEALEDKEY_LEN + DEC_SEALEDKEY_LEN] = {0};

    int fd = open("/dev/sbl_srv", O_RDWR);
    if (fd == -1) {
        return -1;
    }

    memcpy(data, enc_key, ENC_SEALEDKEY_LEN);

    if (ioctl(fd, 0xC0845302, data) == -1) {
        close(fd);
        return -2;
    }

    memcpy(dec_key, &data[ENC_SEALEDKEY_LEN], DEC_SEALEDKEY_LEN);
    close(fd);

    return 0;
}

int decryptSealedKeyAtPath(const char *keyPath, uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN]) {
    uint8_t sealedKey[ENC_SEALEDKEY_LEN] = {0};

    int fd = open(keyPath, O_RDONLY);
    if (fd == -1) {
        return -1;
    }

    if (read(fd, sealedKey, ENC_SEALEDKEY_LEN) != ENC_SEALEDKEY_LEN) {
        close(fd);
        return -2;
    }
    close(fd);

    if (decryptSealedKey(sealedKey, decryptedSealedKey) != 0) {
        return -3;
    }

    return 0;
}

uint16_t maxKeyset = 0;
uint16_t getMaxKeySet(void) {
    if (maxKeyset > 0) {
        return maxKeyset;
    }

    uint8_t sampleSealedKey[ENC_SEALEDKEY_LEN] = {0};
    if (generateSealedKey(sampleSealedKey) != 0) {
        return 0;
    }

    maxKeyset = (sampleSealedKey[9] << 8) + sampleSealedKey[8];
    return maxKeyset;
}