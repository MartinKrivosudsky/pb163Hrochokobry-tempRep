#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"
#include <crypto.hpp>
#include <time.h>

TEST_CASE("TEST CASE - Simple xor")
{
    unsigned char table1[9] = "00000000";
    unsigned char table2[9] = "11111111";
    unsigned char result1[9];
    xor_table(result1, table1, table2, 8);

    unsigned char table3[9] = "00000000";
    unsigned char table4[9] = "11111111";
    unsigned char result2[9];
    xor_table(result2, table3, table4, 8);

    REQUIRE(0 == memcmp(result1, result2, 8));
}

TEST_CASE("TEST CASE - Long Xor")
{
    const unsigned int length = 40560;
    unsigned char * table1 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table2 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table3 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table4 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table5 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table6 = (unsigned char *) malloc(length * sizeof(unsigned char));

    srand(time(NULL));
    for (unsigned int i = 0; i < length; i++)
    {
        (*table1) = (unsigned char) rand() % 256;
        (*table2) = (unsigned char) rand() % 256;
    }

    memcpy(table3, table1, length);
    memcpy(table4, table2, length);

    xor_table(table5, table1, table2, length);
    xor_table(table6, table3, table4, length);

    REQUIRE(0 == memcmp(table5, table6, length));
    free(table1);
    free(table2);
    free(table3);
    free(table4);
    free(table5);
    free(table6);
}

TEST_CASE("TEST CASE - Xor without changes")
{
    const unsigned int length = 1024;
    unsigned char * table1 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table2 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table3 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table4 = (unsigned char *) malloc(length * sizeof(unsigned char));

    srand(time(NULL));
    for (unsigned int i = 0; i < length; i++)
    {
        (*table1) = (unsigned char) rand() % 256;
        (*table2) = (unsigned char) rand() % 256;
    }

    memcpy(table4, table2, length);

    xor_table(table3, table1, table2, length);

    REQUIRE_FALSE(0 == memcmp(table3, table4, length));
    free(table1);
    free(table2);
    free(table3);
    free(table4);
}

TEST_CASE("TEST CASE - Xor shared parameters")
{
    const unsigned int length = 54321;
    unsigned char * table1 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table2 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table3 = (unsigned char *) malloc(length * sizeof(unsigned char));
    unsigned char * table4 = (unsigned char *) malloc(length * sizeof(unsigned char));

    srand(time(NULL));
    for (unsigned int i = 0; i < length; i++)
    {
        (*table1) = (unsigned char) rand() % 256;
        (*table2) = (unsigned char) rand() % 256;
    }

    memcpy(table3, table1, length);
    memcpy(table4, table2, length);

    xor_table(table1, table1, table2, length);
    xor_table(table3, table4, table3, length);

    REQUIRE(0 == memcmp(table1, table3, length));
    free(table1);
    free(table2);
    free(table3);
    free(table4);
}

TEST_CASE("TEST CASE - prepare_table() test equal table")
{
    const int length_size = 20480;
    unsigned char key[32] = "key_private_hahaha";

    prepare_table* table1 = (prepare_table *) malloc(sizeof(prepare_table));
    prepare_table* table2 = (prepare_table *) malloc(sizeof(prepare_table));

    table1->p_table = NULL;
    table1->table_length = length_size;
    table1->counter = 0;

    memcpy(table1->key, (const char*) key, 32);

    table2->p_table = NULL;
    table2->table_length = length_size;
    table2->counter = 0;
    memcpy(table2->key, (const char*) key, 32);

    ecb_prepare_table(table1);
    ecb_prepare_table(table2);

    REQUIRE(0 == memcmp(table1->p_table, table2->p_table, length_size));
    free(table1->p_table);
    free(table2->p_table);
    free(table1);
    free(table2);
}

TEST_CASE("TEST CASE - basic enc. and dec.")
{
    const int length_size = 64;
    unsigned char key[32] = "key_private";

    prepare_table* table1 = (prepare_table *) malloc(sizeof(prepare_table));
    prepare_table* table2 = (prepare_table *) malloc(sizeof(prepare_table));

    table1->p_table = NULL;
    table1->table_length = length_size;
    memcpy((char*) table1->key, (const char*) key, 32);

    table1->counter = 0;
    memcpy(table2, table1, sizeof(prepare_table));

    unsigned char input[length_size] = "Hello  12345678901234567890";
    unsigned char output[length_size];
    unsigned char plaintext[length_size];

    // compute fist table
    ecb_prepare_table(table1);

    // XOR plain text with "cipher" table, result = cipher text
    xor_table(output, input, table1->p_table, length_size);

    // compute second table
    ecb_prepare_table(table2);

    xor_table(plaintext, output, table2->p_table, length_size);

    REQUIRE(0 == memcmp((const char *) plaintext, (const char *) input, length_size));
    free(table1->p_table);
    free(table2->p_table);
    free(table1);
    free(table2);
}

