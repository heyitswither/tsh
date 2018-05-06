CC=gcc
CFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter -O0 -g -std=gnu99
PREFIX="/usr/local"

notarget: tsh

tsh: libtsh.o builtins.o

install: tsh
	$(MKDIR) -p $(PREFIX)/bin
	$(INSTALL) -s tsh $(PREFIX)/bin

clean:
	$(RM) *.o tsh
