
#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "env.h"
#include "reader.h"
#include "lisp.h"

/* #define is_symbol_eq(x, y) (x == intern(y)) */

#define def_prim_symbol_test(x) bool is_## x(Cell *list) { \
        return car(list) == intern(#x);                    \
    }
#define def_prim_symbol_test_manual(x, y) bool is_ ## x(Cell *list) { return car(list) == intern(y); }

bool is_self_evaluating(Cell *x) {
    return is_number(x) || is_string(x);
}

bool is_variable(Cell *x) {
    return is_symbol(x);
}

def_prim_symbol_test(quote)

Cell *eval_quote(Cell *expr, Environment *env) {
    return cadr(expr);
}

def_prim_symbol_test(if)

Cell *eval_if(Cell *expr, Environment *env) {
    if (null(eval(cadr(expr), env))) {
        return null(cdr(cdr(expr)))
            ? eval(cadr(cdr(expr)), env)
            : nil();
    } else {
        return eval(caddr(expr), env);
    }
}

def_prim_symbol_test_manual(assignment, "set!")

Cell *eval_assignment(Cell *exp, Environment *env) {
    Cell *var = cadr(exp);
    Cell *val = caddr(exp);
    return env_set_variable_value(var, val, env);
}

def_prim_symbol_test(lambda); // need this test for (eval (lambda ()))
/* def_prim_symbol_test(procedure); */

Cell *make_procedure(Cell *param, Cell *body, Environment *env) {
    /* Cell *param = cadr(exp); */
    /* Cell *body = caddr(exp); */
    Procedure *proc = calloc(1, sizeof(Procedure));
    proc->param = param;
    proc->body = body;
    proc->env = env;
    return make_cell(TypeProcedure, proc);
}

Cell *eval_lambda(Cell *exp, Environment *env) {
    return make_procedure(cadr(exp), caddr(exp), env);
}

def_prim_symbol_test(define)

Cell *eval_definition(Cell *expr, Environment *env) {
    Cell *var = cadr(expr);
    if (is_pair(var)) {
        debuglog1("defining a function\n");
        Cell *fn_name = car(var);
        Cell *args = cdr(var);
        Cell *proc = make_procedure(args, cddr(expr), env); 
        env_add_var_def(fn_name, proc, env);
        debuglog1("function defined\n");
        return proc;
    } else {
        Cell *val = caddr(expr);
        env_add_var_def(var, val, env);
        return val;
    }
}

/* Cell *make_lambda(Cell *param, Cell *body) { */
/*     Cell *name = intern("lambda"); */
/*     return make_cCell(3, &name, param, body); */
/* } */

def_prim_symbol_test_manual(sequence, "begin")

Cell *eval_sequence(Cell *exps, Environment *env) {
    Cell *out = nil();
    dolist(exp, exps) {
        out = eval(car(exp), env);
    }
    return out;
}

Cell *eval_begin(Cell *expr, Environment *env) {
    return eval_sequence(cdr(expr), env);
}

Cell *list_of_values(Cell *expr, Environment *env) {
    if (null(expr)) {
        return nil();
    } else {
        Cell *c = eval(car(expr), env);
        return cons(c, list_of_values(cdr(expr), env));
    /*     return list_of_values(cdr(expr), */
    /*                           env, */
    /*                           cons(c, acc)); */
    }
}

Cell *apply(Cell *func, Cell *args) {
    if (is_procedure(func)) {
        debuglog1("procedure - ");
        debugObj(func, ", ");
        debuglnObj(args);
        Procedure *proc = func->val;
        //
        Cell *arg_syms = proc->param;
        Cell *body = proc->body;
        Environment *env = proc->env;
        env = env_extend_stack(arg_syms, args, env);
        return eval_sequence(body, env);
    }
    else if (is_primitive(func)) {
        debuglog1("primitive - ");
        debugObj(func, ", ");
        debuglnObj(args);
        return ((PrimLispFn)func->val)(args);
    }
    debuglog1("ERROR unsupported function\n");
    debuglog1("");
    debugObj(func, ", ");
    debuglnObj(args);
    return nil();
}

Cell *eval_apply(Cell *expr, Environment *env) {
    debuglog1("");
    debugObj(expr, ", ");
    printf("env = %p\n", env);
    Cell *var = car(expr);
    Cell *args = cdr(expr);
    Cell *fn = eval(var, env);
    if (null(fn)) {
        return nil();
    }
    Cell *args_vals = list_of_values(args, env);
    return apply(fn, args_vals);
}

Cell *eval(Cell *exp, Environment *env)
{
    debuglog1("");
    debugObj(exp, ", ");
    printf("env = %p\n", env);
    if (is_self_evaluating(exp)) {
        // dont print, segment fault if exp is number
        /* debuglog("is self evaluate%s\n", (char*)exp->val); */
        return exp;
    }
    else if (is_error(exp)) {
        // maybe handle error in a special way
        return exp;
    }
    else if (is_variable(exp)) {
        /* debuglog("is variable, %s, %p\n", (char*)exp->val, env); */
        return env_lookup_var(exp, env);
    }
    else if (is_pair(exp)) {
        if (is_quote(exp)) {      // (quote exp)
            return eval_quote(exp, env);
        }
        else if (is_if(exp)) {    // (if test conseq [alt])
            return eval_if(exp, env);
        }
        else if (is_assignment(exp)) { // (set! var exp)
            return eval_assignment(exp, env);
        }
        else if (is_define(exp)) {
            return eval_definition(exp, env);
        }
        else if (is_lambda(exp)) {
            return eval_lambda(exp, env);
        }
        else if (is_sequence(exp)) {
            return eval_begin(exp, env);
        }
        /* else if (is_application(exp)) { */
        return eval_apply(exp, env);
        /* } */
    }

    // (proc exp*)
    printf("not a function\n");
    exit(1);
}

