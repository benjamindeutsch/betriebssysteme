#cpair
#@author Benjamin Deutsch (12215881)
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
TARGET = cpair

all: $(TARGET)

$(TARGET): cpair.c
	$(CC) $(CFLAGS) -o $@ $< -lm

clean:
	rm -f $(TARGET)
