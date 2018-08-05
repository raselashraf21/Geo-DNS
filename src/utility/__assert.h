
#ifndef ASSERT_PROJECT_CUSTOM_ASSERT_H
#define ASSERT_PROJECT_CUSTOM_ASSERT_H

#endif //ASSERT_PROJECT_CUSTOM_ASSERT_H

#include <assert.h>
#include <stdio.h>


typedef enum {
    EQ,
    NE,
    GT,
    LT,
    GE,
    LE
} comp_op;

#define xassert(a,c,b)	                \
do {		                            \
    switch(c){                          \
        case EQ:                        \
            assert(a == b);             \
            break;                      \
        case NE:                        \
            assert(a != b);             \
            break;                      \
        case GT:                        \
            assert(a > b);              \
            break;                      \
        case LT:                        \
            assert(a < b);              \
            break;                      \
        case GE:                        \
            assert(a >= b);             \
            break;                      \
        case LE:                        \
            assert(a <= b);             \
            break;                      \
    }                                   \
} while (0)                             \


#define AZ(foo)		do { comp_op c= EQ; xassert(foo, c, 0); } while (0)
#define AN(foo)		do { comp_op c= NE; xassert(foo, c, 0); } while (0)
#define ANULL(foo)		do { comp_op c= EQ; xassert(foo, c, '\0'); } while (0)
#define NNULL(foo)		do { comp_op c= NE; xassert(foo, c, '\0'); } while (0)
#define AG(foo, bar)		do { comp_op c= GT; xassert(foo, c, bar); } while (0)
#define AL(foo, bar)		do { comp_op c= LT; xassert(foo, c, bar); } while (0)
#define AGE(foo, bar)		do { comp_op c= GE; xassert(foo, c, bar); } while (0)
#define ALE(foo, bar)		do { comp_op c= LE; xassert(foo, c, bar); } while (0)


