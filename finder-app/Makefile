CC = gcc
CROSS_COMPILE ?=

TARGET = writer

SRC = writer.c

CFLAGS ?=

all: $(TARGET)

$(TARGET): $(SRC)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o $@ $<

clean :
	rm -f $(TARGET) *.o
