#ifndef PARSING_H
#define PARSING_H

#include "mpc.h"
#include "types.h"

mpc_parser_t *Number;
mpc_parser_t *Symbol;
mpc_parser_t *String;
mpc_parser_t *Comment;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *Expr;
mpc_parser_t *Lispy;

void init_parser();
void cleanup_parser();
void lval_expr_print(lval *v, char open, char close);
void lval_print_str(lval *v);
void lval_print(lval *v);
void lval_println(lval *v);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read_str(mpc_ast_t *t) ;
lval *lval_read(mpc_ast_t *t);
#endif
