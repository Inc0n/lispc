
#include "env.h"
#include "reader.h"
#include "data.h"

#define env_addPrim(name, def, env) ({                                  \
            static Cell prim_name = {.type=TypeSymbol, .val=(name)};    \
            static Cell prim_def = {.type=TypePrim, .val=(def),         \
                                    .next=(void*)name};                 \
            env_add_var_def(&prim_name, &prim_def, env);                \
        })

Cell *prim_list(Cell *args) { return args; }
Cell *prim_eq(Cell *args) {
    return to_lisp_bool((Cell*)car(args) == (Cell*)cadr(args));
}
Cell *prim_cons(Cell *args) { return cons(car(args), cadr(args)); }
Cell *prim_car(Cell *args) { return car((Cell*)car(args)); }
Cell *prim_cdr(Cell *args) { return cdr((Cell*)car(args)); }
Cell *prim_atomp(Cell *args) { return to_lisp_bool(is_atom((Cell*)car(args))); }

Cell *prim_exit(Cell *args) { exit(1); }

Environment *init_environment() {
    Environment *env = cons(nil(), nil());
    env_addPrim("list", (void*)prim_list, env);
    env_addPrim("eq", (void*)prim_eq, env);
    env_addPrim("cons", (void*)prim_cons, env);
    env_addPrim("car", (void*)prim_car, env);
    env_addPrim("cdr", (void*)prim_cdr, env);
    env_addPrim("atom?", (void*)prim_atomp, env);
    env_addPrim("exit", (void*)prim_exit, env);
    return env;
}

Cell *env_add_var_def(Cell *var, Cell *val, Environment *env) {
    ensure(var, TypeSymbol);
    ensure(env, TypePair);
    Cell *frame = (Cell*)car(env);
    /* print_expr(env); */
    // check if at root frame
    if (null(cdr(env))) {
        /* debuglog("top level def %p\n", env); */
        /* set_car(env, _cons(_cons(var, val), frame)); */
    } else {
        /* debuglog("nested def %p\n", env); */
    }
    set_car(env, cons(cons(var, val), frame));
    return val;
}

Cell *env_lookup_var(Cell *var, Environment *env) {
    ensure(var, TypeSymbol);
    ensure(env, TypePair);
    /* debuglog("length of env, %p\n", env->type); */
    dolist_cdr(frame, env) {
        Cell *pair = assoc(var, car(frame));
        if (!null(pair)) {
            /* debuglog("variable found, %s\n", (char*)var->val); */
            Cell *def = cdr(pair);
            debuglog1("variable found, ");
            debugObj(var, ", ");
            debuglnObj(def);
            return def;
        }
    }
    return_error("variable not defined, %s\n", (char*)var->val);
}

Cell *env_set_variable_value(Cell *var, Cell *val, Environment *env) {
    ensure(var, TypeSymbol);
    ensure(env, TypePair);
    dolist_cdr(frame, env) {
        Cell *pair = assoc(var, car(frame));
        if (!null(pair)) {
            set_cdr(pair, val);
            return val;
        }
    }
    return nil();
}

Environment *env_extend_stack(Cell *arg_syms, Cell *args, Environment *env) {
    ensure(arg_syms, TypePair);
    ensure(args, TypePair);
    ensure(env, TypePair);
    env = cons(nil(), env);
    for (; !null(arg_syms); arg_syms = cdr(arg_syms), args = cdr(args)) {
        env_add_var_def(car(arg_syms), car(args), env);
    }
    return env;
}
