/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE
#include <errno.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../test-malloc-used.h"

/*
 * hsearch manages one hash table: ENTER adds an item that isn't there
 * and returns the entry, keeping the first data for a key; FIND only
 * looks, with strcmp. hdestroy disposes of the table, entries
 * included (checked with mallinfo on picolibc), after which hcreate
 * makes a new, empty one.
 */

static int errors;

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            errors++;                                  \
        }                                              \
    } while (0)

#define NKEYS 100

static char keys[NKEYS][8];

static ENTRY *
lookup(const char *key, ACTION action, void *data)
{
    char  buf[sizeof(keys[0])];
    ENTRY item;

    /* A copy of the key: the comparison is strcmp, not the pointer */
    strcpy(buf, key);
    item.key = action == FIND ? buf : (char *)key;
    item.data = data;
    return hsearch(item, action);
}

/* Fills a new table and destroys it */
static void
cycle(void)
{
    int i;

    check(hcreate(NKEYS * 2) != 0, "hcreate in a cycle");
    for (i = 0; i < NKEYS; i++)
        if (lookup(keys[i], ENTER, &keys[i]) == NULL) {
            check(0, "ENTER in a cycle");
            break;
        }
    hdestroy();
}

int
main(void)
{
    int    i;
    ENTRY *e, *e2;
    char   other[] = "k1";

    for (i = 0; i < NKEYS; i++)
        snprintf(keys[i], sizeof(keys[i]), "k%d", i);

    check(hcreate(NKEYS * 2) != 0, "hcreate");

    /* ENTER returns the new entry, holding the item's key and data */
    for (i = 0; i < NKEYS; i++) {
        e = lookup(keys[i], ENTER, &keys[i]);
        check(e != NULL && strcmp(e->key, keys[i]) == 0 && e->data == &keys[i], "ENTER new");
    }

    /* FIND compares with strcmp, so a copy of the key finds the entry */
    for (i = 0; i < NKEYS; i++) {
        e = lookup(keys[i], FIND, NULL);
        check(e != NULL && strcmp(e->key, keys[i]) == 0 && e->data == &keys[i], "FIND");
    }

    /* The entry stays where it is: the same pointer each time */
    e = lookup("k7", FIND, NULL);
    e2 = lookup("k7", FIND, NULL);
    check(e != NULL && e == e2, "same entry");

    /* ENTER of a key that's there finds it and doesn't replace its data */
    e = lookup(other, ENTER, other);
    check(e != NULL && e->data == &keys[1] && e->key == keys[1], "ENTER existing");

    /* Keys that aren't there */
    check(lookup("k", FIND, NULL) == NULL, "missing prefix");
    check(lookup("k10x", FIND, NULL) == NULL, "missing extension");
    check(lookup("zz", FIND, NULL) == NULL && lookup("zz", FIND, NULL) == NULL,
          "FIND adds nothing");

    /* hdestroy disposes of the table; hcreate makes a new, empty one */
    hdestroy();
    check(hcreate(4) != 0, "hcreate after hdestroy");
    for (i = 0; i < NKEYS; i++)
        check(lookup(keys[i], FIND, NULL) == NULL, "empty after hdestroy");
    hdestroy();

    /* hdestroy frees the entries as well as the table */
    {
        size_t before, after;

        cycle();
        before = malloc_used();
        for (i = 0; i < 10; i++)
            cycle();
        after = malloc_used();
        if (before != (size_t)-1 && after > before) {
            printf("%zu bytes in use before, %zu after\n", before, after);
            check(0, "hdestroy frees the entries");
        }
    }

    if (errors) {
        printf("%d errors\n", errors);
        return 1;
    }
    printf("success\n");
    return 0;
}
