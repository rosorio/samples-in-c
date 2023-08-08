EXEC= introspect_dbus_service
SRC= introspect_dbus_service.c
OBJ= $(SRC:.c=.o)
CFLAGS!= pkg-config --cflags gio-2.0
LDFLAGS!= pkg-config --libs gio-2.0


all: $(EXEC)

list_available_dbus_services:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(EXEC)

clean:
	@rm -f $(OBJ) $(EXEC) *.core
