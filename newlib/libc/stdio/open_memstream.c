/* Copyright (C) 2007 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
FUNCTION
<<open_memstream>>, <<open_wmemstream>>---open a write stream around an arbitrary-length string

INDEX
	open_memstream
INDEX
	open_wmemstream

SYNOPSIS
	#include <stdio.h>
	FILE *open_memstream(char **restrict <[buf]>,
			     size_t *restrict <[size]>);

	#include <wchar.h>
	FILE *open_wmemstream(wchar_t **restrict <[buf]>,
			      size_t *restrict <[size]>);

DESCRIPTION
<<open_memstream>> creates a seekable, byte-oriented <<FILE>> stream that
wraps an arbitrary-length buffer, created as if by <<malloc>>.  The current
contents of *<[buf]> are ignored; this implementation uses *<[size]>
as a hint of the maximum size expected, but does not fail if the hint
was wrong.  The parameters <[buf]> and <[size]> are later stored
through following any call to <<fflush>> or <<fclose>>, set to the
current address and usable size of the allocated string; although
after fflush, the pointer is only valid until another stream operation
that results in a write.  Behavior is undefined if the user alters
either *<[buf]> or *<[size]> prior to <<fclose>>.

<<open_wmemstream>> is like <<open_memstream>> just with the associated
stream being wide-oriented.  The size set in <[size]> in subsequent
operations is the number of wide characters.

The stream is write-only, since the user can directly read *<[buf]>
after a flush; see <<fmemopen>> for a way to wrap a string with a
readable stream.  The user is responsible for calling <<free>> on
the final *<[buf]> after <<fclose>>.

Any time the stream is flushed, a NUL byte is written at the current
position (but is not counted in the buffer length), so that the string
is always NUL-terminated after at most *<[size]> bytes (or wide characters
in case of <<open_wmemstream>>).  However, data previously written beyond
the current stream offset is not lost, and the NUL value written during a
flush is restored to its previous value when seeking elsewhere in the string.

RETURNS
The return value is an open FILE pointer on success.  On error,
<<NULL>> is returned, and <<errno>> will be set to EINVAL if <[buf]>
or <[size]> is NULL, ENOMEM if memory could not be allocated, or
EMFILE if too many streams are already open.

PORTABILITY
POSIX.1-2008

Supporting OS subroutines required: <<sbrk>>.
*/

#include <stdio.h>
#include <wchar.h>
#include <errno.h>
#include <string.h>
#include <sys/lock.h>
#include <stdint.h>
#include "local.h"

#ifndef __LARGE64_FILES
# define OFF_T off_t
#else
# define OFF_T _off64_t
#endif

/* Describe details of an open memstream.  */
typedef struct memstream {
  void *storage; /* storage to free on close */
  char **pbuf; /* pointer to the current buffer */
  size_t *psize; /* pointer to the current size, smaller of pos or eof */
  size_t pos; /* current position */
  size_t eof; /* current file size */
  size_t max; /* current malloc buffer size, always > eof */
  union {
    char c;
    wchar_t w;
  } saved; /* saved character that lived at *psize before NUL */
  int8_t wide; /* wide-oriented (>0) or byte-oriented (<0) */
} memstream;

/* Write up to non-zero N bytes of BUF into the stream described by COOKIE,
   returning the number of bytes written or EOF on failure.  */
