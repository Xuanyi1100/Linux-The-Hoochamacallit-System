CC = gcc
CFLAGS = -Wall -g -I$(CURDIR)/inc
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

all: $(OBJ)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/*

