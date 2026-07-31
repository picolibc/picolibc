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
#include <stdio.h>
#include <grp.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

static const char         grp_contents[] = "root:x:0:\n"
                                           "daemon:x:1:daemon\n"
                                           "bin:x:2:\n"
                                           "sys:x:3:user,bin,sys\n"
                                           "adm:x:4:user\n";

static const char * const empty_group[] = { NULL };

static const char * const daemon_group[] = { "daemon", NULL };

static const char * const sys_group[] = { "user", "bin", "sys", NULL };

static const char * const adm_group[] = { "user", NULL };

static const struct group expect[] = {
    {
     .gr_name = "root",
     .gr_gid = 0,
     .gr_mem = (char **)empty_group,
     },
    {
     .gr_name = "daemon",
     .gr_gid = 1,
     .gr_mem = (char **)daemon_group,
     },
    {
     .gr_name = "bin",
     .gr_gid = 2,
     .gr_mem = (char **)empty_group,
     },
    {
     .gr_name = "sys",
     .gr_gid = 3,
     .gr_mem = (char **)sys_group,
     },
    { .gr_name = "adm", .gr_gid = 4, .gr_mem = (char **)adm_group }
};

#define NEXPECT (sizeof(expect) / sizeof(expect[0]))

static char buf[BUFSIZ];

static int
same_mem(char **a, char **b)
{
    while (*a && *b) {
        if (strcmp(*a, *b) != 0)
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

int
main(void)
{
    struct group grp, *grp_ret;
    FILE        *f;
    size_t       e, m;
    int          ret = 0;

    f = fmemopen((void *)grp_contents, sizeof(grp_contents), "r");
    if (!f) {
        printf("cannot fmemopen contents\n");
        return 1;
    }

    e = 0;
    while (fgetgrent_r(f, &grp, buf, BUFSIZ, &grp_ret) == 0) {
        if (strcmp(grp_ret->gr_name, expect[e].gr_name) != 0 || grp_ret->gr_gid != expect[e].gr_gid
            || !same_mem(grp_ret->gr_mem, expect[e].gr_mem)) {
            printf("Entry %zd incorrect:\n", e);
            printf("    name: got '%s' want '%s'\n", grp_ret->gr_name, expect[e].gr_name);
            printf("    gid:  got %ld want %ld\n", (long)grp_ret->gr_gid, (long)expect[e].gr_gid);
            printf("    mem:  got { ");
            for (m = 0; grp_ret->gr_mem[m]; m++)
                printf("%s'%s'", m ? ", " : "", grp_ret->gr_mem[m]);
            printf(" } want { ");
            for (m = 0; expect[e].gr_mem[m]; m++)
                printf("%s'%s'", m ? " ," : "", expect[e].gr_mem[m]);
            printf(" }\n");
            ret = 1;
        }
        e++;
    }
    fclose(f);

#ifdef TESTS_ENABLE_POSIX_IO
#define ROOT_NAME "root"
#define ROOT_GID  0

    grp_ret = getgrgid(ROOT_GID);
    if (grp_ret) {
        if (grp_ret->gr_gid != ROOT_GID) {
            printf("Entry for gid %ld got %ld\n", (long)ROOT_GID, (long)grp_ret->gr_gid);
            ret = 1;
        }
    } else {
        perror("Cannot get entry for root GID");
    }

    grp_ret = getgrnam(ROOT_NAME);
    if (grp_ret) {
        if (grp_ret->gr_gid != ROOT_GID || strcmp(grp_ret->gr_name, ROOT_NAME) != 0) {
            printf("Entry for '%s' got '%s' %ld\n", ROOT_NAME, grp_ret->gr_name,
                   (long)grp_ret->gr_gid);
            ret = 1;
        }
    } else {
        perror("Cannot get entry for GID 0");
    }

    grp_ret = NULL;
    bool  found_root = false;
    bool  found_max = false;
    gid_t max_gid = 0;

    while ((grp_ret = getgrent()) != NULL) {
        if (grp_ret->gr_gid == 0)
            found_root = true;
        if (grp_ret->gr_gid > max_gid)
            max_gid = grp_ret->gr_gid;
    }

    setgrent();

    while ((grp_ret = getgrent()) != NULL) {
        if (grp_ret->gr_gid == max_gid)
            found_max = true;
    }

    if (!found_root) {
        printf("getgrent didn't find an entry with GID 0\n");
        ret = 1;
    }

    if (!found_max) {
        printf("getgrent didn't find an entry with GID %ld a second time\n", (long)max_gid);
        ret = 1;
    }

    endgrent();

#endif

    return ret;
}
