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
#include <fcntl.h>
#include <ndbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ndbm: O_WRONLY opens a database for writing; a database of a few
 * hundred records, enough to expand the hash table, keeps every record,
 * also after reopening; dbm_firstkey and dbm_nextkey return each key
 * once, and dbm_nextkey stays at the end.
 */

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "NDBM.TXT"
#endif

#define DB_WR  "W" TEST_FILE_NAME
#define DB_BIG "B" TEST_FILE_NAME

static int errors;

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            errors++;                                  \
        }                                              \
    } while (0)

static void
cleanup(void)
{
    (void)remove(DB_WR ".db");
    (void)remove(DB_BIG ".db");
}

static datum
mem_datum(const void *p, size_t len)
{
    datum d;

    d.dptr = (void *)p;
    d.dsize = len;
    return d;
}

static datum
str_datum(const char *s)
{
    return mem_datum(s, strlen(s));
}

static int
fetch_is(DBM *db, datum key, const void *value, size_t len)
{
    datum d = dbm_fetch(db, key);

    return d.dptr != NULL && d.dsize == len && memcmp(d.dptr, value, len) == 0;
}

/* More than the 81 records which first expand a new table */
#define NREC    500
#define REC_LEN 40

static void
make_content(unsigned char *buf, int i)
{
    int j;

    for (j = 0; j < REC_LEN; j++)
        buf[j] = (unsigned char)(i * 31 + j * 7);
}

/* Fetches every record; returns how many were wrong */
static int
fetch_all(DBM *db)
{
    unsigned char content[REC_LEN];
    char          kbuf[32];
    int           i, bad = 0;

    for (i = 0; i < NREC; i++) {
        snprintf(kbuf, sizeof(kbuf), "key%d", i);
        make_content(content, i);
        if (!fetch_is(db, str_datum(kbuf), content, REC_LEN))
            bad++;
    }
    return bad;
}

/* Walks the keys: each must be one of ours, seen once; returns the count or -1 */
static int
walk_keys(DBM *db)
{
    static unsigned char seen[NREC];
    datum                key;
    char                 buf[32];
    int                  n = 0, i;

    memset(seen, 0, sizeof(seen));
    for (key = dbm_firstkey(db); key.dptr != NULL; key = dbm_nextkey(db)) {
        if (key.dsize < 4 || key.dsize >= sizeof(buf))
            return -1;
        memcpy(buf, key.dptr, key.dsize);
        buf[key.dsize] = '\0';
        if (strncmp(buf, "key", 3) != 0)
            return -1;
        i = atoi(buf + 3);
        if (i < 0 || i >= NREC || seen[i])
            return -1;
        seen[i] = 1;
        if (++n > NREC)
            return -1;
    }
    return n;
}

int
main(void)
{
    DBM          *db;
    unsigned char content[REC_LEN];
    char          kbuf[32];
    int           i, bad;

#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif

    cleanup();

    /* Enough records to expand the table */
    db = dbm_open(DB_BIG, O_RDWR | O_CREAT, 0600);
    if (db == NULL) {
        /* Semihosting lacks some of the file operations the hash code uses */
        printf("dbm_open failed (errno %d), skipping\n", errno);
        cleanup();
        return 77;
    }
    bad = 0;
    for (i = 0; i < NREC; i++) {
        snprintf(kbuf, sizeof(kbuf), "key%d", i);
        make_content(content, i);
        if (dbm_store(db, str_datum(kbuf), mem_datum(content, REC_LEN), DBM_INSERT) != 0)
            bad++;
    }
    check(bad == 0, "store many");
    check(fetch_all(db) == 0, "fetch many");
    check(dbm_error(db) == 0, "no error after storing many");
    dbm_close(db);

    /* Still there after reopening */
    db = dbm_open(DB_BIG, O_RDONLY, 0);
    check(db != NULL, "reopen");
    if (db) {
        check(fetch_all(db) == 0, "fetch after reopening");

        /* Each key once, and then dbm_nextkey stays at the end */
        check(walk_keys(db) == NREC, "walk the keys");
        check(dbm_nextkey(db).dptr == NULL, "past the end");
        check(dbm_nextkey(db).dptr == NULL, "still past the end");
        dbm_close(db);
    }

    /* O_WRONLY opens the database for writing */
    db = dbm_open(DB_WR, O_WRONLY | O_CREAT, 0600);
    check(db != NULL, "dbm_open O_WRONLY");
    if (db) {
        check(dbm_store(db, str_datum("w"), str_datum("3"), DBM_INSERT) == 0, "store O_WRONLY");
        dbm_close(db);
    }
    db = dbm_open(DB_WR, O_RDONLY, 0);
    check(db != NULL && fetch_is(db, str_datum("w"), "3", 1), "after O_WRONLY");
    if (db)
        dbm_close(db);

    cleanup();
    if (errors) {
        printf("%d errors\n", errors);
        return 1;
    }
    printf("success\n");
    return 0;
}
