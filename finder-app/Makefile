CC = gcc
TARGET = writer
OBJECTS = writer.o
CFLAGS = -Wall -g
LDFLAGS =

ifneq ($(CROSS_COMPILE),)
	CC = $(CROSS_COMPILE)gcc
	LDFLAGS += -static
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

writer.o: writer.c
	$(CC) $(CFLAGS) -c writer.c

clean :
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean
