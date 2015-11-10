#include <stdbool.h>
#include <string.h>
#include "types.h"
#include "eval.h"
#include "env.h"
#include "asserts.h"
#include "unused.h"
#include "parsing.h"
#include "mpc.h"

lval *lval_t(){ return lval_sym("T"); }
lval *lval_nil(){ return lval_qexpr(); }

bool lval_is_nil(lval *v){
    switch(v->type){
        case LVAL_SEXPR:
            return v->count == 0;
        case LVAL_QEXPR:
            return v->count == 0;
        case LVAL_SYM:
            return strlen(v->sym) == 0;
            /* return lval_is_nil(lval_eval(v)); */
        case LVAL_STR:
            return strlen(v->str) == 0;
        case LVAL_NUM:
            return v->num <= 0;
    }
    return false;
}


lval *builtin_car(lenv *UNUSED(e), lval *a){
    LASSERT_NUM("car", a, 1);
    LASSERT_TYPE("car", a, 0, LVAL_QEXPR);
    LASSERT_NOT_EMPTY("car", a, 0);

    lval *v = lval_take(a, 0);
    /* delete anything behind the first */
    while (v->count > 1){
        lval_del(lval_pop(v, 1));
    }
    return v;
}

lval *builtin_cdr(lenv *UNUSED(e), lval *a){
    LASSERT_NUM("cdr", a, 1);
    LASSERT_TYPE("cdr", a, 0, LVAL_QEXPR);
    LASSERT_NOT_EMPTY("cdr", a, 0);

    /* take first */
    lval *v = lval_take(a, 0);

    /* delete first then return */
    lval_del(lval_pop(v, 0));
    return v;
}

lval *builtin_list(lenv *UNUSED(e), lval *a){
    a->type = LVAL_QEXPR;
    return a;
}

lval *builtin_eval(lenv *e, lval *a){
    LASSERT(a, a->count == 1,
            "Too Many Arguments For eval. "
            "Expected 1, Got %i", a->count);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Incorrect Type For eval. "
            "Expected: %s, Got %s", ltype_name(LVAL_QEXPR),
                                     ltype_name(a->cell[0]->type));

    lval *x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval *lval_join(lval *x, lval *y){
    while (y->count){
        x = lval_add(x, lval_pop(y, 0));
    }

    lval_del(y);
    return x;
}

lval *builtin_join(lenv *UNUSED(e), lval *args){
    for (int i = 0; i < args->count; i++){
        LASSERT_TYPE("join", args, i, LVAL_QEXPR);
    }

    lval *x = lval_pop(args, 0);

    while (args->count){
        x = lval_join(x, lval_pop(args, 0));
    }

    lval_del(args);
    return x;
}

lval *builtin_op(lenv *UNUSED(e), lval *args, char *op){
    for (int i = 0; i < args->count; i++){
        if (args->cell[i]->type != LVAL_NUM){
           lval *err = lval_err("Can't Operate %s On Non-Number: %s",
                           op, ltype_name(args->cell[i]->type));
           lval_del(args);
           return err;
        }
    }

    lval *x = lval_pop(args, 0);

    if ((strcmp(op, "-") == 0) && args->count == 0){
        x->num = -x->num;
    }

    while (args->count > 0){
        lval *y = lval_pop(args, 0);

        if (strcmp(op, "+") == 0){ x->num += y->num; }
        if (strcmp(op, "-") == 0){ x->num -= y->num; }
        if (strcmp(op, "*") == 0){ x->num *= y->num; }
        if (strcmp(op, "/") == 0){
            if (y->num == 0){
                long n = x->num;
                lval_del(x);
                lval_del(y);
                x = lval_err("Division By Zero %li", n);
                break;
            }
            x->num /= y->num;
        }
        lval_del(y);
    }

    lval_del(args);
    return x;
}

lval *builtin_add(lenv *e, lval *args){
    return builtin_op(e, args, "+");
}

lval *builtin_sub(lenv *e, lval *args){
    return builtin_op(e, args, "-");
}

lval *builtin_mul(lenv *e, lval *args){
    return builtin_op(e, args, "*");
}

lval *builtin_div(lenv *e, lval *args){
    return builtin_op(e, args, "/");
}

lval *builtin_ord(lenv *UNUSED(e), lval *args, char *op){
    LASSERT_NUM(op, args, 2);
    LASSERT_TYPE(op, args, 0, LVAL_NUM);
    LASSERT_TYPE(op, args, 1, LVAL_NUM);

    long x = args->cell[0]->num;
    long y = args->cell[1]->num;

    bool r = false;

    if (strcmp(op, ">") == 0)
        r = (x > y);
    else if (strcmp(op, "<") == 0)
        r = (x < y);
    else if (strcmp(op, ">=") == 0)
        r = (x >= y);
    else if (strcmp(op, "<=") == 0)
        r = (x <= y);

    lval_del(args);
    return r? lval_t() : lval_nil();
}

lval *builtin_gt(lenv *e, lval *args){
    return builtin_ord(e, args, ">");
}

lval *builtin_lt(lenv *e, lval *args){
    return builtin_ord(e, args, "<");
}

lval *builtin_ge(lenv *e, lval *args){
    return builtin_ord(e, args, ">=");
}

lval *builtin_le(lenv *e, lval *args){
    return builtin_ord(e, args, "<=");
}

