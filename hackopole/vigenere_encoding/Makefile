EXEC=vigenere
SRC= vigenere.c
OBJ= $(SRC:.c=.o)

all: $(EXEC) test

wc:
	$(CC) $(SRC) -o $(EXEC) 

clean:
	@rm -f $(OBJ) $(EXEC) *.core

test: $(EXEC)
	echo "Gqfltwj" | ./$(EXEC) FCSC
