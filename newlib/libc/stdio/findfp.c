/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* No user fns here.  Pesch 15apr92. */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/lock.h>
#include "local.h"

__FILE __sf[3];

struct _glue __sglue = {NULL, 3, &__sf[0]};

__FILE *stdin = &__sf[0];
__FILE *stdout = &__sf[1];
__FILE *stderr = &__sf[2];
void (*_stdio_cleanup)(void);

#ifdef _STDIO_BSD_SEMANTICS
  /* BSD and Glibc systems only flush streams which have been written to
     at exit time.  Calling flush rather than close for speed, as on
     the aforementioned systems. */
#define CLEANUP_FILE __sflushw_r
#else
  /* Otherwise close files and flush read streams, too.
     Note we call flush directly if "--enable-lite-exit" is in effect.  */
#ifdef _LITE_EXIT
#define CLEANUP_FILE fflush
#else
#define CLEANUP_FILE fclose
#endif
#endif

#if (defined (__OPTIMIZE_SIZE__) || defined (__PREFER_SIZE_OVER_SPEED))
__noinline static void
#else
static void
#endif
std (FILE *ptr,
            int flags,
            int file)
{
  ptr->_p = 0;
  ptr->_r = 0;
  ptr->_w = 0;
  ptr->_flags = flags;
  ptr->_flags2 = 0;
  ptr->_file = file;
  ptr->_bf._base = 0;
  ptr->_bf._size = 0;
  ptr->_lbfsize = 0;
  memset (&ptr->_mbstate, 0, sizeof (_mbstate_t));
  ptr->_cookie = ptr;
  ptr->_read = __sread;
#ifndef __LARGE64_FILES
  ptr->_write = __swrite;
#else /* __LARGE64_FILES */
  ptr->_write = __swrite64;
  ptr->_seek64 = __sseek64;
  ptr->_flags |= __SL64;
#endif /* __LARGE64_FILES */
  ptr->_seek = __sseek;
  ptr->_close = __sclose;
  if (ptr == &__sf[0] || ptr == &__sf[1] || ptr == &__sf[2])
    __lock_init_recursive (ptr->_lock);
#ifdef __SCLE
  if (__stextmode (ptr->_file))
    ptr->_flags |= __SCLE;
#endif
}

static inline void
stdin_init(FILE *ptr)
{
  std (ptr,  __SRD, 0);
}

static inline void
stdout_init(FILE *ptr)
{
  /* On platforms that have true file system I/O, we can verify
     whether stdout is an interactive terminal or not, as part of
     __smakebuf on first use of the stream.  For all other platforms,
     we will default to line buffered mode here.  Technically, POSIX
     requires both stdin and stdout to be line-buffered, but tradition
     leaves stdin alone on systems without fcntl.  */
#ifdef __HAVE_FCNTL
  std (ptr, __SWR, 1);
#else
  std (ptr, __SWR | __SLBF, 1);
#endif
}

static inline void
stderr_init(FILE *ptr)
{
  /* POSIX requires stderr to be opened for reading and writing, even
     when the underlying fd 2 is write-only.  */
  std (ptr, __SRW | __SNBF, 2);
}

#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wanalyzer-malloc-leak"
#endif

struct glue_with_files {
  struct _glue glue;
  FILE file[];
};

static struct _glue *
sfmoreglue (int n)
{
  struct glue_with_files *g;

  g = (struct glue_with_files *)
    malloc (sizeof (*g) + n * sizeof (FILE));
  if (g == NULL)
    return NULL;
  g->glue._next = NULL;
  g->glue._niobs = n;
  g->glue._iobs = g->file;
  memset (g->file, 0, n * sizeof (FILE));
  return &g->glue;
}

/*
 * Find a free FILE for fopen et al.
 */

FILE *
__sfp (void)
{
  FILE *fp;
  int n;
  struct _glue *g;

  CHECK_INIT();
  _newlib_sfp_lock_start ();

  for (g = &__sglue;; g = g->_next)
    {
      for (fp = g->_iobs, n = g->_niobs; --n >= 0; fp++)
	if (fp->_flags == 0)
	  goto found;
      if (g->_next == NULL &&
	  (g->_next = sfmoreglue (NDYNAMIC)) == NULL)
	break;
    }
  _newlib_sfp_lock_exit ();
  errno = ENOMEM;
  return NULL;

found:
  fp->_file = -1;		/* no file */
  fp->_flags = 1;		/* reserve this slot; caller sets real flags */
  fp->_flags2 = 0;
  __lock_init_recursive (fp->_lock);
  _newlib_sfp_lock_end ();

  fp->_p = NULL;		/* no current pointer */
  fp->_w = 0;			/* nothing to read or write */
  fp->_r = 0;
  fp->_bf._base = NULL;		/* no buffer */
  fp->_bf._size = 0;
  fp->_lbfsize = 0;		/* not line buffered */
  memset (&fp->_mbstate, 0, sizeof (_mbstate_t));
  /* fp->_cookie = <any>; */	/* caller sets cookie, _read/_write etc */
  fp->_ub._base = NULL;		/* no ungetc buffer */
  fp->_ub._size = 0;
  fp->_lb._base = NULL;		/* no line buffer */
  fp->_lb._size = 0;

  return fp;
}

/*
 * exit() calls _cleanup() through *__cleanup, set whenever we
 * open or buffer a file.  This chicanery is done so that programs
 * that do not use stdio need not link it all in.
 *
 * The name `_cleanup' is, alas, fairly well known outside stdio.
 */

static void
cleanup_stdio (void)
{
  if (stdin != &__sf[0])
    CLEANUP_FILE (stdin);
  if (stdout != &__sf[1])
    CLEANUP_FILE (stdout);
  if (stderr != &__sf[2])
    CLEANUP_FILE (stderr);
  (void) _fwalk_sglue (CLEANUP_FILE, &__sglue);
}

/*
 * __sinit() is called whenever stdio's internal variables must be set up.
 */

void
__sinit (void)
{
  __sfp_lock_acquire ();

  if (_stdio_cleanup)
    {
      __sfp_lock_release ();
      return;
    }

  /* make sure we clean up on exit */
  _stdio_cleanup = cleanup_stdio;	/* conservative */

  stdin_init (&__sf[0]);
  stdout_init (&__sf[1]);
  stderr_init (&__sf[2]);
  __sfp_lock_release ();
}
