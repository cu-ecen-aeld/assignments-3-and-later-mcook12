CC = gcc
TARGET = aesdsocket
OBJECTS = aesdsocket.o
CFLAGS = -Wall -g
LDFLAGS =

ifneq ($(CROSS_COMPILE),)
	CC = $(CROSS_COMPILE)gcc
	LDFLAGS += -static
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c

clean :
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean
