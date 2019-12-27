
BIN = xor-decode
CFLAGS = -Wall -W -pedantic 
LDFLABS =

BIN_SRC = xor-decode.c

all: $(BIN)

$(BIN): $(BIN_SRC)
	@$(CC) -o $@ $^

clean:
	@rm -f $(BIN)
	@rm -rf *.o
