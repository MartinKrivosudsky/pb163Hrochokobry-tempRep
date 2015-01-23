#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include ".\polarssl\aes.h"
#include ".\polarssl\entropy.h"
#include ".\polarssl\ctr_drbg.h"
#include ".\polarssl\sha256.h"
#include ".\polarssl\sha512.h"
#include ".\polarssl\config.h"
#include ".\polarssl\check_config.h"

void error_message(const char message[], int exit_code);

void generate_key(const char* pers, unsigned char* key);

void generate_random_iv(unsigned char* iv);

void padding (unsigned char** pp_buffer, int length);

void encryption (unsigned char* buffer, unsigned int size, unsigned char iv[],
                 unsigned char key[], unsigned char** out_cipher_text);

void decryption (unsigned char* buffer, unsigned int size, unsigned char iv[],
                 unsigned char key[], unsigned char** out_plain_text);

void get_hash (unsigned char* buffer, int size, unsigned char** out_our_hash);

bool verify_hash (unsigned char* buffer, int size, unsigned char* hash_to_compare);

#endif // CRYPTO_HPP
