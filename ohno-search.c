/*
 ohno - Interpreter for "ohno", which does not stand for
        "overly hard, nonsensical language"
 Copyright 2016 Ben Wiederhake

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdio.h>

#include "ohno-hash.h"

#define OUTPUT_BYTES (8/2)
#define INPUT_EXTRA_BYTES 3

int main() {
    unsigned char input[4 + INPUT_EXTRA_BYTES] = {0};
    typedef char check_uint_size[(sizeof(unsigned int) == 4) ? 1 : -1];
    assert(sizeof(check_uint_size) > 0);
    input[0] = (unsigned char)((OUTPUT_BYTES >> (3 * 8)) & 0xFF);
    input[1] = (unsigned char)((OUTPUT_BYTES >> (2 * 8)) & 0xFF);
    input[2] = (unsigned char)((OUTPUT_BYTES >> (1 * 8)) & 0xFF);
    input[3] = (unsigned char)((OUTPUT_BYTES >> (0 * 8)) & 0xFF);

    unsigned char output[OUTPUT_BYTES];

    chunk chk_input;
    chk_input.data = (char*)input; /* unsigned / signed */
    chk_input.len = 4 + INPUT_EXTRA_BYTES;
    chunk chk_output;
    chk_output.data = (char*)output;
    chk_output.len = OUTPUT_BYTES;

    while (1) {
        /* Compute hash */
        if (!to_hash(chk_input, chk_output)) {
            return 1;
        }

        /* Print it */
        for (size_t i = 0; i < OUTPUT_BYTES; ++i) {
            printf("%02x", output[i]);
        }
        printf(" ");
        for (size_t i = 0; i < 4 + INPUT_EXTRA_BYTES; ++i) {
            printf("%02x", input[i]);
        }
        printf("\n");

        /* Next */
        for (size_t p = 4 + INPUT_EXTRA_BYTES - 1; p >= 4; --p) {
            if (++input[p] != 0) {
                break;
            }
            if (4 == p) {
                return 0;
            }
        }
    }
}
