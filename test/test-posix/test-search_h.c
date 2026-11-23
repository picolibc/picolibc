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

#define _POSIX_C_SOURCE 200809L
/* <search.h> is an XSI header */
#define _XOPEN_SOURCE 700
#include <search.h>

/*
 * <search.h>: its types, constants and prototypes, checked before any
 * other header, so size_t has to come from <search.h> itself, and the
 * names it mustn't define in strict mode
 */

static size_t size_check __attribute__((unused));

/* ENTRY is struct entry, with char *key and void *data */
static char **
key_check(ENTRY *e)
{
    return &e->key;
}
static void **
data_check(struct entry *e)
{
    return &e->data;
}

/* ACTION and VISIT are enumerations */
static ACTION action_check __attribute__((unused)) = FIND;
static VISIT  visit_check __attribute__((unused)) = leaf;

/* posix_tnode is an alias for void (new in POSIX.1-2024); glibc lacks it */
#ifndef __GLIBC__
static posix_tnode *tnode_check __attribute__((unused));
_Static_assert(_Generic((posix_tnode *)0, void *: 1, default: 0), "posix_tnode is void");
typedef posix_tnode tnode;
#else
typedef void tnode;
#endif

/* The prototypes, with tnode for the nodes */
typedef int         hcreate_fn(size_t);
typedef void        hdestroy_fn(void);
typedef ENTRY      *hsearch_fn(ENTRY, ACTION);
typedef void       *tdelete_fn(const void       *restrict, tnode       **restrict,
                               int (*)(const void *, const void *));
typedef tnode      *tfind_fn(const void *, tnode      *const *, int (*)(const void *, const void *));
typedef tnode      *tsearch_fn(const void *, tnode **, int (*)(const void *, const void *));
typedef void        twalk_fn(const tnode *, void (*)(const tnode *, VISIT, int));

static hcreate_fn  *hcreate_check = hcreate;
static hdestroy_fn *hdestroy_check = hdestroy;
static hsearch_fn  *hsearch_check = hsearch;
static tdelete_fn  *tdelete_check = tdelete;
static tfind_fn    *tfind_check = tfind;
static tsearch_fn  *tsearch_check = tsearch;
static twalk_fn    *twalk_check = twalk;

/*
 * <search.h> reserves no names, so the BSD and GNU extensions must be
 * hidden: each of these would clash with a declaration
 */
static int          hcreate_r __attribute__((unused));
static int          hdestroy_r __attribute__((unused));
static int          hsearch_r __attribute__((unused));
static int          tdestroy __attribute__((unused));
struct hsearch_data {
    int x;
};
static struct hsearch_data hsearch_data_check __attribute__((unused));

/* Nor may it bring in <stdint.h> or the rest of <sys/types.h> */
#ifdef INT8_MAX
#error "<search.h> defines INT8_MAX"
#endif
#ifdef SIZE_MAX
#error "<search.h> defines SIZE_MAX"
#endif
#ifdef FD_SETSIZE
#error "<search.h> defines FD_SETSIZE"
#endif

#include <stdio.h>

int
main(void)
{
    int   errors = 0;
    ENTRY e;

    if (key_check(&e) != &e.key || data_check(&e) != &e.data)
        errors++;
    if (!hcreate_check || !hdestroy_check || !hsearch_check || !tdelete_check || !tfind_check
        || !tsearch_check || !twalk_check)
        errors++;

    if (errors) {
        printf("%d errors\n", errors);
        return 1;
    }
    printf("success\n");
    return 0;
}
