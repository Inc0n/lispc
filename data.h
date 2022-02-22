
#ifndef LIST_HEADER
#define LIST_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#define DEBUG

#ifdef DEBUG
#define debuglog1(msg) printf("%-15s: " msg, __func__)
#define debuglog(fmt, ...) printf("%-15s: " fmt, __func__, __VA_ARGS__)
/* #define debuglogln(msg) printf("%-15s: " msg "\n", __func__) */

#define debugObj(cell, msg) ({                                          \
            printf("%s = ", #cell);                                     \
            print_expr(cell);                                           \
            printf(msg);                                                \
        })
#define debuglnObj(cell) debugObj(cell, "\n")
#else
#define debuglog1(msg)
#define debuglog(...)
#define debugObj(msg1, cell)
#define debuglnObj(cell)
#endif

#define throwError(msg, ...) ({                                         \
            char str[128];                                              \
            sprintf(str, "ERROR: %s, " msg,                             \
                    __func__, __VA_ARGS__);                             \
            return make_error(strdup(str));                             \
        })
#define ensure(exp, thetype) ({                                         \
            if (exp->type != thetype) {                                 \
                throwError("%s is not of type %d", #exp, thetype);      \
            }})
    
typedef enum {
    TypeUnknown, // 0
    //
    TypeInt, // 1
    TypeFloat,
    TypeRatio,
    TypeFixNum,
    //
    TypeString, // 5
    TypeSymbol,
    TypePair,
    TypePrim,
    TypeError, // 9
    TypeProcedure
} LispType;


typedef struct cell_t {
    LispType type;
    void *moveTo;
    void *val;
    struct cell_t *next;
} Cell;


// prim uses next field to store name 
#define prim_name(x) ((char*)x->next)

typedef struct {
    Cell *param;
    Cell *body;
    Cell *env;
} Procedure;

// GC code from https://github.com/munificent/lisp2-gc

#define STACK_MAX 256
#define HEAP_SIZE (1024 * 1024)

typedef struct {
    Cell *stack[STACK_MAX];

    int stackSize;
    unsigned int numObjs;

    // The beginning of the contiguous heap of memory that objects are allocated
    // from.
    void* heap;

    // The beginning of the next chunk of memory to be allocated from the heap.
    void* next;
} VM;


#define string_eq(x, y) (strcmp((char *)x, (char *)y) == 0)
#define cell_type(x) ((x)->type)

Cell *nil(void);

VM *getVM(void);
void gc(VM* vm);

Cell *make_cell(LispType type, void *data);
Cell *cons(Cell *x, Cell *y);
int count_obj(Cell *x);
int count_freeable_obj(Cell *x);

#define car(x)       ((x)->val)
#define cdr(x)       ((x)->next)
#define cddr(x)      (cdr(cdr(x)))
#define cadr(x)      (car(cdr(x)))
#define caddr(x)     (car(cdr(cdr(x))))


#define make_error(msg) make_cell(TypeError, (void*)msg)

#define is_atom(x)   ((x)->next == NULL)

#define is_integer(x)(cell_type(x) == TypeInt)
#define is_float(x)  (cell_type(x) == TypeFloat)
#define is_ratio(x)  (cell_type(x) == TypeRatio)

#define is_string(x) (cell_type(x) == TypeString)
#define is_symbol(x) (cell_type(x) == TypeSymbol)
#define is_pair(x)   (cell_type(x) == TypePair)
#define is_primitive(x) (cell_type(x) == TypePrim)
#define is_error(x)  (cell_type(x) == TypeError)
#define is_procedure(x) (cell_type(x) == TypeProcedure)

typedef Cell *(*PrimLispFn)(Cell*);

#define dolist(var, list) for (Cell *var = list; !null(var); var = cdr(var))
#define prog1(type, var, ret_exp, body) ({      \
            type var = ret_exp;                 \
            body;                               \
            return var;                         \
        })
#define lisp_true intern("t")

#define to_lisp_bool(x) ((x) ? lisp_true : nil())

void *intern(char *sym);
bool null(Cell *x);
bool is_number(Cell *x);

bool equal(Cell *x, Cell *y);

Cell *make_cCell(int num, ...);

Cell *reverse(Cell *l);

void set_car(Cell *c, Cell *val);
void set_cdr(Cell *c, Cell *val);

Cell *assoc(Cell *sym, Cell *alist);
int length(Cell *list);

#endif

