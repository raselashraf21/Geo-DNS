
#include "utility/__assert.h"

#include <stdio.h>


int test_assert(){
    /*
     usage:
        To check if 'foo' is zero, use:      AZ(foo)
        To check if 'foo' is not zero, use:      AN(foo)
        To check if 'foo' is NULL, use:      ANULL(foo)
        To check if 'foo' is not NULL, use:      NNULL(foo)
        To check if 'foo' is greater than 'bar',  use:      AG(foo, bar)
        To check if 'foo' is less than 'bar',  use:      AL(foo, bar)
        To check if 'foo' is greater than/ equal to  'bar',  use:    AGE(foo, bar)
        To check if 'foo' is less than/ equal to  'bar',  use:    ALE(foo, bar)
      */
    // AZ(0);
    // AZ(1);
    // AN(1);
    // AN(0);
    // ANULL(NULL);
    // ANULL("a");
    // NNULL("a");
    // NNULL(NULL);
    
    // do { comp_op c= EQ; xassert(a, c, NULL); } while (0);
    // AL(1,0);
    return 0;
}
