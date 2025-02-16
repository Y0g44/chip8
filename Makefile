CC := gcc
CFLAGS := -O3 -Wall -fPIC

all: src/ch8u src/chip8.so

clean:
	rm -f src/ch8u src/chip8.so src/ch8u.o src/chip8.o

src/ch8u: src/ch8u.o src/chip8.o
src/chip8.so: src/chip8.o
	gcc $< -shared -o $@

.phony: all clean

src/ch8u.o: src/ch8u.c
src/chip8.o: src/chip8.c
