
#include <stddef.h>
#include "reverse.h"

/*
 * ReverseBits - reverses the bit order across an entire byte array
 */
void ReverseBits(unsigned char *arr, int len_arr) {
    // Guard clause for NULL pointer or non-positive length
    if (arr == NULL || len_arr <= 0)
        return;

    int total_bits = len_arr * 8;

    // Loop through half of the bits so we move from left and right side ot the center
    for (int left_index = 0; left_index < total_bits / 2; ++left_index) {
        // Calculate the corresponding left index
        int left_byte = left_index / 8;
        int left_mask  = left_index % 8;

        // Calculate the corresponding right index
        int right_index = total_bits - 1 - left_index;
        int right_byte = right_index / 8;
        int right_mask  = right_index % 8;

        // Get the left and right bits
        unsigned char left_bit = (arr[left_byte] >> left_mask) & 1;
        unsigned char right_bit = (arr[right_byte] >> right_mask) & 1;

        // Swap them if they're different to avoid unnecessary moves
        if (left_bit != right_bit) {
            arr[left_byte] ^= (1 << left_mask);
            arr[right_byte] ^= (1 << right_mask);
        }
    }
}
