
#ifndef LISP_HEADER
#define LISP_HEADER

#include "data.h"
#include "env.h"

Cell *eval(Cell *x, Environment *env);
Cell *apply(Cell *func, Cell *args);

#endif

