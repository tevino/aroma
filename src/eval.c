#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "eval.h"
#include "env.h"
#include "builtins.h"


lval *lval_call(lenv *e, lval *f, lval *a){
    if (f->builtin){ return f->builtin(e, a); }

    int given = a->count;
    int required = f->formals->count;

    while (a->count){
        if (f->formals->count == 0){
            lval_del(a);
            return lval_err("Function passed too many arguments."
                            "Expected %d, got %d.", required, given);
        }

        lval *sym = lval_pop(f->formals, 0);

        if (strcmp(sym->sym, "&") == 0){
            /* Ensure & is followed by another symbol */
            if (f->formals->count != 1){
                lval_del(a);
                return lval_err("& not followed by single symbol");
            }

            /* Bound next formal to remaining arguments */
            lval *nsym = lval_pop(f->formals, 0);
            lenv_set(f->env, nsym, builtin_list(e, a));
            lval_del(sym);
            lval_del(nsym);
            break;
        }

        /* Take out the argument */
        lval *val = lval_pop(a, 0);

        /* Bound to sym */
        lenv_set(f->env, sym, val);

        lval_del(sym);
        lval_del(val);
    }

    lval_del(a);

    /* If & remaining in argument list, bound to () */
    if (f->formals->count > 0 && 
        strcmp(f->formals->cell[0]->sym, "&") == 0){

        /* Ensure & is not passed invalidly */
        if(f->formals->count != 2){
            return lval_err("& not followed by single symbol");
        }

        /* Delete & */
        lval_del(lval_pop(f->formals, 0));

        /* pop next formal */
        lval *sym = lval_pop(f->formals, 0);

        lval *empty = lval_qexpr();

        lenv_set(f->env, sym, empty);
        lval_del(sym);
        lval_del(empty);
    }


    if (f->formals->count == 0){
        f->env->parent = e;

        return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    } else {
        return lval_copy(f);
    }
}

lval *lval_pop(lval *v, int i){
    lval *x = v->cell[i];

    memmove(&v->cell[i], &v->cell[i + 1],
            sizeof(lval*) * (v->count - i - 1));
    v->count--;

    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval *lval_take(lval *v, int i){
    lval *x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval *lval_eval_sexpr(lenv *e, lval *v){
    for (int i = 0; i < v->count; i++){
        v->cell[i] = lval_eval(e, v->cell[i]);
    }

    for (int i = 0; i< v->count; i++){
        if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
    }

    if (v->count == 0){ return v; }

    if (v->count == 1){ return lval_take(v, 0); }

    lval *f = lval_pop(v, 0);
    if (f->type != LVAL_FUNC){
        lval *err = lval_err("S-Expression starts with incorrect type. "
                             "Expected %s, got %s",
                             ltype_name(LVAL_FUNC), ltype_name(f->type));
        lval_del(f);
        lval_del(v);
        return err;
    }

    lval *result = lval_call(e, f, v);
    lval_del(f);
    return result;
}

lval *lval_eval(lenv *e, lval *v){
    if (v->type == LVAL_SYM){
        lval *x = lenv_get(e, v);
        lval_del(v);
        return x;
    }
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
    return v;
}
