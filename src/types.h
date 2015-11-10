#ifndef TYPES_H
#define TYPES_H

struct lenv;
typedef struct lenv lenv;
#include "env.h"

enum { LVAL_NUM, LVAL_SYM, LVAL_STR,
       LVAL_FUNC,
       LVAL_SEXPR, LVAL_QEXPR,
       LVAL_ERR };


typedef lval*(*lbuiltin)(lenv*, lval*);

struct lval {
    int type;

    /* Basic */
    long num;
    char *err;
    char *sym;
    char *str;

    /* Function */
    lbuiltin builtin;
    lenv *env;
    lval *formals;
    lval *body;

    /* Expression */
    int count;
    lval **cell;
};


char *ltype_name(int t);

lval *lval_add(lval *v, lval *x);
lval *lval_copy(lval *v);
void lval_del(lval *v);

lval *lval_func(lbuiltin builtin);
lval *lval_lambda(lval *formals, lval *body);
lval *lval_num(long x);
lval *lval_err(char *fmt, ...);
lval *lval_sym(char *s);
lval *lval_str(char *s);
lval *lval_sexpr();
lval *lval_qexpr();
#endif
