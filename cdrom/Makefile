
CC = gcc
CFLAGS = -I. 
ifneq ($(DEBUG),)
CFLAGS += -O0 -g -ggdb -rdynamic
endif

.PHONY: default all clean

default: cdrom
all: default

#OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJECTS = client-vsock.o server-vsock.o

#HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: client-vsock server-vsock $(OBJECTS)

cdrom:  cdrom.o
	$(CC) cdrom.o -Wall $(LIBS) -o $@

clean:
	-rm -f *.o cdrom
