#ifndef ENV_H
#define ENV_H

struct lval;
typedef struct lval lval;

#include "types.h"

struct lenv {
    lenv *parent;
    int count;
    char **syms;
    lval **vals;
};

lenv *lenv_new();
lenv *lenv_copy(lenv *e);
void lenv_del(lenv *e);
lval *lenv_get(lenv *e, lval *k);
void lenv_set(lenv *e, lval *k, lval *v);
void lenv_setg(lenv *e, lval *k, lval *v);
void lenv_add_builtins(lenv *e);
#endif
