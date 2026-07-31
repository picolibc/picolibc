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
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
int
unsetenv(const char *name)
{
    char **env;
    size_t offset;

    /* Name cannot be NULL, empty, or contain an equal sign.  */
    if (name == NULL || name[0] == '\0' || strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }

    __LIBC_LOCK();

    while (__findenv(name, &offset)) /* if set multiple times */
    {
        for (env = &(environ)[offset];; ++env)
            if (!(*env = *(env + 1)))
                break;
    }

    ++__environ_sequence;

    __LIBC_UNLOCK();
    return 0;
}
