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

#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "local-grp.h"

enum gr_id {
    gr_name,
    gr_passwd,
    gr_gid,
    gr_mem,
    gr_count,
};

int
fgetgrent_r(FILE *stream, struct group *grbuf, char *buf, size_t size, struct group **grbufp)
{
    char      *line;
    char      *nl;
    char      *token, *mem;
    enum gr_id id;
    char     **gr_memp;
    size_t     n_gr_mem;

    errno = 0;
    line = fgets(buf, size, stream);
    if (!line) {
        *grbufp = NULL;
        if (errno == 0)
            errno = ENOENT;
        return errno;
    }
    nl = strchrnul(line, '\n');
    *nl = '\0';

    /* Use the rest of the buffer for pointers to group members */
    gr_memp = (char **)__align_up(nl + 1, sizeof(*gr_memp));
    n_gr_mem = ((buf + size) - (char *)gr_memp) / sizeof(*gr_memp);

    if (n_gr_mem == 0)
        return (errno = ERANGE);

    id = gr_name;

    /*
     * Use strsep instead of strtok_r because strsep permits empty
     * fields
     */
    while ((token = strsep(&line, ":")) != NULL) {
        if (id < gr_count) {
            char *num_end;
            switch (id) {
            case gr_name:
                grbuf->gr_name = token;
                break;
            case gr_passwd:
                break;
            case gr_gid:
                grbuf->gr_gid = strtol(token, &num_end, 10);
                if (*num_end != '\0')
                    break;
                break;
            case gr_mem:
                grbuf->gr_mem = gr_memp;
                while ((mem = strsep(&token, ",")) != NULL && n_gr_mem > 1) {
                    *gr_memp++ = mem;
                    n_gr_mem--;
                }
                if (mem)
                    return (errno = ERANGE);
                *gr_memp = NULL;
                break;
            case gr_count:
                break;
            }
            id++;
        }
    }
    if (id == gr_mem) {
        grbuf->gr_mem = gr_memp;
        *gr_memp = NULL;
        id++;
    }
    if (id != gr_count)
        return (errno = EINVAL);
    *grbufp = grbuf;
    return 0;
}
