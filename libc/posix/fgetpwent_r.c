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
#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

enum pw_id {
    pw_name,
    pw_passwd,
    pw_uid,
    pw_gid,
    pw_gecos,
    pw_dir,
    pw_shell,
    pw_count,
};

static const ptrdiff_t pw_offset[] = {
    [pw_name] = __offsetof(struct passwd, pw_name),
    [pw_passwd] = -1,
    [pw_uid] = __offsetof(struct passwd, pw_uid),
    [pw_gid] = __offsetof(struct passwd, pw_gid),
    [pw_gecos] = -1,
    [pw_dir] = __offsetof(struct passwd, pw_dir),
    [pw_shell] = __offsetof(struct passwd, pw_shell),
};

static inline bool
pw_is_numeric(enum pw_id state)
{
    return state == pw_uid || state == pw_gid;
}

int
fgetpwent_r(FILE *stream, struct passwd *pwbuf, char *buf, size_t size, struct passwd **pwbufp)
{
    char      *line;
    char      *nl;
    char      *token;
    enum pw_id id;
    ptrdiff_t  offset;
    void      *ptr;

    errno = 0;
    line = fgets(buf, size, stream);
    if (!line) {
        *pwbufp = NULL;
        if (errno == 0)
            errno = ENOENT;
        return errno;
    }
    if ((nl = strchr(line, '\n')) != NULL)
        *nl = '\0';
    id = pw_name;

    /*
     * Use strsep instead of strtok_r because strsep permits empty
     * fields
     */
    while ((token = strsep(&line, ":")) != NULL) {
        if (id < pw_count) {
            offset = pw_offset[id];
            if (offset != -1) {
                ptr = (char *)pwbuf + offset;
                if (pw_is_numeric(id)) {
                    char *num_end;
                    uid_t uid = strtol(token, &num_end, 10);
                    if (*num_end != '\0')
                        break;
                    *((uid_t *)ptr) = uid;
                } else {
                    *((char **)ptr) = token;
                }
            }
            id++;
        }
    }
    if (id != pw_count)
        return (errno = EINVAL);
    *pwbufp = pwbuf;
    return 0;
}
