

#include <stdio.h>

#include "lisp.h"
#include "reader.h"

int main() {
    Cell *env = init_environment();

    while (true) {
        debuglog("before, %d(%d)\n", getVM()->numObjs, count_obj(env));
        printf(";;; Eval input:\n");
        Cell *exp = lisp_read(stdin);
        /* print_expr(exp); */
        /* exit(1); */
        printf("\n");
        Cell *result = eval(exp, env);

        printf(";;; Eval value:\n");
        print_expr(result);
        printf("\n");

        /* int freed = destroyObject(getVM(), exp); */
        debuglog("after, %d(%d)\n", getVM()->numObjs, count_obj(env));
    }
}
