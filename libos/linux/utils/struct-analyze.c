#include <stddef.h>
#include "struct-data.h"
#include <stdio.h>
#include <stdint.h>

#define IS_SIGNED  1
#define IS_POINTER 2

int
main(int argc, char **argv)
{
    size_t            i;
    size_t            offset = 0;
    size_t            min_offset;
    size_t            min_i;
    size_t            nelt;
    const struct elt *e;

    if (argc > 1) {
        FILE *typefile = fopen(argv[1], "r");
        int   c;
        if (!typefile) {
            perror(argv[1]);
            return 1;
        }
        while ((c = getc(typefile)) != EOF)
            putchar(c);
        fclose(typefile);
    }

    printf("\n");

    printf("struct __kernel_%s {\n", name);

    for (;;) {
        min_offset = SIZE_MAX;
        min_i = SIZE_MAX;
        for (i = 0; i < sizeof(elts) / sizeof(elts[0]); i++) {
            e = &elts[i];
            if (e->offset >= offset && e->offset - offset < min_offset) {
                min_offset = e->offset - offset;
                min_i = i;
            }
        }

        if (min_i == SIZE_MAX)
            break;

        e = &elts[min_i];

        if (e->offset > offset) {
            printf("    __uint8_t  __adjust_%zd[%zd];\n", offset, e->offset - offset);
            offset = e->offset;
        }

        if (e->flags & IS_POINTER) {
            printf("    void *%s;\n", e->name);
        } else {
            printf("    ");
            if (e->typename)
                printf("%s ", e->typename);
            else {
                nelt = e->nelt ? e->nelt : 1;
                printf("__%sint%zd_t ", (e->flags & IS_SIGNED) ? "" : "u", 8 * (e->size / nelt));
            }
            if (e->nelt > 0) {
                printf("%s[%d]", e->name, e->nelt);
            } else {
                printf("%s", e->name);
            }
            printf(";\n");
        }

        offset += e->size;
    }
    if (offset < sizeof(val))
        printf("    __uint8_t  __adjust_%zd[%zd];\n", offset, sizeof(val) - offset);
    printf("};\n");
    return 0;
}
