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
<<fgetpos64>>---record position in a large stream or file

INDEX
	fgetpos64
INDEX
	_fgetpos64_r

SYNOPSIS
	#include <stdio.h>
	int fgetpos64(FILE *<[fp]>, _fpos64_t *<[pos]>);
	int _fgetpos64_r(struct _reent *<[ptr]>, FILE *<[fp]>, 
	                 _fpos64_t *<[pos]>);

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

You can use <<fgetpos64>> to report on the current position for a file
identified by <[fp]> that was opened by <<fopen64>>; <<fgetpos>> will write 
a value representing that position at <<*<[pos]>>>.  Later, you can
use this value with <<fsetpos64>> to return the file to this
position.

In the current implementation, <<fgetpos64>> simply uses a character
count to represent the file position; this is the same number that
would be returned by <<ftello64>>.

RETURNS
<<fgetpos64>> returns <<0>> when successful.  If <<fgetpos64>> fails, the
result is <<1>>.  Failure occurs on streams that do not support
positioning or streams not opened via <<fopen64>>; the global <<errno>> 
indicates these conditions with the value <<ESPIPE>>.

PORTABILITY
<<fgetpos64>> is a glibc extension.

No supporting OS subroutines are required.
*/

#define _DEFAULT_SOURCE
#include <stdio.h>

#ifdef __LARGE64_FILES

int
fgetpos64 (
	FILE * fp,
	_fpos64_t * pos)
{
  *pos = (_fpos64_t)ftello64 (fp);

  if (*pos != -1)
    {
      return 0;
    }
  return 1;
}

#endif /* __LARGE64_FILES */
