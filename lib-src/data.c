
#include <stdarg.h>
#include "data.h"
#include "reader.h"

/* #define TODO(str) (printf("at %s: %s", __func__, str);) */
#define TODO(str) ;


static Cell sym_nil = {.type = TypeSymbol, .val = NULL, .next = NULL};
static Cell *symbols = &sym_nil;
static VM *global_vm = NULL; 

Cell *nil(void) { return &sym_nil; }

VM *getVM(void) {
    if (global_vm == NULL) {
        // Creates a new VM with an empty stack and an empty (but allocated) heap.
        VM* vm = malloc(sizeof(VM));
        vm->stackSize = 0;
        vm->numObjs = 0;

        vm->heap = malloc(HEAP_SIZE);
        vm->next = vm->heap;

        global_vm = vm;
    }
    return global_vm;
}

int usedCells(VM* vm) {
    return ((vm->heap + HEAP_SIZE) - vm->next) / sizeof(Cell);
}

inline bool null(Cell *x) {
    return x == nil();
}

bool is_number(Cell *x) {
    return x->type == TypeInt
        || x->type == TypeFloat
        || x->type == TypeRatio
        || x->type == TypeFixNum;
}

//

// Marks [object] as being reachable and still (potentially) in use.
void mark(Cell* cell) {
    // If already marked, we're done. Check this first to avoid recursing
    // on cycles in the object graph.
    if (cell->moveTo) return;

    // Any non-zero pointer indicates the object was reached. For no particular
    // reason, we use the object's own address as the marked value.
    cell->moveTo = cell;

    // Recurse into the object's fields.
    if (is_pair(cell)) {
        mark(car(cell));
        mark(cdr(cell));
    }
}

// The mark phase of garbage collection. Starting at the roots (in this case,
// just the stack), recursively walks all reachable objects in the VM.
void markAll(VM* vm) {
    for (int i = 0; i < vm->stackSize; i++) {
        mark(vm->stack[i]);
    }
}


// Phase one of the LISP2 algorithm. Walks the entire heap and, for each live
// object, calculates where it will end up after compaction has moved it.
//
// Returns the address of the end of the live section of the heap after
// compaction is done.
void* calculateNewLocations(VM* vm) {
    // Calculate the new locations of the objects in the heap.
    Cell* from = vm->heap;
    Cell* to = vm->heap;
    while (from < vm->next) {
        Cell* object = from;
        if (object->moveTo) {
            object->moveTo = to;

            // We increase the destination address only when we pass a live object.
            // This effectively slides objects up on memory over dead ones.
            to ++;
        }
        from ++;
    }

    return to;
}

// Phase two of the LISP2 algorithm. Now that we know where each object *will*
// be, find every reference to an object and update that pointer to the new
// value. This includes reference in the stack, as well as fields in (live)
// objects that point to other objects.
//
// We do this *before* compaction. Since an object's new location is stored in
// [object.moveTo] in the object itself, this needs to be able to find the
// object. Doing this process before objects have been moved ensures we can
// still find them by traversing the *old* pointers.
void updateAllObjectPointers(VM* vm) {
    // Walk the stack.
    for (int i = 0; i < vm->stackSize; i++) {
        // Update the pointer on the stack to point to the object's new compacted
        // location.
        vm->stack[i] = vm->stack[i]->moveTo;
    }

    // Walk the heap, fixing fields in live pairs.
    Cell* from = vm->heap;
    while (from < vm->next) {
        Cell* object = (Cell*)from;

        if (object->moveTo && is_pair(object)) {
            object->val = ((Cell*)object->val)->moveTo;
            object->next = object->next->moveTo;
        }

        from += 1;
    }
}

// Phase three of the LISP2 algorithm. Now that we know where everything will
// end up, and all of the pointers have been fixed, actually slide all of the
// live objects up in memory.
void compact(VM* vm) {
    Cell* from = vm->heap;

    while (from < vm->next) {
        Cell* object = (Cell*)from;
        if (object->moveTo) {
            // Move the object from its old location to its new location.
            Cell* to = object->moveTo;
            memmove(to, object, sizeof(Cell));

            // Clear the mark.
            to->moveTo = NULL;
        }

        from += 1;
    }
}

// Free memory for all unused objects.
void gc(VM* vm) {
    // Find out which objects are still in use.
    markAll(vm);

    // Determine where they will end up.
    void* end = calculateNewLocations(vm);

    // Fix the references to them.
    updateAllObjectPointers(vm);

    // Compact the memory.
    compact(vm);

    // Update the end of the heap to the new post-compaction end.
    vm->next = end;

    printf("%ld live bytes after collection.\n", vm->next - vm->heap);
}


