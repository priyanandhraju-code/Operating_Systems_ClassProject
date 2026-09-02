CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -Iinclude

SRC=src/expansion.c src/lexer.c src/parser.c src/integration_test.c

TARGET=shellforge

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
