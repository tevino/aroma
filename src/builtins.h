#ifndef BUILTINS_H
#define BUILTINS_H
#include <stdbool.h>
#include "types.h"
#include "env.h"
#include "unused.h"

lval *builtin_car(lenv *UNUSED(e), lval *a);
lval *builtin_cdr(lenv *UNUSED(e), lval *a);
lval *builtin_list(lenv *UNUSED(e), lval *a);
lval *builtin_eval(lenv *e, lval *a);
lval *lval_join(lval *x, lval *y);
lval *builtin_join(lenv *UNUSED(e), lval *args);
lval *builtin_op(lenv *UNUSED(e), lval *args, char *op);
lval *builtin_add(lenv *e, lval *args);
lval *builtin_sub(lenv *e, lval *args);
lval *builtin_mul(lenv *e, lval *args);
lval *builtin_div(lenv *e, lval *args);
lval *builtin_ord(lenv *UNUSED(e), lval *args, char *op);
lval *builtin_gt(lenv *e, lval *args);
lval *builtin_lt(lenv *e, lval *args);
lval *builtin_ge(lenv *e, lval *args);
lval *builtin_le(lenv *e, lval *args);
bool lval_eq(lval *x, lval *y);
lval *builtin_cmp(lenv *UNUSED(e), lval *args, char *op);
lval *builtin_eq(lenv *e, lval *args);
lval *builtin_ne(lenv *e, lval *args);
bool lval_is_nil(lval *v);
lval *builtin_if(lenv *e, lval *args);
lval *builtin_var(lenv *e, lval *a, char *func);
lval *builtin_set(lenv *e, lval *a);
lval *builtin_setg(lenv *e, lval *a);
lval *builtin_lambda(lenv *UNUSED(e), lval *args);
lval *builtin_load(lenv *e, lval *a);
lval *builtin_print(lenv *UNUSED(e), lval *args);
lval *builtin_error(lenv *UNUSED(e), lval *a);
lval *lval_t();
lval *lval_nil();
#endif
