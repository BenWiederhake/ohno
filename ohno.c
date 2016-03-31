/*
 ohno - Interpreter for the impossibly difficult language "ohno"
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

 --

 Compile like this:
   make ohno
 Run like this:
   ./ohno <path/to/your/program>

 As it is impossibly hard to write a meaningful program in ohno,
 I'm not sure whether there are any bugs.

 For an explanation how this (should) work, see README.md

 This project started at https://github.com/BenWiederhake/ohno
 Feel free to leave me some feedback, or even a Pull Request! :D
 */

#include <assert.h>

#ifndef __USE_MISC
#define __USE_MISC
#endif
#include <endian.h>

#include <stdio.h>
#include <stdlib.h>

#include "ohno-hash.h"

const char* const USAGE =
"Usage:\n"
"  %s <path/to/your/program>\n"
"\n"
"Your program should be written ohno, that is:\n"
"- At least 4 bytes long\n"
"- The first 4 bytes are (in little endian) the length of the ohno bytecode\n"
"- The rest of the file may be anything\n"
"\n"
"The ohno bytecode is computed by computing the KECCAK-1600 of the entire input\n"
"file, and essentially interpreting that hash as brainfuck instructions.\n"
"\n"
"For more information about how to write ohno code, please read\n"
"the documentation.\n";

#ifndef OHNO_SIZE_T_MODIFIER
#define OHNO_SIZE_T_MODIFIER "ld"
#endif

chunk read_to_file(const char* filename) {
    chunk chk;
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("Couldn't open file");
        chk.len = 0;
        return chk;
    }
    if (fseek(fp, 0, SEEK_END)) {
        perror("Problem while reading (fseek)");
        fclose(fp);
        chk.len = 0;
        return chk;
    }
    /* This ignores the glaringly obvious race condition:
     * What if someone writes to 'fp' after this? */
    long int raw_len = ftell(fp);
    if (raw_len < 0) {
        perror("Problem while reading (ftell)");
        fclose(fp);
        chk.len = 0;
        return chk;
    }
    chk.len = (size_t)raw_len;
    if (chk.len < 4) {
        fprintf(stderr, "File is too short\n");
        fclose(fp);
        chk.len = 0;
        return chk;
    }
    if (fseek(fp, 0, SEEK_SET)) {
        perror("Problem while reading (fseek back)");
        fclose(fp);
        chk.len = 0;
        return chk;
    }

    chk.data = malloc(chk.len);
    if (!chk.data) {
        fprintf(stderr, "Bad malloc for size %" OHNO_SIZE_T_MODIFIER "?!\n", chk.len);
        fclose(fp);
        chk.len = 0;
        return chk;
    }

    if (1 != fread(chk.data, chk.len, 1, fp)) {
        fprintf(stderr, "Problem while reading %" OHNO_SIZE_T_MODIFIER " bytes from %s\n", chk.len,
                filename);
        fclose(fp);
        chk.len = 0;
        free(chk.data);
        chk.data = NULL;
        return chk;
    }

    fclose(fp);

    /* Whew! Finally. */
    return chk;
}

//typedef struct bf_chunk {
//    unsigned char content[4096 - 2 * sizeof(void*)];
//    bf_chunk* left;
//    bf_chunk* right;
//} bf_chunk;

//int run_bf(chunk* code, bf_chunk* tape) {
//
//}

int main(const int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, USAGE, argv[0]);
        return 1;
    }

    /* Read file */
    chunk content = read_to_file(argv[1]);
    if (content.len < 4) {
        return 1;
    }

    /* Compute hash */
    typedef char check_uint_size[(sizeof(unsigned int) >= 4) ? 1 : -1];
    assert(sizeof(check_uint_size) > 0);
    chunk hash;
    hash.len = le64toh(*(unsigned int* )content.data);
    fprintf(stderr, "That'll be %" OHNO_SIZE_T_MODIFIER " bytes of brainfuck.\n", hash.len);
    hash.data = malloc(hash.len);
    if (!hash.data) {
        fprintf(stderr, "Couldn't allocate that much.\n");
        free(content.data);
        return 1;
    }

    if (!to_hash(content, hash)) {
        fprintf(stderr, "Couldn't compute SHA3 ... huh?\n");
        free(content.data);
        free(hash.data);
        return 1;
    }

    fprintf(stderr, "So far, so good.\n");
    return 0;

//    bf_chunk tape;
//    memset(tape.content, 0, sizeof(tape.content));
//    tape.left = NULL;
//    tape.right = NULL;
//
//    return run_bf(hash, &tape);
}
