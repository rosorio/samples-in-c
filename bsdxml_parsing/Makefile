EXEC=parse_svn_history
SRC= parse_svn_history.c
OBJ= $(SRC:.c=.o)
CFLAGS!= pkg-config --cflags expat
LDFLAGS!= pkg-config --libs expat


all: $(EXEC)

parse_svn_history:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(EXEC)

clean:
	@rm -f $(OBJ) $(EXEC) *.core
