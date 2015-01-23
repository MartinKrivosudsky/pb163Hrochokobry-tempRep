#include <iostream>
#include <../crypto_crt/crypto.hpp>

#include <polarSSL/aes.h>
#include <polarSSL/aesni.h>
#include <polarSSL/check_config.h>
#include <polarSSL/config.h>
#include <polarSSL/padlock.h>
#include <polarSSL/platform.h>
#include "polarssl/ctr_drbg.h"
#include <polarSSL/entropy.h>

#include <polarSSL/catch.hpp>

using namespace std;

void upgrade_counter(prepare_table* table, unsigned int to_upgrade)
{
    table->counter += to_upgrade;
}

void int_to_array(unsigned long int num, unsigned char array[128])
{
    for (int i = 0; i < 128; i++)
    {
        array[i] = (char) num % 256;
        num = num / 256;
    }
}

void xor_table(
                        unsigned char* output,
                        unsigned char* xor_table,
                        unsigned char* input,
                        unsigned int length
              )
{
    for (unsigned int i = 0; i < length; i++){
        *(output+i) = *(input+i) ^ *(xor_table+i);
    }
}

void ecb_prepare_table(prepare_table *table)
{
    unsigned char* input = NULL;
    aes_context aes;

    input = (unsigned char*) malloc(table->table_length * sizeof(unsigned char));
    if (NULL == input)
    {   // ERROR
        printf("ERROR allocations ctr_prepare_table()");
    }
    // set all values in table to '0'
    memset(input, 48, table->table_length);

    table->p_table = (unsigned char*) malloc(table->table_length * sizeof(unsigned char));
    if (NULL == table->p_table)
    {   // ERROR
        printf("ERROR allocations ctr_prepare_table()");
    }

    unsigned char temp_counter[128];
    unsigned char temp_key[128];
    int_to_array(table->counter, temp_counter);
    xor_table(temp_key, table->key, temp_counter, 128);
    aes_setkey_enc(&aes, temp_key, 128);

    int next = 0;
    for (int i = 0; ( (int) (table->table_length / 16)) > i; i++)
    {
        next = i * 16;
        if (0 != aes_crypt_ecb(&aes, AES_ENCRYPT, input + next, table->p_table + next))
        {   // ERROR
            printf("ERROR aes_crypt_ecb() in ctr_prepare_table()");
        }
        upgrade_counter(table, 1);
        int_to_array(table->counter, temp_counter);
        xor_table(temp_key, table->key, temp_counter, 128);
        aes_setkey_enc(&aes, temp_key, 128);
    }

}


