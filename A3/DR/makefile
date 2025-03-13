CC = gcc
CFLAGS = -Wall  -I../common
SRC = src/DataReader.c
OBJ = obj/DataReader.o
BIN = bin/DataReader


all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): $(SRC) ../common/msg_queue.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf bin/* obj/*

.PHONY: all clean