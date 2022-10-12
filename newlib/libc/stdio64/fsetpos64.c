/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
by the University of California, Berkeley.  The name of the
University may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
FUNCTION
<<fsetpos64>>---restore position of a large stream or file

INDEX
	fsetpos64
INDEX
	_fsetpos64_r

SYNOPSIS
	#include <stdio.h>
	int fsetpos64(FILE *<[fp]>, const _fpos64_t *<[pos]>);
	int _fsetpos64_r(struct _reent *<[ptr]>, FILE *<[fp]>, 
	                 const _fpos64_t *<[pos]>);

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

You can use <<fsetpos64>> to return the large file identified by <[fp]> to a 
previous position <<*<[pos]>>> (after first recording it with <<fgetpos64>>).

See <<fseeko64>> for a similar facility.

RETURNS
<<fgetpos64>> returns <<0>> when successful.  If <<fgetpos64>> fails, the
result is <<1>>.  The reason for failure is indicated in <<errno>>:
either <<ESPIPE>> (the stream identified by <[fp]> doesn't support
64-bit repositioning) or <<EINVAL>> (invalid file position).

PORTABILITY
<<fsetpos64>> is a glibc extension.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek64>>, <<read>>, <<sbrk>>, <<write>>.
*/

#define _DEFAULT_SOURCE
#include <stdio.h>

#ifdef __LARGE64_FILES

int
fsetpos64 (
	FILE * iop,
	const _fpos64_t * pos)
{
  int x = fseeko64 (iop, (_off64_t)(*pos), SEEK_SET);

  if (x != 0)
    return 1;
  return 0;
}

#endif /* __LARGE64_FILES */
