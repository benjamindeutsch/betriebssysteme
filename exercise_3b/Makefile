#HTTP Server
#@author Benjamin Deutsch (12215881)
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: server

server: server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LINKER_FLAGS)
	
server.o: server.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f server.o server
