#mycompress
#@author Benjamin Deutsch (12215881)
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
TARGET = mycompress

all: $(TARGET)

$(TARGET): mycompress.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
