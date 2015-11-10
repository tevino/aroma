#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "env.h"

/* lval Types */
char *ltype_name(int t){
    switch(t){
        case LVAL_NUM: return "Number";
        case LVAL_SYM: return "Symbol";
        case LVAL_STR: return "String";
        case LVAL_FUNC: return "Function";
        case LVAL_SEXPR: return "S-Expression";
        case LVAL_QEXPR: return "Q-Expression";
        case LVAL_ERR: return "Error";
        default: return "Unknown";
    }
}

lval *lval_func(lbuiltin builtin){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_FUNC;
    v->builtin = builtin;
    return v;
}

lval *lval_lambda(lval *formals, lval *body){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_FUNC;

    v->builtin = NULL;

    v->env = lenv_new();

    v->formals = formals;
    v->body = body;
    return v;
}

lval *lval_num(long x){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval *lval_err(char *fmt, ...){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);
    v->err = malloc(sizeof(char*));
    if (vasprintf(&v->err, fmt, va) == -1){
        printf("ERROR: CAN'T ALLOCT MEMORY!");
        va_end(va);
        exit(1);
        return NULL;
    }
    va_end(va);
    return v;
}

lval *lval_sym(char *s){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

lval *lval_str(char *s){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
    return v;
}

lval *lval_sexpr(){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval *lval_qexpr(){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval *lval_add(lval *v, lval *x){
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*)  *v->count);
    v->cell[v->count-1] = x;
    return v;
}


void lval_del(lval *v){
    switch (v->type){
        case LVAL_NUM: break;
        case LVAL_FUNC:
            if (!v->builtin){
                lenv_del(v->env);
                lval_del(v->formals);
                lval_del(v->body);
            }
            break;

        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_STR: free(v->str); break;

        case LVAL_SEXPR:
        case LVAL_QEXPR:
            for (int i = 0; i < v->count; i++){
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }
    free(v);
}

lval *lval_copy(lval *v){
    lval *x = malloc(sizeof(lval));
    x->type = v->type;

    switch(v->type){
        case LVAL_NUM: x->num = v->num; break;
        case LVAL_FUNC:
            if (v->builtin){
                x->builtin = v->builtin;
            } else {
                x->builtin = NULL;
                x->env = lenv_copy(v->env);
                x->formals = lval_copy(v->formals);
                x->body = lval_copy(v->body);
            }
            break;
        case LVAL_ERR:
            x->err = malloc(strlen(v->err) + 1);
            strcpy(x->err, v->err);
            break;
        case LVAL_SYM:
            x->sym = malloc(strlen(v->sym) + 1);
            strcpy(x->sym, v->sym);
            break;
        case LVAL_STR:
            x->str = malloc(strlen(v->str) + 1);
            strcpy(x->str, v->str);
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            x->count = v->count;
            x->cell = malloc(sizeof(lval*) * x->count);
            for (int i = 0; i < x->count; i++){
                x->cell[i] = lval_copy(v->cell[i]);
            }
            break;
    }

    return x;
}
