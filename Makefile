CC = gcc
CFLAGS = -std=c99 -O2 -g -Wall -Wextra -pedantic
LDFLAGS = 

SOURCEDIR = src/
HEADERDIR = src/

HEADER_FILES = chip8.h window.h 
SOURCE_FILES = main.c chip8.c window.c instructions.c

HEADERS_FP = $(addprefix $(HEADERDIR),$(HEADER_FILES))
SOURCE_FP = $(addprefix $(SOURCEDIR),$(SOURCE_FILES))

OBJECTS =$(SOURCE_FP:.c=.o)

TARGET = chip8

ifeq ($(OS),Windows_NT)
    CFLAGS += -IC:/SDL2/include
    LDFLAGS += -LC:/SDL2/lib -lSDL2main -lSDL2
    TARGET := $(TARGET).exe
    RM = del /Q
else
    CFLAGS += `sdl2-config --cflags`
    LDFLAGS += `sdl2-config --libs`
    RM = rm -f
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
ifeq ($(OS),Windows_NT)
	del /Q src\main.o src\chip8.o src\window.o src\instructions.o
else
	$(RM) $(OBJECTS)
endif

%.o: %.c $(HEADERS_FP)
	$(CC) $(CFLAGS) -c $< -o $@ || exit 1

-include $(OBJECTS:.o=.d)