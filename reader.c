
#include "reader.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"


int is_space(char x) { return x == ' ' || x == '\n'; }
int is_parens(char x) { return x == '(' || x == ')'; }
int is_double_quotes(char x) { return x == '"'; }

#define SYMBOL_MAX 32
#define is_valid_char(look) (look != EOF                    \
                             && !is_space(look)             \
                             && !is_parens(look)            \
                             && !is_double_quotes(look))

static char get_next_char(FILE *input) {
    char look = getc(input);
    while(is_space(look)) { look = getc(input); }
    return look;
}

static char *gettoken(FILE *input) {
    int index = 0;
    char token[SYMBOL_MAX]; /* token */
    char look = get_next_char(input);

    if (look == EOF) {
        exit(1);
    }
    else if (is_parens(look) || is_double_quotes(look)) {
        token[index++] = look;
    }
    else {
        while(index < SYMBOL_MAX - 1 && is_valid_char(look)) {
            token[index++] = look;
            look = getc(input);
        }
        // put the extra char read back into
        ungetc(look, input);
    }
    token[index] = '\0';
    return strdup(token);
}

LispType getNumType(char *token, LispType t)
{
    if (isdigit(*token)) {
        return getNumType(token + 1,
                          t == TypeUnknown ? TypeInt : t);
    }
    else if (t == TypeInt && *token == '.') {
        return getNumType(token + 1, TypeFloat);
    }
    else if (t == TypeInt && *token == '/') {
        return getNumType(token + 1, TypeRatio);
    }
    if (*token == '\0') {
        return t;
    }
    // type_unknown = 0 or false
    return TypeUnknown;
}

Cell *getlist(FILE *input);
Cell *getstring(FILE *input);
Cell *getnumber(LispType t, char *token);

Cell *getobj(FILE *input) {
    char *token = gettoken(input);
    LispType type = TypeUnknown;

    /* debuglog("Getting obj start, %s\n", token); */
    if (token[0] == '(')
        return getlist(input);
    else if (token[0] == '"')
        return getstring(input);
    else if ((type = getNumType(token, type)) != TypeUnknown)
        return getnumber(type, token);
    return intern(token);
}

Cell *getlist(FILE *input) {

    /* debuglogln("Getting list start"); */
    // N/A -- check for missing closing paren
    // stdin will hang when list is not balanced.

    char peek = get_next_char(input); 
    if (peek == ')')
        return nil();
    ungetc(peek, input);
    return cons(getobj(input), getlist(input));
}

Cell *getstring(FILE *input) {

    char *token = gettoken(input);
    /* debuglog("Getting list start with content %s\n", token); */
    Cell *tmp = make_cell(TypeString, token);
    token = gettoken(input);
    if (token[0] != '"') {
        printf("error reading string, missing \"\n");
        return nil();
    }
    return tmp;
}

Cell *getnumber(LispType type, char *token) {

    /* debuglogln("Getting number start"); */
    if (type == TypeInt) {
        return make_cell(type, (void *)atoi(token));
    }
    if (type == TypeFloat) {
        float *f = malloc(sizeof(float));
        *f = atof(token);
        return make_cell(type, f);
    }
    return nil();
}
Cell *lisp_read(FILE* input) {

    prog1(Cell*, res, getobj(input),
          debuglog("after, read %d, %d(%d)\n",
                   getVM()->numObjs, count_obj(res), count_freeable_obj(res)));
    return getobj(input);
}

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

Cell *read_from_string(const char* string) {
    pid_t pid;
    int pipeIDs[2];

    if (pipe(pipeIDs)) {
        fprintf (stderr, "ERROR, cannot create pipe.\n");
        return nil();
    } 

    pid = fork();
    if (pid == (pid_t) 0) {
        /* Write to PIPE in this THREAD  */

        FILE * file = fdopen( pipeIDs[1], "w");

        fprintf(file, "%s", string);
        return nil();

    } else if (pid < (pid_t) 0) {
        fprintf (stderr, "ERROR, cannot create thread.\n");
        return nil();
    }

    FILE* myFile = fdopen(pipeIDs[0], "r");
    Cell *obj = lisp_read(myFile);
    fclose(myFile);
    return obj;
}

void print_expr(Cell *exp) {
    if (null(exp)) {
        printf("nil");
    }
    else if (is_symbol(exp) || is_string(exp) || is_error(exp)) {
        printf("%s", (char *)exp->val);
    }
    // check procedure before list
    else if (is_procedure(exp)) {
        printf("<Proc %p>", (void *)exp);
    }
    else if (is_pair(exp)) {
        printf("(");
        /* debuglog("print_expr: %s, %d\n", ((Cell*)car(exp))->val, ((Cell*)car(exp))->type); */
        print_expr(car(exp));
        /* debuglog("print_expr: mid %p, %d\n", (exp)->next, exp->type); */
        Cell *e = cdr(exp);
        /* debuglog("print_expr: after %d, %d\n", e->next == NULL, e->type); */
        if (!null(e) && is_atom(e)) {
            // print cons
            printf(" . ");
            print_expr(e);
            printf(")");
        } else {
            // print normal list
            for (; e && !null(e); e = cdr(e)) {
                printf(" ");
                /* printf("SPACE %s %d", ((Cell*)e)->val, ((Cell*)e)->type); */
                print_expr(car(e));
            }
            printf(")");
        }
    }
    else if (is_integer(exp)) {
        printf("%d", (int)exp->val);
    }
    else if (is_float(exp)) {
        printf("%f", *(float *)exp->val);
    }
    else if (is_primitive(exp)) {
        printf("<Prim %s %p>", prim_name(exp), exp);
    }
    else {
        // should not reach this stage
        printf("<%s: unsupported exp type=%d>", __func__, exp->type);
    }
}