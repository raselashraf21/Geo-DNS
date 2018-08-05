
#ifndef GEODNS_NEW_H
#define GEODNS_NEW_H

#include <stdarg.h>
#include "p99_id.h"

struct Class {
    size_t size;
    void * (* ctor) (void * self, size_t argc, va_list *argv);
    void * (* dtor) (void * self);
//    void * (* clone) (const void * self);
//    int (* differ) (const void * self, const void * b);
};

void * P99_FSYMB(new)(const void * type, P99_VA_ARGS(number));
#define new(A, ...) P99_FSYMB(new)(A, P99_LENGTH_VA_ARG(__VA_ARGS__))

void delete (void * item);

#endif // GEODNS_NEW_H
