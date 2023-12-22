/*
Copyright (c) 2002-2004 Tim J. Robbins.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/*
FUNCTION
<<fwide>>---set and determine the orientation of a FILE stream

INDEX
	fwide
INDEX
	_fwide_r

SYNOPSIS
	#include <wchar.h>
	int fwide(FILE *<[fp]>, int <[mode]>);

	int fwide( FILE *<[fp]>, int <[mode]>);

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

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <wchar.h>
#include "local.h"

int
fwide (
	FILE *fp,
	int mode)
{
  int ret;

  CHECK_INIT(ptr, fp);

  _newlib_flockfile_start (fp);
  if (mode != 0) {
    (void) ORIENT (fp, mode);
  }
  if (!(fp->_flags & __SORD))
    ret = 0;
  else
    ret = (fp->_flags2 & __SWID) ? 1 : -1;
  _newlib_flockfile_end (fp);
  return ret;
}
