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
 */

#include <assert.h>
#include <stdio.h> /* perror */

#include "libkeccak/src/libkeccak.h"

#include "ohno-hash.h"

int to_hash(chunk chk_raw, chunk chk_hash) {
    libkeccak_spec_t spec;
    /* Essentially libkeccak_spec_sha3(&spec, 512); */
    /* However, different output length */
    spec.bitrate = 1600 - 2 * 512;
    spec.capacity = 2 * 512;
    spec.output = chk_hash.len * 8;
    /* Make sure it didn't overflow */
    assert(chk_hash.len == (size_t)(spec.output / 8));

    libkeccak_state_t state;
    if (libkeccak_state_initialise(&state, &spec)) {
        perror("libkeccak_state_initialise");
        return 0;
    }
    if (libkeccak_digest(&state, chk_raw.data, chk_raw.len, 0, "", chk_hash.data)) {
        perror("libkeccak_digest");
        return 0;
    }
    return 1;
}
