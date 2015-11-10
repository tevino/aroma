#include <stdlib.h>
#include <string.h>
#include "eval.h"
#include "builtins.h"

lenv *lenv_new(){
    lenv *e = malloc(sizeof(lenv));
    e->parent = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

lenv *lenv_copy(lenv *e){
    lenv *n = malloc(sizeof(lenv));
    n->parent = e->parent;
    n->count = e->count;
    n->syms = malloc(sizeof(char*)  *n->count);
    n->vals = malloc(sizeof(lval*)  *n->count);

    for (int i = 0; i < n->count; i++){
        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }
    return n;
}

void lenv_del(lenv *e){
    for (int i = 0; i < e->count; i++){
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}


lval *lenv_get(lenv *e, lval *k){
    for (int i = 0; i < e->count; i++){
        if (strcmp(e->syms[i], k->sym) == 0){
            return lval_copy(e->vals[i]);
        }
    }

    if (e->parent){
        return lenv_get(e->parent, k);
    }

    return lval_err("Unbound Symbol '%s'", k->sym);
}

void lenv_set(lenv *e, lval *k, lval *v){
    /* key exists */
    for (int i = 0; i < e->count; i++){
        if (strcmp(e->syms[i], k->sym) == 0){
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    e->count++;

    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(lval*) * e->count);

    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count - 1], k->sym);
}

void lenv_setg(lenv *e, lval *k, lval *v){
    while (e->parent){
        e = e->parent;
    }

    lenv_set(e, k, v);
}

void lenv_add_builtin(lenv *e, char *name, lbuiltin func){
    lval *k = lval_sym(name);
    lval *v = lval_func(func);
    lenv_set(e, k, v);
    lval_del(k);
    lval_del(v);
}

void lenv_add_builtins(lenv *e){
    lenv_add_builtin(e, "set", builtin_set);
    lenv_add_builtin(e, "setg", builtin_setg);
    lenv_add_builtin(e, "lambda", builtin_lambda);

    lenv_add_builtin(e, "car", builtin_car);
    lenv_add_builtin(e, "cdr", builtin_cdr);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "eval", builtin_eval);

    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);

    lenv_add_builtin(e, ">", builtin_gt);
    lenv_add_builtin(e, "<", builtin_lt);
    lenv_add_builtin(e, ">=", builtin_ge);
    lenv_add_builtin(e, "<=", builtin_le);
    lenv_add_builtin(e, "==", builtin_eq);
    lenv_add_builtin(e, "!=", builtin_ne);

    lenv_add_builtin(e, "if", builtin_if);
    lenv_add_builtin(e, "load", builtin_load);
    lenv_add_builtin(e, "print", builtin_print);
    lenv_add_builtin(e, "error", builtin_error);

    /* Add consts */
    lenv_setg(e, lval_sym("t"), lval_t());
    lenv_setg(e, lval_sym("nil"), lval_nil());
}

