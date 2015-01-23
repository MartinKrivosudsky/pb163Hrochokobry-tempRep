#ifndef CRYPTO_HPP
#define CRYPTO_HPP

struct prepare_table
{
    unsigned char key[128];
    unsigned long int counter;  // should be initialize to zero
    unsigned char* p_table;
    int table_length;
};


/**
 * @brief int_to_array convert long int number to unsigned char array[32]
 * @param num convert this number
 * @param[out] array to this array
 */

void int_to_array(unsigned long int num, unsigned char array[32]);

/**
 * \brief   function to xor two *char of length
 *
 * \param output[out]               here is storred text after xoring
 * \param xor_table                 generated table to xor
 * \param input                     input to xor
 * \paramlength              length of char*'s to xor
 *
 */
void xor_table(unsigned char* output,
                        unsigned char* xor_table,
                        unsigned char* input,
                        unsigned int length
              );

/**
 * \brief   function to compute table to xor,
 *          require table_length % 16 == (use padding)
 *          key and nonce must be same in both side!
 *
 *
 * \param prepare_table             struct where all values/pointers below are stored:
 * \key                       key - symetric key used by both part
 * \counter[out]              counter, be carefull, couter is changed by computation
 * \output[out]               table to xor
 * \table_length              required length of generated tabl,
 *                                  !!! table_length % 16 == 0 !!!
 *
 */
void ecb_prepare_table(prepare_table *table);

void ctr_prepare_table(
                        unsigned char key[128],
                        unsigned char nonce_counter[16],
                        unsigned char** output,
                        int table_length
                        );


#endif // CRYPTO_HPP
