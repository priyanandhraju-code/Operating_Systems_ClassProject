CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -Iinclude

SRC=src/main.c \
    src/history.c \
    src/expansion.c \
    src/lexer.c \
    src/parser.c \
    src/builtin.c \
    src/executor.c

TARGET=shellforge

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -lreadline -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
