CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -Iinclude

SRC=src/lexer.c src/lexer_test.c
TARGET=shellforge

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -lreadline -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
