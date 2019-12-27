EXEC=wordcount
SRC= wordcount.c
OBJ= $(SRC:.c=.o)

all: $(EXEC) test

wc: 
	$(CC) $(SRC) -o $(EXEC) 

clean:
	@rm -f $(OBJ) $(EXEC) *.core

test: $(EXEC)
	./$(EXEC) test/reduce.dic test/candide.txt
