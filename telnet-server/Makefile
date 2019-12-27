
BIN = tcp-server
TEST = test

CFLAGS = -Wall -W -pedantic 
LDFLABS =

SRC = commands.c server.c
BIN_SRC = $(SRC) tcp-server.c
TEST_SRC = $(SRC) unit_test.c


all: test $(BIN)

$(BIN): $(BIN_SRC)
	@$(CC) -o $@ $^

$(TEST): $(TEST_SRC)
	$(CC) -o $@ $^
	./$(TEST)

clean:
	@rm -f $(TEST)
	@rm -f $(BIN)
	@rm -rf *.o
