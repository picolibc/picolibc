/*-
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)pwd.h	5.13 (Berkeley) 5/28/91
 */

#ifndef _PWD_H_
#define _PWD_H_

#include <sys/cdefs.h>
#define __need_size_t
#include <stddef.h>

_BEGIN_STD_C

#include <sys/_types.h>
#ifndef ___FILE_DECLARED
typedef struct __file __FILE;
#define ___FILE_DECLARED
#endif

#ifndef _GID_T_DECLARED
typedef __gid_t gid_t; /* group id */
#define _GID_T_DECLARED
#endif

#ifndef _UID_T_DECLARED
typedef __uid_t uid_t; /* user id */
#define _UID_T_DECLARED
#endif

#ifndef __PASSWORD_FILENAME
#define __PASSWORD_FILENAME "/etc/passwd"
#endif

struct passwd {
    char *pw_name;  /* user name */
    uid_t pw_uid;   /* user uid */
    gid_t pw_gid;   /* user gid */
    char *pw_dir;   /* home directory */
    char *pw_shell; /* default shell */
};

struct passwd *getpwuid(uid_t uid);

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);

struct passwd *getpwnam(const char *name);

int            getpwnam_r(const char *name, struct passwd *pwd, char *buffer, size_t bufsize,
                          struct passwd **result);

#if __MISC_VISIBLE || __XSI_VISIBLE >= 4
struct passwd *getpwent(void);
void           setpwent(void);
void           endpwent(void);
#endif

#if __MISC_VISIBLE || __SVID_VISIBLE
struct passwd *fgetpwent(__FILE *stream);
int            fgetpwent_r(__FILE *stream, struct passwd *pwbuf, char *buf, size_t size,
                           struct passwd **pwbufp);
#endif

_END_STD_C

#endif /* _PWD_H_ */
