/*
FUNCTION
<<fgetpos64>>---record position in a large stream or file

INDEX
	fgetpos64

ANSI_SYNOPSIS
	#include <stdio.h>
	int fgetpos64(FILE *<[fp]>, _fpos64_t *<[pos]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fgetpos64(<[fp]>, <[pos]>)
	FILE *<[fp]>;
	_fpos64_t *<[pos]>;

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

#include <stdio.h>

#ifdef __LARGE64_FILES

int
_DEFUN (fgetpos64, (fp, pos),
	FILE * fp _AND
	_fpos64_t * pos)
{
  _flockfile(fp);
  *pos = (_fpos64_t)ftello64 (fp);

  if (*pos != -1)
    {
      _funlockfile(fp);
      return 0;
    }
  _funlockfile(fp);
  return 1;
}

#endif /* __LARGE64_FILES */
