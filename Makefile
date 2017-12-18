CC=gcc
CFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter -O0 -g -std=gnu99
PREFIX="/usr/local"

notarget: shell

shell: tsh

install: shell
	mkdir -p $(PREFIX)/bin
	install -s tsh $(PREFIX)/bin
