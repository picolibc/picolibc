/*
FUNCTION
<<fsetpos64>>---restore position of a large stream or file

INDEX
	fsetpos64

ANSI_SYNOPSIS
	#include <stdio.h>
	int fsetpos64(FILE *<[fp]>, const _fpos64_t *<[pos]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fsetpos64(<[fp]>, <[pos]>)
	FILE *<[fp]>;
	_fpos64_t *<[pos]>;

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

#include <stdio.h>

#ifdef __LARGE64_FILES

int
_DEFUN (fsetpos64, (iop, pos),
	FILE * iop _AND
	_CONST _fpos64_t * pos)
{
  int x = fseeko64 (iop, (_off64_t)(*pos), SEEK_SET);

  if (x != 0)
    return 1;
  return 0;
}

#endif /* __LARGE64_FILES */
