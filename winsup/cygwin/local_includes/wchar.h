/* wchar.h: Extra wchar defs

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_WCHAR_H
#define _CYGWIN_WCHAR_H

#include_next <wchar.h>

/* Internal headers from newlib */
#include "../locale/setlocale.h"

#define ENCODING_LEN 31

#ifdef __cplusplus
extern "C" {
#endif

typedef int mbtowc_f (struct _reent *, wchar_t *, const char *, size_t,
		      mbstate_t *);
typedef mbtowc_f *mbtowc_p;

extern mbtowc_f __ascii_mbtowc;
extern mbtowc_f __utf8_mbtowc;
extern mbtowc_p __iso_mbtowc (int);
extern mbtowc_p __cp_mbtowc (int);

#define __MBTOWC (__get_current_locale ()->mbtowc)

typedef int wctomb_f (struct _reent *, char *, wchar_t, mbstate_t *);
typedef wctomb_f *wctomb_p;

extern wctomb_f __ascii_wctomb;
extern wctomb_f __utf8_wctomb;

#define __WCTOMB (__get_current_locale ()->wctomb)

/* convert wint_t string to wchar_t string.  Make sure dest
   has room for at least twice as much characters to account
   for surrogate pairs, plus a wchar_t NUL. */
void wcintowcs (wchar_t *, wint_t *, size_t);

/* replacement function for wcrtomb, converting a UTF-32 char to a
   multibyte string. */
size_t wirtomb (char *, wint_t, mbstate_t *);

/* replacement function for mbrtowc, returning a wint_t representing
   a UTF-32 value. Defined in strfuncs.cc */
extern size_t mbrtowi (wint_t *, const char *, size_t, mbstate_t *);

/* replacement function for mbsnrtowcs, returning a wint_t representing
   a UTF-32 value. Defined in strfuncs.cc.
   Deviation from standard: If the input is broken, the output will be
   broken.  I. e., we just copy the current byte over into the wint_t
   destination and try to pick up on the next byte.  This is in line
   with the way fnmatch works. */
extern size_t mbsnrtowci(wint_t *, const char **, size_t, size_t, mbstate_t *);

/* convert wint_t string to char string, but *only* if the string consists
   entirely of ASCII chars */
static inline void
wcitoascii(char *dst, wint_t *src)
{
        while ((*dst++ = *src++));
}

/* like wcslen, just for wint_t */
static inline size_t
wcilen (const wint_t *wcs)
{
  size_t ret = 0;

  if (wcs)
    while (*wcs++)
      ++ret;
  return ret;
}

/* like wcschr, just for wint_t */
static inline wint_t *
wcichr (const wint_t *str, wint_t chr)
{
  do
    {
      if (*str == chr)
	return (wint_t *) str;
    }
  while (*str++);
  return NULL;
}

/* like wcscmp, just for wint_t */
static inline int
wcicmp (const wint_t *s1, const wint_t *s2)
{
  while (*s1 == *s2++)
    if (*s1++ == 0)
      return (0);
  return (*s1 - *--s2);
}

/* like wcsncmp, just for wint_t */
static inline int
wcincmp (const wint_t *s1, const wint_t *s2, size_t n)
{
  if (n == 0)
    return (0);
  do
    {
      if (*s1 != *s2++)
        {
          return (*s1 - *--s2);
        }
      if (*s1++ == 0)
        break;
    }
  while (--n != 0);
  return (0);
}

/* like wcpcpy, just for wint_t */
static inline wint_t *
wcipcpy (wint_t *s1, const wint_t *s2)
{
  while ((*s1++ = *s2++))
    ;
  return --s1;
}

/* like wcpncpy, just for wint_t */
static inline wint_t *
wcipncpy (wint_t *dst, const wint_t *src, size_t count)
{
  wint_t *ret = NULL;

  while (count > 0)
    {
      --count;
      if ((*dst++ = *src++) == L'\0')
        {
          ret = dst - 1;
          break;
        }
    }
  while (count-- > 0)
    *dst++ = L'\0';

  return ret ? ret : dst;
}

#ifdef __cplusplus
}
#endif

#ifdef __INSIDE_CYGWIN__
#ifdef __cplusplus
extern size_t _sys_wcstombs (char *dst, size_t len, const wchar_t *src,
			     size_t nwc, bool is_path);
extern size_t _sys_wcstombs_alloc (char **dst_p, int type, const wchar_t *src,
				   size_t nwc, bool is_path);

static inline size_t
sys_wcstombs (char *dst, size_t len, const wchar_t * src,
	      size_t nwc = (size_t) -1)
{
  return _sys_wcstombs (dst, len, src, nwc, true);
}

static inline size_t
sys_wcstombs_no_path (char *dst, size_t len, const wchar_t * src,
		      size_t nwc = (size_t) -1)
{
  return _sys_wcstombs (dst, len, src, nwc, false);
}

static inline size_t
sys_wcstombs_alloc (char **dst_p, int type, const wchar_t *src,
		    size_t nwc = (size_t) -1)
{
  return _sys_wcstombs_alloc (dst_p, type, src, nwc, true);
}

static inline size_t
sys_wcstombs_alloc_no_path (char **dst_p, int type, const wchar_t *src,
			    size_t nwc = (size_t) -1)
{
  return _sys_wcstombs_alloc (dst_p, type, src, nwc, false);
}

size_t _sys_mbstowcs (mbtowc_p, wchar_t *, size_t, const char *,
		      size_t = (size_t) -1);

static inline size_t
sys_mbstowcs (wchar_t * dst, size_t dlen, const char *src,
	      size_t nms = (size_t) -1)
{
  mbtowc_p f_mbtowc = (__MBTOWC == __ascii_mbtowc) ? __utf8_mbtowc : __MBTOWC;
  return _sys_mbstowcs (f_mbtowc, dst, dlen, src, nms);
}

size_t sys_mbstowcs_alloc (wchar_t **, int, const char *, size_t = (size_t) -1);

static inline size_t
sys_mbstouni (PUNICODE_STRING dst, int type, const char *src,
	      size_t nms = (size_t) -1)
{
  /* sys_mbstowcs returns length *excluding* trailing \0 */
  size_t len = sys_mbstowcs (dst->Buffer, type, src, nms);
  dst->Length = len * sizeof (WCHAR);
  dst->MaximumLength = dst->Length + sizeof (WCHAR);
  return dst->Length;
}

static inline size_t
sys_mbstouni_alloc (PUNICODE_STRING dst, int type, const char *src,
		    size_t nms = (size_t) -1)
{
  /* sys_mbstowcs_alloc returns length *including* trailing \0 */
  size_t len = sys_mbstowcs_alloc (&dst->Buffer, type, src, nms);
  dst->MaximumLength = len * sizeof (WCHAR);
  dst->Length = dst->MaximumLength - sizeof (WCHAR);
  return dst->MaximumLength;
}

#endif /* __cplusplus */
#endif /* __INSIDE_CYGWIN__ */

#endif /* _CYGWIN_WCHAR_H */
