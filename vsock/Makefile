
CC = gcc
CFLAGS = -I. 
ifneq ($(DEBUG),)
CFLAGS += -O0 -g -ggdb -rdynamic
endif

.PHONY: default all clean

default: client-vsock server-vsock speed-test
all: default

#OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJECTS = client-vsock.o server-vsock.o

#HEADERS = $(wildcard *.h)
#HEADERS = usb-device-manager.h usb-device-widget.h spice-client.h config.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: client-vsock server-vsock speed-test $(OBJECTS)

client-vsock:  client-vsock.o
	$(CC) client-vsock.o -Wall $(LIBS) -o $@

server-vsock:  server-vsock.o
	$(CC) server-vsock.o -Wall $(LIBS) -o $@

speed-test:  speed-test.o
	$(CC) speed-test.o -Wall $(LIBS) -o $@

clean:
	-rm -f *.o client-vsock server-vsock speed-test
