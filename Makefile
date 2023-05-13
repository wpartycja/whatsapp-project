BIN_FILES  =  server 

CC = gcc

CPPFLAGS = -I$(INSTALL_PATH)/include -Wall

LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lpthread -lm


all: $(BIN_FILES)
.PHONY : all


server: server.o servicios.o lines.o servicios_help.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ -lrt

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	rm -f $(BIN_FILES) *.o *.so

.SUFFIXES:
.PHONY : clean