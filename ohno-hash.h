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

#include <stdlib.h> /* size_t */

typedef struct chunk {
    size_t len;
    char* data;
} chunk;

int to_hash(chunk chk_raw, chunk chk_hash);
