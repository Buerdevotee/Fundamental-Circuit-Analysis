CC=gcc
CFLAGS=-Wall -Wextra -g -O0 -Iinclude
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)
TARGET=circuit_analyser

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)
