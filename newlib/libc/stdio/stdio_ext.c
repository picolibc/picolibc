/*
FUNCTION
<<stdio_ext>>,<<__fbufsize>>,<<__fpending>>,<<__flbf>>,<<__freadable>>,<<__fwritable>>,<<__freading>>,<<__fwriting>>---access internals of FILE structure

INDEX
	__fbufsize
INDEX
	__fpending
INDEX
	__flbf
INDEX
	__freadable
INDEX
	__fwritable
INDEX
	__freading
INDEX
	__fwriting

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdio_ext.h>
	size_t __fbufsize(FILE *<[fp]>);
	size_t __fpending(FILE *<[fp]>);
	int __flbf(FILE *<[fp]>);
	int __freadable(FILE *<[fp]>);
	int __fwritable(FILE *<[fp]>);
	int __freading(FILE *<[fp]>);
	int __fwriting(FILE *<[fp]>);

DESCRIPTION
These functions provides access to the internals of the FILE structure <[fp]>.

RETURNS
<<__fbufsize>> returns the number of bytes in the buffer of stream <[fp]>.

<<__fpending>> returns the number of bytes in the output buffer of stream <[fp]>.

<<__flbf>> returns nonzero if stream <[fp]> is line-buffered, and <<0>> if not.

<<__freadable>> returns nonzero if stream <[fp]> may be read, and <<0>> if not.

<<__fwritable>> returns nonzero if stream <[fp]> may be written, and <<0>> if not.

<<__freading>> returns nonzero if stream <[fp]> if the last operation on
it was a read, or if it read-only, and <<0>> if not.

<<__fwriting>> returns nonzero if stream <[fp]> if the last operation on
it was a write, or if it write-only, and <<0>> if not.

PORTABILITY
These functions originate from Solaris and are also provided by GNU libc.

No supporting OS subroutines are required.
*/

#ifndef __rtems__

#include <_ansi.h>
#include <stdio.h>

/* Subroutine versions of the inline or macro functions. */

size_t
_DEFUN(__fbufsize, (fp),
       FILE * fp)
{
  return (size_t) fp->_bf._size;
}

size_t
_DEFUN(__fpending, (fp),
       FILE * fp)
{
  return fp->_p - fp->_bf._base;
}

int
_DEFUN(__flbf, (fp),
       FILE * fp)
{
  return (fp->_flags & __SLBF) != 0;
}

int
_DEFUN(__freadable, (fp),
       FILE * fp)
{
  return (fp->_flags & (__SRD | __SRW)) != 0;
}

int
_DEFUN(__fwritable, (fp),
       FILE * fp)
{
  return (fp->_flags & (__SWR | __SRW)) != 0;
}

int
_DEFUN(__freading, (fp),
       FILE * fp)
{
  return (fp->_flags & __SRD) != 0;
}

int
_DEFUN(__fwriting, (fp),
       FILE * fp)
{
  return (fp->_flags & __SWR) != 0;
}

#endif /* __rtems__ */
