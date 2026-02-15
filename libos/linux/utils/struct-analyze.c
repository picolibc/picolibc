#include <stddef.h>
#include "struct-data.h"
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#define IS_SIGNED  1
#define IS_POINTER 2
#define IS_USED    4
#define IS_UNION   8

#ifdef MAKE_UNION
#define STRUCT_OR_UNION "union"
#else
#define STRUCT_OR_UNION "struct"
#endif

#define NELT (sizeof(elts) / sizeof(elts[0]))
struct {
    struct elt *ref[NELT];
    size_t      size;
} unions[NELT];

static void
print_elt(struct elt *e, const char *head)
{
    size_t nelt;
    printf("%s", head);
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
}

int
main(int argc, char **argv)
{
    size_t      i, j, r;
    size_t      offset;
    size_t      min_offset;
    size_t      min_i;
    size_t      e_size;
    struct elt *e;

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

    printf("%s __kernel_%s {\n", STRUCT_OR_UNION, name);

#ifndef MAKE_UNION

    offset = 0;

    for (;;) {
        min_offset = SIZE_MAX;
        min_i = SIZE_MAX;
        for (i = 0; i < NELT; i++) {
            e = &elts[i];
            if (e->flags & IS_UNION)
                continue;
            if (e->offset >= offset && e->offset - offset < min_offset) {
                min_offset = e->offset - offset;
                min_i = i;
            }
        }

        if (min_i == SIZE_MAX)
            break;

        i = min_i;

        unions[i].size = elts[i].size;

        elts[i].flags |= IS_UNION;

        r = 0;

        for (;;) {
            bool found_any = false;
            for (j = 0; j < NELT; j++) {
                size_t j_end = elts[j].offset + elts[j].size;
                size_t u_end = elts[i].offset + unions[i].size;

                if (elts[j].flags & IS_UNION)
                    continue;

                if (elts[i].offset <= elts[j].offset && elts[j].offset < u_end) {

                    unions[i].ref[r++] = &elts[j];

                    if (j_end > u_end)
                        unions[i].size = j_end - elts[i].offset;

                    found_any = true;
                    elts[j].flags |= IS_UNION;
                }
            }
            if (!found_any)
                break;
        }

        offset += unions[i].size;
    }

#if 0
    for (i = 0; i < NELT; i++) {
        if (unions[i].ref[0]) {
            printf("// union: %s", elts[i].name);
            for (j = 0; j < NELT; j++) {
                e = unions[i].ref[j];
                if (!e)
                    break;
                if (e->offset == elts[i].offset)
                    printf(" %s", e->name);
                else
                    printf(" %s(+%zd)", e->name, e->offset - elts[i].offset);
            }
            printf("\n");
        }
    }
#endif

#endif

    offset = 0;

    for (;;) {
        min_offset = SIZE_MAX;
        min_i = SIZE_MAX;
        for (i = 0; i < NELT; i++) {
            e = &elts[i];
            if (e->flags & IS_USED)
                continue;
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

        e_size = e->size;

        if (unions[min_i].ref[0]) {
            e_size = unions[min_i].size;
            printf("    union {\n");
            print_elt(e, "    ");
            for (r = 0; r < NELT; r++) {
                struct elt *u_e = unions[min_i].ref[r];
                if (!u_e)
                    break;
                if (u_e->offset == offset)
                    print_elt(u_e, "    ");
                else {
                    printf("        struct {\n");
                    printf("            __uint8_t  __adjust_%s_%zd[%zd];\n", e->name,
                           u_e->offset - offset, u_e->offset - offset);
                    print_elt(u_e, "        ");
                    printf("        };\n");
                }
                u_e->flags |= IS_USED;
            }
            printf("    };\n");
        } else
            print_elt(e, "");

        e->flags |= IS_USED;
#ifndef MAKE_UNION
        offset += e_size;
#endif
    }
    if (offset < sizeof(val))
        printf("    __uint8_t  __adjust_%zd[%zd];\n", offset, sizeof(val) - offset);
    printf("};\n");

    printf("\n");

    printf("#define SIMPLE_MAP_");
    for (i = 0; name[i]; i++)
        printf("%c", toupper(name[i]));
    printf("(_t, _f) do {\\\n");
    for (i = 0; i < NELT; i++) {
        e = &elts[i];
        if (!e->typename)
            printf("        (_t)->%s = (_f)->%s;\\\n", e->name, e->name);
    }
    printf("    } while(0)\n");

    return 0;
}
