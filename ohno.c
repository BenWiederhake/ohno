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

 --

 Compile like this:
   make ohno
 Run like this:
   ./ohno <path/to/your/program>

 As it is overly hard to write a meaningful program in ohno,
 I'm not sure whether there are any bugs.

 For an explanation how this (should) work, see README.md

 This project started at https://github.com/BenWiederhake/ohno
 Feel free to leave me some feedback, or even a Pull Request! :D
 */

#include <assert.h>

#include <stdio.h>
#include <string.h>

#include "ohno-hash.h"

const char* const USAGE =
"Usage:\n"
"  %s <path/to/your/program>\n"
"\n"
"Your program should be written ohno, that is:\n"
"- At least 4 bytes long\n"
"- The first 4 bytes are (in big endian) the length of the ohno bytecode\n"
"- The rest of the file may be anything\n"
"\n"
"The ohno bytecode is computed by computing a Keccak variant of the entire input\n"
"file, and essentially interpreting that hash as Brainfuck instructions.\n"
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

typedef struct bf_chunk {
    unsigned char content[4096 - 2 * sizeof(void*)];
    struct bf_chunk* left;
    struct bf_chunk* right;
} bf_chunk;

void init_bf_chunk(bf_chunk* tape) {
    assert(sizeof(tape->content) == 4080); /* Sanity check for my machine */
    memset(tape->content, 0, sizeof(tape->content));
    tape->left = NULL;
    tape->right = NULL;
}

unsigned char read_instr(chunk* code, int pc_nibble) {
    if (pc_nibble < 0) {
        double x = 5;
        /* Crash */
        while (1) {
            x = x*x - 20;
        }
    }
    assert(pc_nibble >= 0);
    assert((size_t)(pc_nibble / 2) < code->len);
    unsigned char instr = code->data[pc_nibble / 2];
    if (!(pc_nibble % 2)) {
        instr >>= 4;
    }
    return instr & 0x07;
}

int run_bf(chunk* code, bf_chunk* tape) {
    int tape_byte = sizeof(tape->content) / 2;
    int pc_nibble = 0;
    while ((size_t)(pc_nibble / 2) < code->len) {
        /* Read instruction */
        const unsigned char instr = read_instr(code, pc_nibble);
        /* Execute instruction */
        switch (instr) {
        case 0:
            /* '+' */
            ++tape->content[tape_byte];
            break;
        case 1:
            /* '-' */
            --tape->content[tape_byte];
            break;
        case 2:
            /* '{' */
            break;
        case 3:
            /* '}' */
            if (tape->content[tape_byte]) {
                /* Scan backwards to corresponding opening bracket */
                size_t need_opens = 1;
                do {
                    --pc_nibble;
                    switch (read_instr(code, pc_nibble)) {
                    case 2:
                        --need_opens;
                        break;
                    case 3:
                        ++need_opens;
                        break;
                    default:
                        /* Ignore */
                        break;
                    }
                } while (need_opens > 0);
            }
            break;
        case 4:
            /* '>' */
            ++tape_byte;
            if ((size_t)tape_byte >= sizeof(tape->content)) {
                assert(tape_byte == sizeof(tape->content));
                tape_byte -= sizeof(tape->content);
                if (!tape->right) {
                    tape->right = (bf_chunk*) malloc(sizeof(bf_chunk));
                    if (!tape->right) {
                        free(tape); /* Hopefully enough to construct the string */
                        fprintf(stderr, "Out of memory :(\n");
                        return 1;
                    }
                    init_bf_chunk(tape->right);
                    tape->right->left = tape;
                }
                tape = tape->right;
            }
            break;
        case 5:
            /* '<' */
            --tape_byte;
            if (tape_byte < 0) {
                assert(tape_byte == -1);
                tape_byte += sizeof(tape->content);
                if (!tape->left) {
                    tape->left = (bf_chunk*) malloc(sizeof(bf_chunk));
                    if (!tape->left) {
                        free(tape); /* Hopefully enough to construct the string */
                        fprintf(stderr, "Out of memory :(\n");
                        return 1;
                    }
                    init_bf_chunk(tape->left);
                    tape->left->right = tape;
                }
                tape = tape->left;
            }
            break;
        case 6:
            /* ','
             * Specifically, this implements the "no change" variant for highest
             * imagined portability. */
        {
            int it = getchar();
            if (it == EOF) {
                /* Whatever, call it the end of input, even if that actually
                 * was an error. */
                return 0;
            }
            tape->content[tape_byte] = (unsigned char)it;
            break;
        }
        case 7:
            /* '.' */
            putchar(tape->content[tape_byte]);
            break;
        }
        ++pc_nibble;
    }
    return 0;
}

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
    typedef char check_uint_size[(sizeof(unsigned int) == 4) ? 1 : -1];
    assert(sizeof(check_uint_size) > 0);
    chunk hash;
    hash.len = 0;
    hash.len |= ((unsigned char)content.data[0]) << (8 * 3);
    hash.len |= ((unsigned char)content.data[1]) << (8 * 2);
    hash.len |= ((unsigned char)content.data[2]) << (8 * 1);
    hash.len |= ((unsigned char)content.data[3]) << (8 * 0);
    fprintf(stderr, "That'll be %" OHNO_SIZE_T_MODIFIER " bytes of brainfuck.\n", hash.len);
    hash.data = malloc(hash.len);
    if (!hash.data) {
        fprintf(stderr, "Couldn't allocate that much.\n");
        return 1;
    }

    if (!to_hash(content, hash)) {
        fprintf(stderr, "Couldn't compute SHA3 ... huh?\n");
        return 1;
    }

    fprintf(stderr, "Bytecode is: ");
    for (size_t i = 0; i < hash.len; ++i) {
        fprintf(stderr, "%02x", (unsigned char)hash.data[i]);
    }
    fprintf(stderr, "\nStart\n");

    bf_chunk tape;
    init_bf_chunk(&tape);

    return run_bf(&hash, &tape);
}
