TARGET = snake
LIBS = 
CC = gcc
CFLAGS = -lncurses -lpthread

.PHONY: default all

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(CFLAGS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f *.tmp
	-rm -f $(TARGET)
