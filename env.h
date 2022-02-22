#ifndef ENV_HEADER
#define ENV_HEADER

#include "data.h"

typedef Cell Environment;

/* typedef struct { */
/*     Cell *frames; */
/*     Cell *root; */
/* } Environment; */

Environment *init_environment();
Cell *env_add_var_def(Cell *var, Cell *val, Environment *env);
Cell *env_lookup_var(Cell *var, Environment *env);
Cell *env_set_variable_value(Cell *var, Cell *val, Environment *env);
Environment *env_extend_stack(Cell *arg_syms, Cell *args, Environment *env);

#endif

