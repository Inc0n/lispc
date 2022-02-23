#
#         File: Makefile
#       Author: Danny He
#      License: Apache License
#         Date: 22th February 2022
#  Description: Makefile to build lisp
#

CC  = gcc
C++ = g++

HEADERS_DIR = header

LDFLAGS = -shared
CFLAGS 	= -pedantic -Wall -Wno-gnu-statement-expression -I$(HEADERS_DIR)
OBJ_DIR = obj
OUTPUT_DIR = build


BIN_TARGET  = $(OUTPUT_DIR)/lisp.out

BIN_SRC = main-repl.c
BIN_OBJ = $(BIN_SRC:%.c=$(OBJ_DIR)/%.o)


LIB_TARGET	= $(OUTPUT_DIR)/lisp.so

LIB_SRC_DIR = lib-src
LIB_SRC = $(wildcard $(LIB_SRC_DIR)/*.c)
# LIB_OBJ = $($(notdir $(LIB_SRC)):%.c=$(OBJ_DIR)/%.o)
LIB_OBJ = $(LIB_SRC:$(LIB_SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
# data.c env.c lisp.c reader.c

# $(addprefix $(OBJ_DIR)/, )

all: PREP help
# all:
# 	$(CC) data.c env.c lisp.c reader.c main-repl.c -o $(BIN_TARGET)

lib: PREP $(LIB_TARGET)
bin: PREP $(BIN_TARGET)

PREP:
	@mkdir -p $(OBJ_DIR) $(OUTPUT_DIR)

help:
	@echo make [option]
	@echo 	options:
	@echo 		bin - build the binary
	@echo 		lib - build the library

obj/%.o : %.c 
	$(CC) $(CFLAGS) -c $< -o $@

# obj/%.o : $(LIB_SRC_DIR)/%.c 
# 	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_TARGET):
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(LIB_SRC)
# $(LIB_OBJ)
# $^, replace with the arguements 

$(BIN_TARGET): $(LIB_TARGET) $(BIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -r $(OBJ_DIR) $(OUTPUT_DIR)
