
/* The register machine */
#include "data.h"
#include <stdbool.h>

// a customized, variant of type Cell
typedef struct {
    LispType type;
    void *val;
    bool monitorp;
} Register;

typedef struct {
    Cell *exp;
    Cell *compiledproc;
} Instruction;

#define STACK_MAX_SIZE 256

typedef struct {
    int maxdepth;
    int numpushes;
    int currentdepth;
    void *_stack[STACK_MAX_SIZE];
    void **next;
} Stack;

void initialize_stack(Stack *stack) {
    stack->maxdepth = 0;
    stack->numpushes = 0;
    stack->currentdepth = 0;
    stack->next = stack->_stack;
}
void push_stack(Stack *stack, void *val) {
    stack->numpushes++; 
    stack->currentdepth++; 
    if (stack->maxdepth < stack->currentdepth) 
        stack->maxdepth = stack->currentdepth;
    // actual push
    *(stack->next) = val;
    stack->next++;
}

void* pop_stack(Stack *stack) {
    return stack->next--;
}

void print_stack_stats(Stack *stack) {
    printf("total pushes = %d, max-depth = %d\n",
           stack->numpushes, stack->maxdepth);
}

typedef struct {
    Register PC;
    Register Flag;
    Stack stack;
    Register regs[7];
} Machine;