static _READ_WRITE_RETURN_TYPE
memwriter (struct _reent *ptr,
       void *cookie,
       const char *buf,
       _READ_WRITE_BUFSIZE_TYPE n)
{
  memstream *c = (memstream *) cookie;
  char *cbuf = *c->pbuf;

  /* size_t is unsigned, but off_t is signed.  Don't let stream get so
     big that user cannot do ftello.  */
  if (sizeof (OFF_T) == sizeof (size_t) && (ssize_t) (c->pos + n) < 0)
    {
      ptr->_errno = EFBIG;
      return EOF;
    }
  /* Grow the buffer, if necessary.  Choose a geometric growth factor
     to avoid quadratic realloc behavior, but use a rate less than
     (1+sqrt(5))/2 to accomodate malloc overhead.  Overallocate, so
     that we can add a trailing \0 without reallocating.  The new
     allocation should thus be max(prev_size*1.5, c->pos+n+1). */
  if (c->pos + n >= c->max)
    {
      size_t newsize = c->max * 3 / 2;
      if (newsize < c->pos + n + 1)
	newsize = c->pos + n + 1;
      cbuf = _realloc_r (ptr, cbuf, newsize);
      if (! cbuf)
	return EOF; /* errno already set to ENOMEM */
      *c->pbuf = cbuf;
      c->max = newsize;
    }
  /* If we have previously done a seek beyond eof, ensure all
     intermediate bytes are NUL.  */
  if (c->pos > c->eof)
    memset (cbuf + c->eof, '\0', c->pos - c->eof);
  memcpy (cbuf + c->pos, buf, n);
  c->pos += n;
  /* If the user has previously written further, remember what the
     trailing NUL is overwriting.  Otherwise, extend the stream.  */
  if (c->pos > c->eof)
    c->eof = c->pos;
  else if (c->wide > 0)
    c->saved.w = *(wchar_t *)(cbuf + c->pos);
  else
    c->saved.c = cbuf[c->pos];
  cbuf[c->pos] = '\0';
  *c->psize = (c->wide > 0) ? c->pos / sizeof (wchar_t) : c->pos;
  return n;
}

/* Seek to position POS relative to WHENCE within stream described by
   COOKIE; return resulting position or fail with EOF.  */
static _fpos_t
memseeker (struct _reent *ptr,
       void *cookie,
       _fpos_t pos,
       int whence)
{
  memstream *c = (memstream *) cookie;
  OFF_T offset = (OFF_T) pos;

  if (whence == SEEK_CUR)
    offset += c->pos;
  else if (whence == SEEK_END)
    offset += c->eof;
  if (offset < 0)
    {
      ptr->_errno = EINVAL;
      offset = -1;
    }
  else if ((size_t) offset != offset)
    {
      ptr->_errno = ENOSPC;
      offset = -1;
    }
#ifdef __LARGE64_FILES
  else if ((_fpos_t) offset != offset)
    {
      ptr->_errno = EOVERFLOW;
      offset = -1;
    }
#endif /* __LARGE64_FILES */
  else
    {
      if (c->pos < c->eof)
	{
	  if (c->wide > 0)
	    *(wchar_t *)((*c->pbuf) + c->pos) = c->saved.w;
	  else
	    (*c->pbuf)[c->pos] = c->saved.c;
	  c->saved.w = L'\0';
	}
      c->pos = offset;
      if (c->pos < c->eof)
	{
	  if (c->wide > 0)
	    {
	      c->saved.w = *(wchar_t *)((*c->pbuf) + c->pos);
	      *(wchar_t *)((*c->pbuf) + c->pos) = L'\0';
	      *c->psize = c->pos / sizeof (wchar_t);
	    }
	  else
	    {
	      c->saved.c = (*c->pbuf)[c->pos];
	      (*c->pbuf)[c->pos] = '\0';
	      *c->psize = c->pos;
	    }
	}
      else if (c->wide > 0)
	*c->psize = c->eof / sizeof (wchar_t);
      else
	*c->psize = c->eof;
    }
  return (_fpos_t) offset;
}

/* Seek to position POS relative to WHENCE within stream described by
   COOKIE; return resulting position or fail with EOF.  */
#ifdef __LARGE64_FILES
static _fpos64_t
memseeker64 (struct _reent *ptr,
       void *cookie,
       _fpos64_t pos,
       int whence)
{
  _off64_t offset = (_off64_t) pos;
  memstream *c = (memstream *) cookie;

  if (whence == SEEK_CUR)
    offset += c->pos;
  else if (whence == SEEK_END)
    offset += c->eof;
  if (offset < 0)
    {
      ptr->_errno = EINVAL;
      offset = -1;
    }
  else if ((size_t) offset != offset)
    {
      ptr->_errno = ENOSPC;
      offset = -1;
    }
  else
    {
      if (c->pos < c->eof)
	{
	  if (c->wide > 0)
	    *(wchar_t *)((*c->pbuf) + c->pos) = c->saved.w;
	  else
	    (*c->pbuf)[c->pos] = c->saved.c;
	  c->saved.w = L'\0';
	}
      c->pos = offset;
      if (c->pos < c->eof)
	{
	  if (c->wide > 0)
	    {
	      c->saved.w = *(wchar_t *)((*c->pbuf) + c->pos);
	      *(wchar_t *)((*c->pbuf) + c->pos) = L'\0';
	      *c->psize = c->pos / sizeof (wchar_t);
	    }
	  else
	    {
	      c->saved.c = (*c->pbuf)[c->pos];
	      (*c->pbuf)[c->pos] = '\0';
	      *c->psize = c->pos;
	    }
	}
      else if (c->wide > 0)
	*c->psize = c->eof / sizeof (wchar_t);
      else
	*c->psize = c->eof;
    }
  return (_fpos64_t) offset;
}
#endif /* __LARGE64_FILES */

