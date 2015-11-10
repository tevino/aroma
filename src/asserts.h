#ifndef ASSERTS_H
#define ASSERTS_H

#define LASSERT(args, cond, fmt, ...) \
    if (!(cond)) { \
        lval *err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args); \
        return err; \
    }

#define LASSERT_NUM(func, args, num) \
    LASSERT(args, args->count == num, \
            "Function '%s' Expected %d arguments, got %d", \
            func, num, args->count);

#define LASSERT_TYPE(func, args, i, t) \
    LASSERT(args, args->cell[i]->type == t, \
            "Function '%s' Expected %s at index %d, got %s", \
            func, ltype_name(t), i, ltype_name(args->cell[i]->type));

#define LASSERT_NOT_EMPTY(func, args, i) \
    LASSERT(args, args->cell[i]->count != 0, \
            "Function '%s' passed {} at index %d", func, i);
#endif
