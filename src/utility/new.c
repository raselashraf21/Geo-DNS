
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "new.h"

void * P99_FSYMB(new)(const void * _class, P99_VA_ARGS(number))
{
    const struct Class * class = _class;
    void * p = calloc(1, class->size);
    assert(p);
    *((const struct Class **)p) = class;
    if (class->ctor)
    {
        va_list ap;
        va_start(ap, number);
        p = class->ctor(p, number, &ap);
        va_end(ap);
    }
    return p;
}

void delete (void * self)
{
    const struct Class **cp = self;
    if (self && *cp && (*cp)->dtor)
        self = (*cp)->dtor(self);
    free(self);
}