Cell* newObject(VM* vm) {
    if ((vm->next + 1) > (vm->heap + HEAP_SIZE)) {
        gc(vm);

        // If there still isn't room after collection, we can't fit it.
        if ((vm->next + 1) > (vm->heap + HEAP_SIZE)) {
            perror("Out of memory");
            exit(1);
        }
    }

    vm->numObjs++;
    vm->next += 1;

    Cell* object = (Cell*)vm->next;
    return object;
}

//

Cell *make_cell(LispType type, void *data) {
    Cell *_cell = newObject(getVM());
    debuglog("type=%d, %p\n", type, data);
    /* print_expr(_cell); */
    /* Cell *_cell = calloc(1, sizeof(Cell)); */
    _cell->type = type;
    _cell->val = data;
    _cell->next = NULL;
    return _cell;
}

Cell *cons(Cell *x, Cell *y) {
    Cell *_pair = make_cell(TypePair, x);
    _pair->next = y;
    return _pair;
}

int count_obj(Cell* cell) {
    if (null(cell)) {
        return 0;
    }
    // check procedure before list
    // procedure has cylic reference to env, this is a black hole (segment fault)
    else if (is_atom(cell) || is_primitive(cell) || is_procedure(cell)) {
        return 1;
    }
    else if (is_pair(cell)) {
        return 1 + count_obj(car(cell)) + count_obj(cdr(cell));
    }
    else {
        printf("<%s: unsupported exp type=%d>", __func__, cell->type);
    }
    return 1;
}


int count_freeable_obj(Cell* cell) {
    if (null(cell) || is_symbol(cell)) {
        return 0;
    }
    else if (is_atom(cell)) {
        return 1;
    }
    // check procedure before list
    else if (is_pair(cell)) {
        return 1 + count_obj(car(cell)) + count_obj(cdr(cell));
    }
    else {
        printf("<%s: unsupported exp type=%d>", __func__, cell->type);
    }
    return 0;
}

//

void *intern(char *sym) {
    if (sym == NULL) return symbols;

    /* debuglog("interning symbol %s\n", sym); */
    dolist_cdr(_pair, symbols) {
        /* debuglog("interning symbol, %p, %p|\n", car(_pair), cdr(_pair)); */
        if (car(_pair)
            && strncmp(sym, (char*)((Cell*)car(_pair))->val, 32) == 0)
            return car(_pair);
    }

    Cell *newSym = make_cell(TypeSymbol, strdup(sym));
    debuglog("creating new symbol %s\n", sym);
    symbols = cons(newSym, symbols);
    return car(symbols);
}

bool equal(Cell *x, Cell *y) {
    if (x == y) {
        return true;
    }
    if (x->type == y->type) {
        switch(x->type) {
        case TypeString:
        case TypeSymbol:
            return string_eq(x->val, y->val);
        case TypePair:
            return (equal((Cell *)car(x),
                          (Cell *)car(y))
                    && equal(cdr(x),
                             cdr(y)));
        case TypeInt:
        case TypeFixNum:
            return *((int*)x->val) == *((int *)y->val);
        case TypeFloat:
            return *((float*)x->val) == *((float *)y->val);
        case TypeRatio:
            TODO("implement strust of ratio w {denom, detanator}");
        default:
            TODO("should raise error for undefined type?")
            return x->val == y->val;
        }
    }
    return false;
}

/* Cell *_reverse(Cell *l, Cell *acc) { */
/*     if (null(l)) { */
/*         return acc; */
/*     } else { */
/*         return _reverse(cdr(l), cons(car(l), acc)); */
/*     } */
/* } */

/* Cell *reverse(Cell *l) { */
/*     if (is_pair(l)) { */
/*         return _reverse(l, nil()); */
/*     } */
/*     TODO("should signal error for not list arg") */
/*     return nil(); */
/* } */

//
Cell *_make_list(int num, va_list valist) {
    if (num > 0) {
        Cell *tempC = va_arg(valist, Cell *);
        return cons(tempC , _make_list(num - 1, valist));
    }
    /* clean memory reserved for valist */
    va_end(valist);
    return nil();
}

Cell *make_cCell(int num, ...) {
    va_list valist;

    /* initialize valist for num number of arguments */
    va_start(valist, num);
    return _make_list(num, valist);
}

inline void set_car(Cell *c, Cell *val) {
    c->val = val;
}
inline void set_cdr(Cell *c, Cell *val) {
    c->next = val;
}


Cell *assoc(Cell *sym, Cell *alist) {
    dolist_cdr(c, alist) {
        Cell *pair = car(c);
        /* debuglog("env_lookup_var: %p, %d, %s\n", */
        /*          car(pair), ((Cell*)car(pair))->type, var->val); */
        /* print_expr(pair); */
        if (equal(car(pair), sym)) {
            return pair;
        }
    }
    return nil();
}

int length(Cell *list) {
    int acc = 0;
    dolist_cdr(c, list) {
        acc++;
    }
    return acc; 
}
