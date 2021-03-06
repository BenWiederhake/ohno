# ohno - Interpreter for "ohno", which does not stand for
#        "overly hard, nonsensical language"
# Copyright 2016 Ben Wiederhake
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

HASH_OBJECTS   := ohno-hash.o
OHNO_OBJECTS   := ohno.o
SEARCH_OBJECTS := ohno-search.o
ALL_OBJECTS    := ${HASH_OBJECTS} ${OHNO_OBJECTS} ${SEARCH_OBJECTS}

all = bin

.PHONY: bin
bin: ohno search

libkeccak/Makefile:
	@echo "You didn't use '--recursive' when cloning, did you?"
	@echo "Try 'git submodule update --init' to fix this."
# The great thing about 'false' is that it does it's job,
# even if it's not installed!  ;-)
	@false

libkeccak/bin/libkeccak.a: libkeccak/Makefile
	+make -C libkeccak

${ALL_OBJECTS}: %.o: %.c | libkeccak/Makefile
	gcc -Wall -Wextra -pedantic -Werror -g -O2 -std=c99 -c -o $@ $<

ohno: ${HASH_OBJECTS} ${OHNO_OBJECTS} libkeccak/bin/libkeccak.a
	gcc -Wall -Wextra -pedantic -Werror -g -O2 -std=c99 -o $@ $^

search: ${HASH_OBJECTS} ${SEARCH_OBJECTS} libkeccak/bin/libkeccak.a
	gcc -Wall -Wextra -pedantic -Werror -g -O2 -std=c99 -o $@ $^
