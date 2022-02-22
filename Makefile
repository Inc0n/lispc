#
#         File: makefile
#       Author: Steve Gunn
#      License: MIT License
#         Date: 27th October 2018
#  Description: Makefile to build circuit simulator.
#

CC 		= gcc

CFLAGS 	= -pedantic -Wall -Wextra
LDFLAGS = -shared

HEADERS = $(wildcard include/*.h)
OBJ_DIR = obj

BINARY  = lisp
BIN_SRC = repl.c
BIN_OBJ = $(addprefix $(OBJ_DIR)/, $(BIN_SRC:.c=.o))

TARGET	= $(BINARY).so
LIB_SRC = $(wildcard src/*.c) data.c reader.c env.c lisp.c
LIB_OBJ = $(addprefix $(OBJ_DIR)/, $(LIB_SRC:.c=.o))

# $(LIB_SRC:.c=.o)
# $(addprefix $(OBJ_DIR)/, $(LIB_SRC:.c=.o))

# all: PREP HELP
all:
	$(CC) data.c env.c lisp.c reader.c main-repl.c
lib: PREP $(TARGET)
bin: PREP $(BINARY)

PREP:
	@mkdir -p $(OBJ_DIR)

HELP:
	@echo make [option]
	@echo 	options:
	@echo 		bin - build the binary
	@echo 		lib - build the library

obj/%.o : %.c
	$(CC) $(CsLAGS) -c $< -o $@

$(TARGET): $(LIB_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(BINARY): $(LIB_OBJ) $(BIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -r $(OBJ_DIR)
