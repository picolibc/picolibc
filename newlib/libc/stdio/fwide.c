/*
FUNCTION
<<fwide>>---set and determine the orientation of a FILE stream

INDEX
	fwide
INDEX
	_fwide_r

ANSI_SYNOPSIS
	#include <wchar.h>
	int fwide(FILE *<[fp]>, int <[mode]>)

	int _fwide_r(struct _reent *<[ptr]>, FILE *<[fp]>, int <[mode]>)

TRAD_SYNOPSIS
	#include <wchar.h>
	int fwide(<[fp]>, <[mode]>)
	FILE *<[fp]>;
	int <[mode]>;

	int _fwide_r(<[ptr]>, <[fp]>, <[mode]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;
	int <[mode]>;

DESCRIPTION
When <[mode]> is zero, the <<fwide>> function determines the current
orientation of <[fp]>. It returns a value > 0 if <[fp]> is
wide-character oriented, i.e. if wide character I/O is permitted but
char I/O is disallowed. It returns a value < 0 if <[fp]> is byte
oriented, i.e. if char I/O is permitted but wide character I/O is
disallowed. It returns zero if <[fp]> has no orientation yet; in
this case the next I/O operation might change the orientation (to byte
oriented if it is a char I/O operation, or to wide-character oriented
if it is a wide character I/O operation).

Once a stream has an orientation, it cannot be changed and persists
until the stream is closed, unless the stream is re-opened with freopen,
which removes the orientation of the stream.

When <[mode]> is non-zero, the <<fwide>> function first attempts to set
<[fp]>'s orientation (to wide-character oriented if <[mode]> > 0, or to
byte oriented if <[mode]> < 0). It then returns a value denoting the
current orientation, as above.

RETURNS
The <<fwide>> function returns <[fp]>'s orientation, after possibly
changing it. A return value > 0 means wide-character oriented. A return
value < 0 means byte oriented. A return value of zero means undecided.

PORTABILITY
C99, POSIX.1-2001.

*/

#include <_ansi.h>
#include <wchar.h>
#include "local.h"

int
_DEFUN(_fwide_r, (ptr, fp, mode),
	struct _reent *ptr _AND
	FILE *fp _AND
	int mode)
{
  int ret;

  CHECK_INIT(ptr, fp);

  _newlib_flockfile_start (fp);
  if (mode != 0) {
    ORIENT (fp, mode);
  }
  if (!(fp->_flags & __SORD))
    ret = 0;
  else
    ret = (fp->_flags2 & __SWID) ? 1 : -1;
  _newlib_flockfile_end (fp);
  return ret;
}

int
_DEFUN(fwide, (fp, mode),
	FILE *fp _AND
	int mode)
{
  return _fwide_r (_REENT, fp, mode);
}
