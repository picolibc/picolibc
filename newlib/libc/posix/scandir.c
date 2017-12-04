#ifndef HAVE_OPENDIR

/*
 * Copyright (c) 1983 Regents of the University of California.
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
 * 4. Neither the name of the University nor the names of its contributors
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)scandir.c	5.10 (Berkeley) 2/23/91";
#endif /* LIBC_SCCS and not lint */

/*
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct dirent (through namelist). Returns -1 if there were any errors.
 */

#include <sys/types.h>
#include <sys/stat.h>
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

#ifndef __P
#define __P(args) ()
#endif

int
scandir (const char *dirname,
	struct dirent ***namelist,
	int (*select) __P((const struct dirent *)),
	int (*dcomp) __P((const struct dirent **, const struct dirent **)))
{
	register struct dirent *d, *p, **names;
	register size_t nitems;
	struct stat stb;
	long arraysz;
	DIR *dirp;
	int successful = 0;
	int rc = 0;

	dirp = NULL;
	names = NULL;

	if ((dirp = opendir(dirname)) == NULL)
		return(-1);
#ifdef HAVE_DD_LOCK
	__lock_acquire_recursive(dirp->dd_lock);
#endif
	if (fstat(dirp->dd_fd, &stb) < 0)
		goto cleanup;

	/*
 	 * If there were no directory entries, then bail.
 	 */
	if (stb.st_size == 0)
		goto cleanup;

	/*
	 * estimate the array size by taking the size of the directory file
	 * and dividing it by a multiple of the minimum size entry. 
	 */
	arraysz = (stb.st_size / 24);
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		goto cleanup;

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !(*select)(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			goto cleanup;
		p->d_ino = d->d_ino;
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
		if (++nitems >= arraysz) {
			if (fstat(dirp->dd_fd, &stb) < 0)
				goto cleanup;
			arraysz = stb.st_size / 12;
			names = (struct dirent **)reallocf((char *)names,
				arraysz * sizeof(struct dirent *));
			if (names == NULL)
				goto cleanup;
		}
		names[nitems-1] = p;
	}
	successful = 1;
cleanup:
	closedir(dirp);
	if (successful) {
		if (nitems && dcomp != NULL)
			qsort(names, nitems, sizeof(struct dirent *), (void *)dcomp);
		*namelist = names;
		rc = nitems;
	} else {  /* We were unsuccessful, clean up storage and return -1.  */
		if ( names ) {
			int i;
			for (i=0; i < nitems; i++ )
				free( names[i] );
			free( names );
		}
		rc = -1;
	}

#ifdef HAVE_DD_LOCK
	__lock_release_recursive(dirp->dd_lock);
#endif
	return(rc);
}

/*
 * Alphabetic order comparison routine for those who want it.
 */
int
alphasort (const struct dirent **d1,
       const struct dirent **d2)
{
       return(strcmp((*d1)->d_name, (*d2)->d_name));
}

#endif /* ! HAVE_OPENDIR */
