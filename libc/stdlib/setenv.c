/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution.
 * Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#define _DEFAULT_SOURCE
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/lock.h>
#include "local.h"

/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */

extern char **environ;
static bool   setenv_alloced; /* if allocated space before */

int
setenv(const char *name, const char *value, int rewrite)
{
    size_t   offset;
    char    *cur_entry;
    char   **new_environ;
    char    *new_entry;
    uint32_t old_sequence;
    size_t   l_value, l_name;

    /* Name cannot be NULL, empty, or contain an equal sign.  */
    if (name == NULL || name[0] == '\0' || strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }

    l_value = strlen(value);

    /*
     * Loop until we manage to perform all operations without being
     * interrupted by another thread
     */
    for (;;) {

        new_environ = NULL;
        new_entry = NULL;

        __LIBC_LOCK();

        /* Record current sequence value */
        old_sequence = __environ_sequence;

        if ((cur_entry = __findenv(name, &offset))) { /* find if already exists */
            if (!rewrite) {
                __LIBC_UNLOCK();
                return 0;
            }
            if (strlen(cur_entry) >= l_value) { /* old larger; copy over */
                strcpy(cur_entry, value);
                __LIBC_UNLOCK();
                return 0;
            }
        } else { /* create new slot */
            size_t n_environ = 0;
            char **env;

            for (env = environ; *env; ++env)
                n_environ++;

            __LIBC_UNLOCK();

            new_environ = calloc(n_environ + 2, sizeof(char *));
            if (!new_environ)
                return -1;

            __LIBC_LOCK();
            if (__environ_sequence != old_sequence)
                goto retry;

            memcpy(new_environ, environ, n_environ * sizeof(char *));
            new_environ[n_environ + 1] = NULL;
            offset = n_environ;
        }

        __LIBC_UNLOCK();

        l_name = strlen(name);

        new_entry = malloc(l_name + 1 + l_value + 1);

        if (!new_entry) {
            free(new_environ);
            return -1;
        }

        char *eq = stpcpy(new_entry, name);
        *eq++ = '=';
        strcpy(eq, value);

        __LIBC_LOCK();

        if (__environ_sequence == old_sequence)
            break;

    retry:
        __LIBC_UNLOCK();
        free(new_environ);
        free(new_entry);
    }

    /* note change in environment */
    __environ_sequence++;

    if (new_environ) {
        if (setenv_alloced)
            free(environ);
        environ = new_environ;
        setenv_alloced = true;
    }

    environ[offset] = new_entry;

    __LIBC_UNLOCK();

    return 0;
}
