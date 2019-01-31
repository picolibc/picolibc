#ifndef HAVE_OPENDIR

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 */

#include <sys/cdefs.h>
__SCCSID("@(#)scandir.c	8.3 (Berkeley) 1/2/94");
__FBSDID("$FreeBSD: head/lib/libc/gen/scandir.c 335898 2018-07-03 17:31:45Z jhb $");

/*
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct dirent (through namelist). Returns -1 if there were any errors.
 */

#include <stddef.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/lock.h>

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct dirent
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#ifdef _DIRENT_HAVE_D_NAMLEN
#define DIRSIZ(dp) \
    (offsetof (struct dirent, d_name) + (((dp)->d_namlen+1 + 3) &~ 3))
#else
#define DIRSIZ(dp) \
    (offsetof (struct dirent, d_name) + ((strlen((dp)->d_name)+1 + 3) &~ 3))
#endif


int
scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(const struct dirent *), int (*dcomp)(const struct dirent **,
	const struct dirent **))
{
	register struct dirent *d, *p, **names = NULL;
	register size_t arraysz, numitems;
	DIR *dirp;

	if ((dirp = opendir(dirname)) == NULL)
		return(-1);
#ifdef HAVE_DD_LOCK
	__lock_acquire_recursive(dirp->dd_lock);
#endif

	numitems = 0;
	arraysz = 32;	/* initial estimate of the array size */
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		goto fail;

	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !(*select)(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			goto fail;
		p->d_ino = d->d_ino;
#ifdef DT_UNKNOWN
		p->d_type = d->d_type;
#endif
		p->d_reclen = d->d_reclen;
#ifdef _DIRENT_HAVE_D_NAMLEN
		p->d_namlen = d->d_namlen;
		bcopy(d->d_name, p->d_name, p->d_namlen + 1);
#else
               strcpy(p->d_name, d->d_name);
#endif
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (numitems >= arraysz) {
			struct dirent **names2;

			names2 = reallocarray(names, arraysz,
			    2 * sizeof(struct dirent *));
			if (names2 == NULL) {
				free(p);
				goto fail;
			}
			names = names2;
			arraysz *= 2;
		}
		names[numitems++] = p;
	}
	closedir(dirp);
	if (numitems && dcomp != NULL)
		qsort(names, numitems, sizeof(struct dirent *), (void *)dcomp);
	*namelist = names;
#ifdef HAVE_DD_LOCK
	__lock_release_recursive(dirp->dd_lock);
#endif
	return (numitems);

fail:
	while (numitems > 0)
		free(names[--numitems]);
	free(names);
	closedir(dirp);
#ifdef HAVE_DD_LOCK
	__lock_release_recursive(dirp->dd_lock);
#endif
	return (-1);
}

/*
 * Alphabetic order comparison routine for those who want it.
 * POSIX 2008 requires that alphasort() uses strcoll().
 */
int
alphasort(const struct dirent **d1, const struct dirent **d2)
{

	return (strcoll((*d1)->d_name, (*d2)->d_name));
}

#endif /* ! HAVE_OPENDIR */