bool lval_eq(lval *x, lval *y){
    if (lval_is_nil(x)) return lval_is_nil(y);
    if (lval_is_nil(y)) return lval_is_nil(x);

    if (x->type != y->type)
        return false;

    switch(x->type){
        case LVAL_NUM:
            return (x->num == y->num);
        case LVAL_SYM:
            return (strcmp(x->sym, y->sym) == 0);
        case LVAL_STR:
            return (strcmp(x->str, y->str) == 0);
        case LVAL_ERR:
            return (strcmp(x->err, y->err) == 0);
        case LVAL_FUNC:
            if (x->builtin || y->builtin){
                return x->builtin == y->builtin;
            } else {
                return lval_eq(x->formals, y->formals) &&
                       lval_eq(x->body, y->body);
            }
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if (x->count != y->count)
                return false;

            for (int i = 0; i < x->count; i++){
                if (!lval_eq(x->cell[i], y->cell[i])) return false;
            }
            return true;
    }
    return false;
}

lval *builtin_cmp(lenv *UNUSED(e), lval *args, char *op){
    LASSERT_NUM(op, args, 2);

    bool r = false;
    if (strcmp(op, "==") == 0)
        r = lval_eq(args->cell[0], args->cell[1]);
    else if (strcmp(op, "!=") == 0)
        r = !lval_eq(args->cell[0], args->cell[1]);

    lval_del(args);
    return r? lval_t() : lval_nil();
}

lval *builtin_eq(lenv *e, lval *args){
    return builtin_cmp(e, args, "==");
}

lval *builtin_ne(lenv *e, lval *args){
    return builtin_cmp(e, args, "!=");
}

lval *builtin_if(lenv *e, lval *args){
    LASSERT_NUM("if", args, 3);
    LASSERT_TYPE("if", args, 1, LVAL_QEXPR);
    LASSERT_TYPE("if", args, 2, LVAL_QEXPR);

    args->cell[1]->type = LVAL_SEXPR;
    args->cell[2]->type = LVAL_SEXPR;

    lval *r = NULL;
    if (lval_is_nil(lval_pop(args, 0))){
        r = lval_eval(e, lval_pop(args, 1));
    } else {
        r = lval_eval(e, lval_pop(args, 0));
    }

    lval_del(args);
    return r;
}

lval *builtin_var(lenv *e, lval *a, char *func){
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

    lval *syms = a->cell[0];
    for (int i = 0; i < syms->count; i++){
        LASSERT(a, syms->cell[i]->type == LVAL_SYM,
                "Function '%s' cannot define non-symbol %s.",
                ltype_name(syms->cell[i]->type),
                ltype_name(LVAL_SYM));
    }

    LASSERT(a, syms->count == a->count - 1,
            "Function '%s' passed too many arguments."
            "Expected %d, Got %d.", func, syms->count, a->count - 1);

    for (int i = 0; i < syms->count; i++){
        if(strcmp(func, "set") == 0){
            lenv_set(e, syms->cell[i], a->cell[i + 1]);
        }
        if (strcmp(func, "setg") == 0){
            lenv_setg(e, syms->cell[i], a->cell[i + 1]);
        }
    }

    lval_del(a);
    return lval_sexpr();
}

lval *builtin_set(lenv *e, lval *a){
    return builtin_var(e, a, "set");
}

lval *builtin_setg(lenv *e, lval *a){
    return builtin_var(e, a, "setg");
}

lval *builtin_lambda(lenv *UNUSED(e), lval *args){
    LASSERT_NUM("lambda", args, 2);
    LASSERT_TYPE("lambda", args, 0, LVAL_QEXPR);
    LASSERT_TYPE("lambda", args, 1, LVAL_QEXPR);

    lval *syms = args->cell[0];
    for (int i = 0; i < syms->count; i++){
        LASSERT(args, syms->cell[i]->type == LVAL_SYM,
                "Argument must be a symbol. Got %s", ltype_name(syms->cell[i]->type));
    }

    lval *formals = lval_pop(args, 0);
    lval *body = lval_pop(args, 0);

    lval_del(args);
    return lval_lambda(formals, body);
}

lval *builtin_load(lenv *e, lval *a){
    LASSERT_NUM("load", a, 1);
    LASSERT_TYPE("load", a, 0, LVAL_STR);

    mpc_result_t r;
    if(mpc_parse_contents(a->cell[0]->str, Lispy, &r)){
        /* Read contents */
        lval *expr = lval_read(r.output);
        mpc_ast_delete(r.output);

        /* Eval each expression */
        while(expr->count){
            lval *x = lval_eval(e, lval_pop(expr, 0));
            if (x->type == LVAL_ERR) { lval_println(x); }
            lval_del(x);
        }

        lval_del(expr);
        lval_del(a);

        return lval_sexpr();
    } else {
        /* Get Parse Error as String */
        char *err_msg = mpc_err_string(r.error);
        mpc_err_delete(r.error);

        lval *err = lval_err("Could not load Library %s", err_msg);
        free(err_msg);
        lval_del(a);

        return err;
    }
}

lval *builtin_print(lenv *UNUSED(e), lval *args){
    for (int i = 0; i < args->count; i++){
        lval_print(args->cell[i]); putchar(' ');
    }

    putchar('\n');
    lval_del(args);
    return lval_sexpr();
}

lval *builtin_error(lenv *UNUSED(e), lval *a){
    LASSERT_NUM("error", a, 1);
    LASSERT_TYPE("error", a, 0, LVAL_STR);

    lval *err = lval_err(a->cell[0]->str);

    lval_del(a);
    return err;
}