/* Reclaim resources used by stream described by COOKIE.  */
static int
memcloser (struct _reent *ptr,
       void *cookie)
{
  memstream *c = (memstream *) cookie;
  char *buf;

  /* Be nice and try to reduce any unused memory.  */
  buf = _realloc_r (ptr, *c->pbuf,
		    c->wide > 0 ? (*c->psize + 1) * sizeof (wchar_t)
				: *c->psize + 1);
  if (buf)
    *c->pbuf = buf;
  _free_r (ptr, c->storage);
  return 0;
}

/* Open a memstream that tracks a dynamic buffer in BUF and SIZE.
   Return the new stream, or fail with NULL.  */
static FILE *
internal_open_memstream_r (struct _reent *ptr,
       char **buf,
       size_t *size,
       int wide)
{
  FILE *fp;
  memstream *c;

  if (!buf || !size)
    {
      ptr->_errno = EINVAL;
      return NULL;
    }
  if ((fp = __sfp (ptr)) == NULL)
    return NULL;
  if ((c = (memstream *) _malloc_r (ptr, sizeof *c)) == NULL)
    {
      _newlib_sfp_lock_start ();
      fp->_flags = 0;		/* release */
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      _newlib_sfp_lock_end ();
      return NULL;
    }
  /* Use *size as a hint for initial sizing, but bound the initial
     malloc between 64 bytes (same as asprintf, to avoid frequent
     mallocs on small strings) and 64k bytes (to avoid overusing the
     heap if *size was garbage).  */
  c->max = *size;
  if (wide == 1)
    c->max *= sizeof(wchar_t);
  if (c->max < 64)
    c->max = 64;
#if (SIZE_MAX >= 64 * 1024)
  else if (c->max > 64 * 1024)
    c->max = 64 * 1024;
#endif
  *size = 0;
  *buf = _malloc_r (ptr, c->max);
  if (!*buf)
    {
      _newlib_sfp_lock_start ();
      fp->_flags = 0;		/* release */
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      _newlib_sfp_lock_end ();
      _free_r (ptr, c);
      return NULL;
    }
  if (wide == 1)
    **((wchar_t **)buf) = L'\0';
  else
    **buf = '\0';

  c->storage = c;
  c->pbuf = buf;
  c->psize = size;
  c->pos = 0;
  c->eof = 0;
  c->saved.w = L'\0';
  c->wide = (int8_t) wide;

  _newlib_flockfile_start (fp);
  fp->_file = -1;
  fp->_flags = __SWR;
  fp->_cookie = c;
  fp->_read = NULL;
  fp->_write = memwriter;
  fp->_seek = memseeker;
#ifdef __LARGE64_FILES
  fp->_seek64 = memseeker64;
  fp->_flags |= __SL64;
#endif
  fp->_close = memcloser;
  ORIENT (fp, wide);
  _newlib_flockfile_end (fp);
  return fp;
}

FILE *
_open_memstream_r (struct _reent *ptr,
       char **buf,
       size_t *size)
{
  return internal_open_memstream_r (ptr, buf, size, -1);
}

FILE *
_open_wmemstream_r (struct _reent *ptr,
       wchar_t **buf,
       size_t *size)
{
  return internal_open_memstream_r (ptr, (char **)buf, size, 1);
}

#ifndef _REENT_ONLY
FILE *
open_memstream (char **buf,
       size_t *size)
{
  return _open_memstream_r (_REENT, buf, size);
}

FILE *
open_wmemstream (wchar_t **buf,
       size_t *size)
{
  return _open_wmemstream_r (_REENT, buf, size);
}
#endif /* !_REENT_ONLY */
