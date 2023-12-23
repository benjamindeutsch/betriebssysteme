#HTTP Client
#@author Benjamin Deutsch (12215881)
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: client

client: client.o
	$(CC) $(CFLAGS) -o $@ $^ $(LINKER_FLAGS)
	
client.o: client.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f client.o client
