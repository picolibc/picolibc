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
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

static const char          pwd_contents[] = "root:x:0:0:root:/root:/bin/bash\n"
                                            "sys:x:3:3:sys:/dev:/usr/sbin/nologin\n"
                                            "nohome:x:4:3:::/usr/sbin/nologin\n"
                                            "user:x:1000:4:Sample user account:/home/user:/bin/sh\n";

static const struct passwd expect[] = {
    {
     .pw_name = "root",
     .pw_uid = 0,
     .pw_gid = 0,
     .pw_dir = "/root",
     .pw_shell = "/bin/bash",
     },
    {
     .pw_name = "sys",
     .pw_uid = 3,
     .pw_gid = 3,
     .pw_dir = "/dev",
     .pw_shell = "/usr/sbin/nologin",
     },
    {
     .pw_name = "nohome",
     .pw_uid = 4,
     .pw_gid = 3,
     .pw_dir = "",
     .pw_shell = "/usr/sbin/nologin",
     },
    {
     .pw_name = "user",
     .pw_uid = 1000,
     .pw_gid = 4,
     .pw_dir = "/home/user",
     .pw_shell = "/bin/sh",
     },
};

#define NEXPECT (sizeof(expect) / sizeof(expect[0]))

static char buf[BUFSIZ];

int
main(void)
{
    struct passwd pwd, *pwd_ret;
    FILE         *f;
    size_t        e;
    int           ret = 0;

    f = fmemopen((void *)pwd_contents, sizeof(pwd_contents), "r");
    if (!f) {
        printf("cannot fmemopen contents\n");
        return 1;
    }

    e = 0;
    while (fgetpwent_r(f, &pwd, buf, BUFSIZ, &pwd_ret) == 0) {
        if (strcmp(pwd_ret->pw_name, expect[e].pw_name) != 0 || pwd_ret->pw_uid != expect[e].pw_uid
            || pwd_ret->pw_gid != expect[e].pw_gid || strcmp(pwd_ret->pw_dir, expect[e].pw_dir) != 0
            || strcmp(pwd_ret->pw_shell, expect[e].pw_shell) != 0) {
            printf("Entry %zd incorrect:\n", e);
            printf("    name:  got '%s' want '%s'\n", pwd_ret->pw_name, expect[e].pw_name);
            printf("    uid:   got %ld want %ld\n", (long)pwd_ret->pw_uid, (long)expect[e].pw_uid);
            printf("    gid:   got %ld want %ld\n", (long)pwd_ret->pw_gid, (long)expect[e].pw_gid);
            printf("    dir:   got '%s' want '%s'\n", pwd_ret->pw_dir, expect[e].pw_dir);
            printf("    shell: got '%s' want '%s'\n", pwd_ret->pw_shell, expect[e].pw_shell);
            printf("\n");
            ret = 1;
        }
        e++;
    }
    fclose(f);

#ifdef TESTS_ENABLE_POSIX_IO
#define ROOT_NAME "root"
#define ROOT_UID  0

    pwd_ret = getpwuid(ROOT_UID);
    if (pwd_ret) {
        if (pwd_ret->pw_uid != ROOT_UID) {
            printf("Entry for uid %ld got %ld\n", (long)ROOT_UID, (long)pwd_ret->pw_uid);
            ret = 1;
        }
    } else {
        perror("Cannot get entry for root UID");
    }

    pwd_ret = getpwnam(ROOT_NAME);
    if (pwd_ret) {
        if (pwd_ret->pw_uid != ROOT_UID || strcmp(pwd_ret->pw_name, ROOT_NAME) != 0) {
            printf("Entry for '%s' got '%s' %ld\n", ROOT_NAME, pwd_ret->pw_name,
                   (long)pwd_ret->pw_uid);
            ret = 1;
        }
    } else {
        perror("Cannot get entry for UID 0");
    }

    pwd_ret = NULL;
    bool  found_root = false;
    bool  found_max = false;
    uid_t max_uid = 0;

    while ((pwd_ret = getpwent()) != NULL) {
        if (pwd_ret->pw_uid == 0)
            found_root = true;
        if (pwd_ret->pw_uid > max_uid)
            max_uid = pwd_ret->pw_uid;
    }

    setpwent();

    while ((pwd_ret = getpwent()) != NULL) {
        if (pwd_ret->pw_uid == max_uid)
            found_max = true;
    }

    if (!found_root) {
        printf("getpwent didn't find an entry with UID 0\n");
        ret = 1;
    }

    if (!found_max) {
        printf("getpwent didn't find an entry with UID %ld a second time\n", (long)max_uid);
        ret = 1;
    }

    endpwent();

#endif

    return ret;
}
