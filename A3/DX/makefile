CC = gcc
CFLAGS = -Wall  -I../common
SRC = src/DataCorruptor.c
OBJ = obj/DataCorruptor.o
BIN = bin/DataCorruptor


all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): $(SRC) ../common/msg_queue.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf bin/* obj/*

.PHONY: all clean