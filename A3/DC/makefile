CC = gcc
CFLAGS = -Wall  -I../common
SRC = src/DateCreator.c
OBJ = obj/DateCreator.o
BIN = bin/DateCreator

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): $(SRC) ../common/msg_queue.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf bin/* obj/*

.PHONY: all clean