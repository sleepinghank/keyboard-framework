#ifndef AES128_H
#define AES128_H

#include <stdint.h>

// AES128 加密函数
void AES_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

// AES128 解密函数
void AES_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

#endif /* AES128_H */



