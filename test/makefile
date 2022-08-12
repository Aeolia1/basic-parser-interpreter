CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -std=c99 -Wvla -Wfloat-equal -O2 -Werror
SANI= -fsanitize=undefined -fsanitize=address

LDLIBS= -lm

parse : parse.c
	$(CC) parse.c -o parse $(CFLAGS) $(SANI) $(LDLIBS)

interp : interp.c
	$(CC) interp.c -o interp $(CFLAGS) $(SANI) $(LDLIBS)

extension : extension.c
	$(CC) extension.c -o extension $(CFLAGS) $(SANI) $(LDLIBS)

	