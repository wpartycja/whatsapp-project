BIN_FILES  = cliente servidor libclaves.so

CC = gcc

CPPFLAGS = -I$(INSTALL_PATH)/include -Wall

LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lpthread -lm


all: $(BIN_FILES)
.PHONY : all


servidor: servidor.o servicios.o lines.o servicios_help.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ -lrt

cliente: cliente.o libclaves.so
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@  $(LDLIBS)

claves.o: claves.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -fPIC
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared $< -o libclaves.so -fPIC

libclaves.so: lines.o claves.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared $^ -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	rm -f $(BIN_FILES) *.o *.so

.SUFFIXES:
.PHONY : clean