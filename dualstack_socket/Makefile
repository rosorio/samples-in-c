EXEC=dualstack
SRC= dualstack.c
OBJ= $(SRC:.c=.o)

all: $(EXEC)

parse_svn_history:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(EXEC)

clean:
	@rm -f $(OBJ) $(EXEC) *.core
