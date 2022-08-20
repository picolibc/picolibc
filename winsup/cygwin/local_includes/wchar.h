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
