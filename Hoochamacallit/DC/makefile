CC = gcc
CFLAGS = -Wall -g -I$(CURDIR)/inc -I$(abspath ../Common/inc)
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
COMMON_OBJ = $(wildcard ../Common/obj/*.o)
TARGET = bin/DC

all: $(TARGET)

$(TARGET): $(OBJ) $(COMMON_OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(OBJ) $(COMMON_OBJ) -o $(TARGET)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* bin/*

