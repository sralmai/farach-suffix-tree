TARGET = farach_suftree
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall -std=c99 -pedantic-errors

REMOVECOMMAND = rm
TARGETEXECUTABLE = $(TARGET)

ifeq ($(OS),Windows_NT)
    REMOVECOMMAND = del
    TARGETEXECUTABLE = $(TARGET).exe
endif

.PHONY: test default all clean

test: $(TARGET) tests/huckleberry_finn tests/huckleberry_finn_tests
	$+ >tests/output

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	$(REMOVECOMMAND) -f *.o
	$(REMOVECOMMAND) -f $(TARGETEXECUTABLE)
