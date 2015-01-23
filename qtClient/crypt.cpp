#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <time.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "crypt.h"


using namespace std;

char our_key[] = "aes_generate_key";
const int key_length = 32;
const int iv_length = 16;
const int block_length = 16;
const int hash_output_length = 32;

// treba
void error_message(const char message[], int exit_code)
{
    fputs ((char*) message, stderr);
    exit (exit_code);
}

// treba
void generate_key(const char* pers, unsigned char* key)
{
    // generate a key, fill value 256 bits key in char* key
    // second parameter is used to return key compute from function
    ctr_drbg_context ctr_drbg;
    entropy_context entropy;

    int ret;

    entropy_init( &entropy );
    if( ( ret = ctr_drbg_init( &ctr_drbg, entropy_func, &entropy, (unsigned char *) pers, strlen( pers ) ) ) != 0 )
    {
        printf( " failed\n ! ctr_drbg_init\n");
    }

    if( ( ret = ctr_drbg_random( &ctr_drbg, key, key_length ) ) != 0 )
    {
        printf( " failed\n ! ctr_drbg_random\n");
    }
}

// treba
void generate_random_iv(unsigned char* iv)
{
    srand(time(NULL));
    for (int i = 0; i < iv_length; i++) {
        iv[i] = rand() % 256;
    }
}

// treba
void padding (unsigned char** pp_buffer, int length)
{
    if(0 == (length % block_length))
    {
        return;
    }

    // allocating new area for padding:
    int to_pad = block_length - (length % block_length);
    unsigned char* p_buffer_new = (unsigned char *) realloc((*pp_buffer), (sizeof(unsigned char) * (to_pad + length)));
    if(NULL == p_buffer_new)
    {   // in case of error:
        free((*pp_buffer));
        *pp_buffer = NULL;
        error_message("realocating memory error", 1);
    }
    *pp_buffer = p_buffer_new;

    // pad new area with '0'
    memset((*pp_buffer + length), 48, to_pad);
}

// treba
void encryption (unsigned char* buffer, unsigned int size, unsigned char iv[iv_length],
                 unsigned char key[key_length], unsigned char** out_cipher_text)
{	// last parameter expected NULL pointer
    // it is outputparameter for encrypted file
    aes_context aes_enc;
    if (0 != (size % block_length))
    {
        padding(&buffer, size);
        size = size + (block_length - (size % block_length));
    }

    unsigned char* cipher_text = NULL;
    cipher_text = (unsigned char*) malloc (sizeof(unsigned char) * size);
    if (cipher_text != NULL){
        aes_setkey_enc( &aes_enc, key, 256);
        aes_crypt_cbc( &aes_enc, AES_ENCRYPT, size, iv, buffer, cipher_text );
    } else{
        error_message("Allocating memory error", 4);
    }
    (*out_cipher_text) = cipher_text;
}

// treba
void decryption (unsigned char* buffer, unsigned int size, unsigned char iv[iv_length],
                 unsigned char key[key_length], unsigned char** out_plain_text)
{   // last parameter expected NULL pointer
    // it is outputparameter for encrypted file
    aes_context aes_dec;
    if (0 != (size % block_length))
    {
        size = size + (block_length - (size % block_length));
    }

    // allocate area for plain text
    unsigned char* plain_text = NULL;
    plain_text = (unsigned char*) malloc (sizeof(unsigned char) * size);

    if (plain_text != NULL){
        aes_setkey_dec( &aes_dec, key, 256);
        aes_crypt_cbc( &aes_dec, AES_DECRYPT, size, iv, buffer, plain_text );
    }
    (*out_plain_text) = plain_text;
}

// treba
void get_hash (unsigned char* buffer, int size, unsigned char** out_our_hash)
{      // last parameter expected NULL pointer
       // it is outputparameter hash of first parameter
       // ALERT - expect hashing sizeof(buffer) % 16 == 0
    if (0 != (size % block_length))
    {
        size = size + (block_length - (size % block_length));
    }

    unsigned char* our_hash = NULL;
    our_hash = (unsigned char*) malloc (sizeof(unsigned char)* hash_output_length);
    sha256(buffer, size, our_hash, 0);
    (*out_our_hash) = our_hash;
}

// treba
bool verify_hash (unsigned char* buffer, int size, unsigned char* hash_to_compare)
{
    bool b_ret = false;
    if (0 != (size % block_length))
    {
        size = size + (block_length - (size % block_length));
    }
    unsigned char* our_hash = NULL;
    our_hash = (unsigned char*) malloc (sizeof(unsigned char)* hash_output_length);
    sha256(buffer, size, our_hash, 0);
    if (0 == memcmp(hash_to_compare, our_hash, hash_output_length))
    {
        b_ret = true;
    }
    return b_ret;
}

