#Feedback Arc Set
#@author Benjamin Deutsch (12215881)
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
SRC_FILES = adjacency_list.c generator.c shared_buffer.c supervisor.c
OBJ_FILES = $(SRC_FILES:.c=.o)

all: generator supervisor

generator: generator.o adjacency_list.o shared_buffer.o
	$(CC) $(CFLAGS) -o $@ $^
	
supervisor: supervisor.o shared_buffer.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_FILES) generator supervisor