TEST_CASE("TEST CASE - long text enc. and dec.")
{
    const int length_size = 20480;
    unsigned char key[32] = "key_private";

    prepare_table* table1 = (prepare_table *) malloc(sizeof(prepare_table));
    prepare_table* table2 = (prepare_table *) malloc(sizeof(prepare_table));

    table1->p_table = NULL;
    table1->table_length = length_size;
    memcpy((char*) table1->key, (const char*) key, 32);
    table1->counter = 0;

    memcpy(table2, table1, sizeof(prepare_table));

    unsigned char* input = (unsigned char *) malloc(length_size * sizeof(unsigned char));
    unsigned char* output = (unsigned char *) malloc(length_size * sizeof(unsigned char));
    unsigned char* plaintext = (unsigned char *) malloc(length_size * sizeof(unsigned char));
    // generete randoms for plain text
    for (int i = 0; i < length_size; i++)
    {
        input[i] = (unsigned char) rand() % 256;
    }

    // compute fist table
    ecb_prepare_table(table1);

    // XOR plain text with "cipher" table, result = cipher text
    xor_table(output, input, table1->p_table, length_size);

    // compute second table
    ecb_prepare_table(table2);

    xor_table(plaintext, output, table2->p_table, length_size);

    REQUIRE(0 == memcmp((const char *) plaintext, (const char *) input, length_size));
    free(table1->p_table);
    free(table2->p_table);
    free(table1);
    free(table2);
    free(input);
    free(output);
    free(plaintext);
}

TEST_CASE("TEST CASE - small divided table computing")
{
    const int length_size = 64;
    unsigned char key[32] = "key_private_hahaha";

    prepare_table* table1 = (prepare_table *) malloc(sizeof(prepare_table));

    table1->p_table = NULL;
    table1->table_length = length_size;
    table1->counter = 0;
    memcpy(table1->key, (const char*) key, 32);

    prepare_table* table2 = (prepare_table *) malloc(sizeof(prepare_table));
    prepare_table* table3 = (prepare_table *) malloc(sizeof(prepare_table));

    table2->p_table = NULL;
    table2->table_length = length_size / 2;
    table2->counter = 0;
    memcpy(table2->key, (const char*) key, 32);

    table3->p_table = NULL;
    table3->table_length = length_size / 2;
    table3->counter =  (int) ((length_size / 16) / 2);
    memcpy(table3->key, (const char*) key, 32);

    ecb_prepare_table(table1);
    ecb_prepare_table(table2);
    ecb_prepare_table(table3);

    REQUIRE(0 == memcmp(table1->p_table, table2->p_table, 32));
    REQUIRE(0 == memcmp(table1->p_table+32, table3->p_table, 32));
    free(table1->p_table);
    free(table2->p_table);
    free(table3->p_table);
    free(table1);
    free(table2);
    free(table3);
}

TEST_CASE("TEST CASE - divided table computing")
{
    const int length_size = 20480;
    unsigned char key[32] = "key_private_hahaha";

    // big table
    prepare_table* table1 = (prepare_table *) malloc(sizeof(prepare_table));

    // compute first big table
    table1->p_table = NULL;
    table1->table_length = length_size;
    table1->counter = 0;
    memcpy(table1->key, (const char*) key, 32);
    ecb_prepare_table(table1);

    // four smaller table, compute same value as big
    prepare_table* table2 = (prepare_table *) malloc(sizeof(prepare_table));
    prepare_table* table3 = (prepare_table *) malloc(sizeof(prepare_table));
    prepare_table* table4 = (prepare_table *) malloc(sizeof(prepare_table));
    prepare_table* table5 = (prepare_table *) malloc(sizeof(prepare_table));

    // compute first quater of shared table
    table2->p_table = NULL;
    table2->table_length = length_size / 4;
    table2->counter = 0;
    memcpy(table2->key, (const char*) key, 32);
    ecb_prepare_table(table2);

    // second quater...
    table3->p_table = NULL;
    table3->table_length = length_size / 4;
    table3->counter =  (int) ((length_size / 16) / 4);
    memcpy(table3->key, (const char*) key, 32);
    ecb_prepare_table(table3);

    // third quater...
    table4->p_table = NULL;
    table4->table_length = length_size / 4;
    table4->counter =  (int) ((length_size / 16) / 2);
    memcpy(table4->key, (const char*) key, 32);
    ecb_prepare_table(table4);

    // last quater
    table5->p_table = NULL;
    table5->table_length = length_size / 4;
    table5->counter =  (int) (((length_size / 16) / 4) * 3);
    memcpy(table5->key, (const char*) key, 32);
    ecb_prepare_table(table5);

    // compare per partes :)
    int next = (int) (length_size  / 4);
    REQUIRE(0 == memcmp(table1->p_table, table2->p_table, next));
    REQUIRE(0 == memcmp(table1->p_table + next, table3->p_table, next));
    REQUIRE(0 == memcmp(table1->p_table + (next * 2), table4->p_table, next));
    REQUIRE(0 == memcmp(table1->p_table + (next * 3), table5->p_table, next));

    // join tables
    unsigned char* joined_table = (unsigned char*) malloc(length_size * sizeof(unsigned char));
    int shift = 0;
    memcpy(joined_table + shift, table2->p_table, next);
    shift += next;
    memcpy(joined_table + shift, table3->p_table, next);
    shift += next;
    memcpy(joined_table + shift, table4->p_table, next);
    shift += next;
    memcpy(joined_table + shift, table5->p_table, next);

    // compare joined tables with original big table
    REQUIRE(0 == memcmp(table1->p_table, joined_table, length_size));

    free(table1->p_table);
    free(table2->p_table);
    free(table3->p_table);
    free(table4->p_table);
    free(table5->p_table);
    free(table1);
    free(table2);
    free(table3);
    free(table4);
    free(table5);
}
