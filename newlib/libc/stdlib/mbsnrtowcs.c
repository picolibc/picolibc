/*
FUNCTION
<<mbsrtowcs>>, <<mbsnrtowcs>>---convert a character string to a wide-character string

INDEX
	mbsrtowcs
INDEX
	_mbsrtowcs_r
INDEX
	mbsnrtowcs
INDEX
	_mbsnrtowcs_r

ANSI_SYNOPSIS
	#include <wchar.h>
	size_t mbsrtowcs(wchar_t *__restrict <[dst]>,
			 const char **__restrict <[src]>,
			 size_t <[len]>,
			 mbstate_t *__restrict <[ps]>);

	#include <wchar.h>
	size_t _mbsrtowcs_r(struct _reent *<[ptr]>, wchar_t *<[dst]>,
			    const char **<[src]>, size_t <[len]>,
			    mbstate_t *<[ps]>);

	#include <wchar.h>
	size_t mbsnrtowcs(wchar_t *__ restrict <[dst]>, 
			  const char **__restrict <[src]>, size_t <[nms]>,
			  size_t <[len]>, mbstate_t *__restrict <[ps]>);

	#include <wchar.h>
	size_t _mbsnrtowcs_r(struct _reent *<[ptr]>, wchar_t *<[dst]>,
			     const char **<[src]>, size_t <[nms]>,
			     size_t <[len]>, mbstate_t *<[ps]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	size_t mbsrtowcs(<[dst]>, <[src]>, <[len]>, <[ps]>)
	wchar_t *__restrict <[dst]>;
	const char **__restrict <[src]>;
	size_t <[len]>;
	mbstate_t *__restrict <[ps]>;

	#include <wchar.h>
	size_t _mbsrtowcs_r(<[ptr]>, <[dst]>, <[src]>, <[len]>, <[ps]>)
	struct _reent *<[ptr]>;
	wchar_t *<[dst]>;
	const char **<[src]>;
	size_t <[len]>;
	mbstate_t *<[ps]>;

	#include <wchar.h>
	size_t mbsnrtowcs(<[dst]>, <[src]>, <[nms]>, <[len]>, <[ps]>)
	wchar_t *__restrict <[dst]>;
	const char **__restrict <[src]>;
	size_t <[nms]>;
	size_t <[len]>;
	mbstate_t *__restrict <[ps]>;

	#include <wchar.h>
	size_t _mbsnrtowcs_r(<[ptr]>, <[dst]>, <[src]>, <[nms]>, <[len]>, <[ps]>)
	struct _reent *<[ptr]>;
	wchar_t *<[dst]>;
	const char **<[src]>;
	size_t <[nms]>;
	size_t <[len]>;
	mbstate_t *<[ps]>;

DESCRIPTION
The <<mbsrtowcs>> function converts a sequence of multibyte characters
pointed to indirectly by <[src]> into a sequence of corresponding wide
characters and stores at most <[len]> of them in the wchar_t array pointed
to by <[dst]>, until it encounters a terminating null character ('\0').

If <[dst]> is NULL, no characters are stored.

If <[dst]> is not NULL, the pointer pointed to by <[src]> is updated to point
to the character after the one that conversion stopped at.  If conversion
stops because a null character is encountered, *<[src]> is set to NULL.

The mbstate_t argument, <[ps]>, is used to keep track of the shift state.  If
it is NULL, <<mbsrtowcs>> uses an internal, static mbstate_t object, which
is initialized to the initial conversion state at program startup.

The <<mbsnrtowcs>> function behaves identically to <<mbsrtowcs>>, except that
conversion stops after reading at most <[nms]> bytes from the buffer pointed
to by <[src]>.

RETURNS
The <<mbsrtowcs>> and <<mbsnrtowcs>> functions return the number of wide
characters stored in the array pointed to by <[dst]> if successful, otherwise
it returns (size_t)-1.

PORTABILITY
<<mbsrtowcs>> is defined by the C99 standard.
<<mbsnrtowcs>> is defined by the POSIX.1-2008 standard.
*/

#include <reent.h>
#include <newlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

size_t
_DEFUN (_mbsnrtowcs_r, (r, dst, src, nms, len, ps), 
	struct _reent *r _AND
	wchar_t *dst _AND
	const char **src _AND
	size_t nms _AND
	size_t len _AND
	mbstate_t *ps)
{
  wchar_t *ptr = dst;
  const char *tmp_src;
  size_t max;
  size_t count = 0;
  int bytes;

#ifdef _MB_CAPABLE
  if (ps == NULL)
    {
      _REENT_CHECK_MISC(r);
      ps = &(_REENT_MBSRTOWCS_STATE(r));
    }
#endif

  if (dst == NULL)
    {
      /* Ignore original len value and do not alter src pointer if the
         dst pointer is NULL.  */
      len = (size_t)-1;
      tmp_src = *src;
      src = &tmp_src;
    }      
  
  max = len;
  while (len > 0)
    {
      bytes = _mbrtowc_r (r, ptr, *src, nms, ps);
      if (bytes > 0)
	{
	  *src += bytes;
	  nms -= bytes;
	  ++count;
	  ptr = (dst == NULL) ? NULL : ptr + 1;
	  --len;
	}
      else if (bytes == -2)
	{
	  *src += nms;
	  return count;
	}
      else if (bytes == 0)
	{
	  *src = NULL;
	  return count;
	}
      else
	{
	  ps->__count = 0;
	  r->_errno = EILSEQ;
	  return (size_t)-1;
	}
    }

  return (size_t)max;
}

#ifndef _REENT_ONLY
size_t
_DEFUN (mbsnrtowcs, (dst, src, nms, len, ps),
	wchar_t *__restrict dst _AND
	const char **__restrict src _AND
	size_t nms _AND
	size_t len _AND
	mbstate_t *__restrict ps)
{
  return _mbsnrtowcs_r (_REENT, dst, src, nms, len, ps);
}
#endif /* !_REENT_ONLY */
