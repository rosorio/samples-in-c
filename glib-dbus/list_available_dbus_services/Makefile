EXEC= list_available_dbus_services
SRC= list_available_dbus_services.c
OBJ= $(SRC:.c=.o)
CFLAGS!= pkg-config --cflags gio-2.0
LDFLAGS!= pkg-config --libs gio-2.0


all: $(EXEC)

list_available_dbus_services:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(EXEC)

clean:
	@rm -f $(OBJ) $(EXEC) *.core
