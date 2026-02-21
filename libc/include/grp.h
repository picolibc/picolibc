/*	$NetBSD: grp.h,v 1.7 1995/04/29 05:30:40 cgd Exp $	*/

/*-
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)grp.h	8.2 (Berkeley) 1/21/94
 */

#ifndef _GRP_H_
#define _GRP_H_

#include <sys/cdefs.h>
#define __need_size_t
#include <stddef.h>
#include <sys/_types.h>

#if __BSD_VISIBLE
#define _PATH_GROUP "/etc/group"
#endif

_BEGIN_STD_C

#ifndef ___FILE_DECLARED
typedef struct __file __FILE;
#define ___FILE_DECLARED
#endif

#ifndef _GID_T_DECLARED
typedef __gid_t gid_t; /* group id */
#define _GID_T_DECLARED
#endif

struct group {
    char  *gr_name; /* group name */
    gid_t  gr_gid;  /* group id */
    char **gr_mem;  /* group members */
};

struct group *getgrgid(gid_t);
struct group *getgrnam(const char *);

#if __MISC_VISIBLE || __POSIX_VISIBLE
int getgrnam_r(const char *, struct group *, char *, size_t, struct group **);
int getgrgid_r(gid_t, struct group *, char *, size_t, struct group **);
#endif /* __MISC_VISIBLE || __POSIX_VISIBLE */

#if __MISC_VISIBLE
int           getgrent_r(struct group           *__restrict gbuf, char           *__restrict buf, size_t size,
                         struct group           **__restrict gbufp);
struct group *fgetgrent(__FILE *stream);
int fgetgrent_r(__FILE * __restrict stream, struct group * __restrict gbuf, char * __restrict buf,
                size_t size, struct group ** __restrict gbufp);
#endif

#if __MISC_VISIBLE || __XSI_VISIBLE >= 4
struct group *getgrent(void);
void          setgrent(void);
void          endgrent(void);
#endif /* __MISC_VISIBLE || __XSI_VISIBLE >= 4 */

#if __BSD_VISIBLE
int initgroups(const char *, gid_t);
#endif /* __BSD_VISIBLE */

_END_STD_C

#endif /* !_GRP_H_ */
