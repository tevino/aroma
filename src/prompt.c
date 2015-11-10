#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <editline/readline.h>
#include "types.h"
#include "eval.h"
#include "builtins.h"
#include "parsing.h"
#include "unused.h"

const char STD_LIB[] = "libs/std.aroma";

int main(int argc, char* *argv){
    init_parser();
    lenv *e = lenv_new();
    lenv_add_builtins(e);
    if (access(STD_LIB, F_OK) != -1){
        lval *err = builtin_load(e, lval_add(lval_sexpr(), lval_str((char *)STD_LIB)));
        if (err->type == LVAL_ERR) lval_println(err);
        lval_del(err);
    } else {
        printf("Can't find stdlib at %s.\n", STD_LIB);
    }

    if (argc > 1){
        for (int i = 1; i < argc; i++){
            lval *args = lval_add(lval_sexpr(), lval_str(argv[i]));
            lval *x = builtin_load(e, args);

            if (x->type == LVAL_ERR) lval_println(x);
            lval_del(x);
        }
    } else {
        puts("Aroma Version v0.0.0.1");
        puts("Press Ctrl+C to Exit.");

        char *input = NULL;
        mpc_result_t r;

        while (true){
            input = readline(">>> ");
            if (strlen(input) < 1) {
                continue;
            }
            add_history(input);
            if (mpc_parse("<stdin>", input, Lispy, &r)){
                /* mpc_ast_print(r.output); */
                lval *x = lval_eval(e, lval_read(r.output));
                lval_println(x);
                lval_del(x);

                mpc_ast_delete(r.output);
            } else {
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
            }
            /* printf("%s\n", input); */
            free(input);
        }
    }

    lenv_del(e);
    cleanup_parser();

    return 0;
}
