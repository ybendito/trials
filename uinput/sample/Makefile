
TARGET = uinput
LIBS = `pkg-config --libs gtk+-3.0`
CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -I. 
ifneq ($(DEBUG),)
CFLAGS += -O0 -g -ggdb -rdynamic
endif

.PHONY: default all clean

default: $(TARGET)
all: default

#OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJECTS = uinput-sample.o

#HEADERS = $(wildcard *.h)
#HEADERS = usb-device-manager.h usb-device-widget.h spice-client.h config.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o $(TARGET)
