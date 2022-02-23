
#ifndef READER_HEAD
#define READER_HEAD

#include "data.h"

Cell *lisp_read(FILE *input);
void print_expr(Cell *exp);

#endif

