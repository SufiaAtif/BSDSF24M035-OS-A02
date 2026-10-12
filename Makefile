CC = gcc
CFLAGS = -Wall -Wextra -g

all: bin/ls

obj/ls-v1.0.0.o: src/ls-v1.0.0.c
	$(CC) $(CFLAGS) -c src/ls-v1.0.0.c -o obj/ls-v1.0.0.o

bin/ls: obj/ls-v1.0.0.o
	$(CC) obj/ls-v1.0.0.o -o bin/ls

clean:
	rm -f obj/ls-v1.0.0.o bin/ls

.PHONY: all clean
