#include <stddef.h>
#include <stdio.h>
#include "type-data.h"

#define IS_SIGNED  1
#define IS_POINTER 2

int
main(void)
{
    size_t i;
    for (i = 0; types[i].name != NULL; i++) {
        struct type *t = &types[i];
        printf("typedef __%sint%zd_t %s;\n", (t->flags & IS_SIGNED) ? "" : "u", 8 * t->size,
               t->name);
    }
    return 0;
}
