#ifndef SEALEDKEY_H
#define SEALEDKEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ENC_SEALEDKEY_LEN 0x60
#define DEC_SEALEDKEY_LEN 0x20

int generateSealedKey(uint8_t data[ENC_SEALEDKEY_LEN]);
int decryptSealedKey(uint8_t enc_key[ENC_SEALEDKEY_LEN], uint8_t dec_key[DEC_SEALEDKEY_LEN]);
int decryptSealedKeyAtPath(const char *keyPath, uint8_t decryptedSealedKey[DEC_SEALEDKEY_LEN]);
uint16_t getMaxKeySet(void);

#ifdef __cplusplus
}
#endif

#endif // SEALEDKEY_H