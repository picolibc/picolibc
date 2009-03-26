/* strfuncs.cc: misc funcs that don't belong anywhere else

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <wchar.h>
#include <winnls.h>
#include <ntdll.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"

/* The SJIS, JIS and EUCJP conversion in newlib does not use UTF as
   wchar_t character representation.  That's unfortunate for us since
   we require UTF for the OS.  What we do here is to have our own
   implementation of the base functions for the conversion using
   the MulitByteToWideChar/WideCharToMultiByte functions. */

/* GBK, eucKR, and Big5 conversions are not available so far in newlib. */

static int
__db_wctomb (struct _reent *r, char *s, wchar_t wchar, UINT cp)
{
  if (s == NULL)
    return 0;

  if (wchar < 0x80)
    {
      *s = (char) wchar;
      return 1;
    }

  BOOL def_used = false;
  int ret = WideCharToMultiByte (cp, cp > 50000 ? 0 : WC_NO_BEST_FIT_CHARS,
				 &wchar, 1, s, MB_CUR_MAX, NULL, &def_used);
  if (ret > 0 && !def_used)
    return ret;

  r->_errno = EILSEQ;
  return -1;
}

extern "C" int
__sjis_wctomb (struct _reent *r, char *s, wchar_t wchar, const char *charset,
	       mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 932);
}

extern "C" int
__jis_wctomb (struct _reent *r, char *s, wchar_t wchar, const char *charset,
	       mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 50220);
}

extern "C" int
__eucjp_wctomb (struct _reent *r, char *s, wchar_t wchar, const char *charset,
	       mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 51932);
}

extern "C" int
__gbk_wctomb (struct _reent *r, char *s, wchar_t wchar, const char *charset,
	       mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 936);
}

extern "C" int
__kr_wctomb (struct _reent *r, char *s, wchar_t wchar, const char *charset,
	       mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 51949);
}

extern "C" int
__big5_wctomb (struct _reent *r, char *s, wchar_t wchar, const char *charset,
	       mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 950);
}

static int
__db_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	     UINT cp, mbstate_t *state)
{
  wchar_t dummy;
  char buf[2];
  int ret;
  
  if (pwc == NULL)
    pwc = &dummy;

  if (s == NULL)
    return 0;  /* not state-dependent */

  if (n == 0)
    return -2;

  if (state->__count == 0)
    {
      if (*(unsigned char *) s < 0x80)
	{
	  *pwc = *(unsigned char *) s;
	  return *s ? 1 : 0;
	}
      ret = MultiByteToWideChar (cp, cp > 50000 ? 0 : MB_ERR_INVALID_CHARS,
				 s, 2, pwc, 1);
      if (ret)
	return *s ? 2 : 0;
      if (n == 1)
	{
	  state->__count = 1;
	  state->__value.__wchb[0] = *s;
	  return -2;
	}
      else
	{
	  /* These Win32 functions are really crappy.  Assuming n is 2
	     but the first byte is a singlebyte charcode, the function
	     does not convert that byte and return 1, rather it just
	     returns 0.  So, what we do here is to check if the first
	     byte returns a valid value... */
	  ret = MultiByteToWideChar (cp,
				     cp > 50000 ? 0 : MB_ERR_INVALID_CHARS,
				     s, 1, pwc, 1);
	  if (ret)
	    return *s ? 1 : 0;
	}
      r->_errno = EILSEQ;
      return -1;
    }
  if (!*s)
    return -2;
  buf[0] = state->__value.__wchb[0];
  buf[1] = *s;
  ret = MultiByteToWideChar (cp, cp > 50000 ? 0 : MB_ERR_INVALID_CHARS,
			     buf, 2, pwc, 1);
  if (!ret)
    {
      r->_errno = EILSEQ;
      return -1;
    }
  return ret;
}

extern "C" int
__sjis_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	       const char *charset, mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 932, state);
}

extern "C" int
__jis_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	       const char *charset, mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 50220, state);
}

extern "C" int
__eucjp_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	       const char *charset, mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 51932, state);
}

extern "C" int
__gbk_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	       const char *charset, mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 936, state);
}

extern "C" int
__kr_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	       const char *charset, mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 51949, state);
}

extern "C" int
__big5_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	       const char *charset, mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 950, state);
}

/* Convert Windows codepage to a setlocale compatible character set code.
   Called from newlib's setlocale() with the current ANSI codepage, if the
   charset isn't given explicitely in the POSIX compatible locale specifier.
   The function also returns a pointer to the corresponding _mbtowc_r
   function.  This is used below in the sys_cp_mbstowcs function which
   is called directly from fhandler_console if the "Alternate Charset" has
   been switched on by an escape sequence. */
extern "C" mbtowc_p
__set_charset_from_codepage (UINT cp, char *charset)
{
  switch (cp)
    {
    case 437:
    case 720:
    case 737:
    case 775:
    case 850:
    case 852:
    case 855:
    case 857:
    case 858:
    case 862:
    case 866:
    case 874:
    case 1125:
    case 1250:
    case 1251:
    case 1252:
    case 1253:
    case 1254:
    case 1255:
    case 1256:
    case 1257:
    case 1258:
      __small_sprintf (charset, "CP%u", cp);
      return __cp_mbtowc;
    case 28591:
    case 28592:
    case 28593:
    case 28594:
    case 28595:
    case 28596:
    case 28597:
    case 28598:
    case 28599:
    case 28603:
    case 28605:
      __small_sprintf (charset, "ISO-8859-%u", cp - 28590);
      return __iso_mbtowc;
    case 932:
      strcpy (charset, "SJIS");
      return __sjis_mbtowc;
    case 936:
      strcpy (charset, "GBK");
      return __gbk_mbtowc;
    case 949:
    case 51949:
      strcpy (charset, "EUCKR");
      return __kr_mbtowc;
    case 950:
      strcpy (charset, "BIG5");
      return __big5_mbtowc;
    case 50220:
      strcpy (charset, "JIS");
      return __jis_mbtowc;
    case 51932:
      strcpy (charset, "EUCJP");
      return __eucjp_mbtowc;
    case 65001:
      strcpy (charset, "UTF-8");
      return __utf8_mbtowc;
    default:
      break;
    }
  strcpy (charset, "ASCII");
  return __ascii_mbtowc;
}

/* Our own sys_wcstombs/sys_mbstowcs functions differ from the
   wcstombs/mbstowcs API in three ways:

   - The UNICODE private use area is used in filenames to specify
     characters not allowed in Windows filenames ('*', '?', etc).
     The sys_wcstombs converts characters in the private use area
     back to the corresponding ASCII chars.

   - If a wide character in a filename has no representation in the current
     multibyte charset, then usually you wouldn't be able to access the
     file.  To fix this problem, sys_wcstombs creates a replacement multibyte
     sequences for the non-representable wide-char.  The sequence starts with
     an ASCII SO (0x0e, Ctrl-N), followed by the UTF-8 representation of the
     character.  The sys_(cp_)mbstowcs function detects ASCII SO characters
     in the input multibyte string and converts the following multibyte
     sequence in by treating it as an UTF-8 char.  If that fails, the ASCII
     SO was probably standalone and it gets just copied over as ASCII SO.

   - The functions always create 0-terminated results, no matter what.
     If the result is truncated due to buffer size, it's a bug in Cygwin
     and the buffer in the calling function should be raised. */
size_t __stdcall
sys_wcstombs (char *dst, size_t len, const PWCHAR src, size_t nwc)
{
  char buf[10];
  char *ptr = dst;
  wchar_t *pwcs = (wchar_t *) src;
  size_t n = 0;
  mbstate_t ps;

  memset (&ps, 0, sizeof ps);
  if (dst == NULL)
    len = (size_t) -1;
  while (n < len && nwc-- > 0)
    {
      wchar_t pw = *pwcs;
      /* Convert UNICODE private use area.  Reverse functionality (only for
	 path names) is transform_chars in path.cc. */
      if ((pw & 0xff00) == 0xf000)
	pw &= 0xff;
      int bytes = _wctomb_r (_REENT, buf, pw, &ps);
      /* Convert chars invalid in the current codepage to a sequence
         ASCII SO; UTF-8 representation of invalid char. */
      if (bytes == -1 && *__locale_charset () != 'U'/*TF-8*/)
        {
	  buf[0] = 0x0e; /* ASCII SO */
	  bytes = __utf8_wctomb (_REENT, buf + 1, pw, __locale_charset (), &ps);
	  if (bytes == -1)
	    {
	      ++pwcs;
	      ps.__count = 0;
	      continue;
	    }
	  ++bytes; /* Add the ASCII SO to the byte count. */
	  if (ps.__count == -4) /* First half of a surrogate pair. */
	    {
	      ++pwcs;
	      if ((*pwcs & 0xfc00) != 0xdc00) /* Invalid second half. */
		{
		  ++pwcs;
		  ps.__count = 0;
		  continue;
		}
	      bytes += __utf8_wctomb (_REENT, buf + bytes, *pwcs,
				      __locale_charset (), &ps);
	    }
        }
      if (n + bytes <= len)
        {
          n += bytes;
          if (dst)
            {
              for (int i = 0; i < bytes; ++i)
                *ptr++ = buf[i];
            }
          if (*pwcs++ == 0x00)
	    break;
        }
      else
        break;
    }
  if (n && dst)
    {
      n = (n < len) ? n : len - 1;
      dst[n] = '\0';
    }

  return n;
}

/* Allocate a buffer big enough for the string, always including the
   terminating '\0'.  The buffer pointer is returned in *dst_p, the return
   value is the number of bytes written to the buffer, as usual.
   The "type" argument determines where the resulting buffer is stored.
   It's either one of the cygheap_types values, or it's "HEAP_NOTHEAP".
   In the latter case the allocation uses simple calloc.

   Note that this code is shared by cygserver (which requires it via
   __small_vsprintf) and so when built there plain calloc is the
   only choice.  */
size_t __stdcall
sys_wcstombs_alloc (char **dst_p, int type, const PWCHAR src, size_t nwc)
{
  size_t ret;

  ret = sys_wcstombs (NULL, (size_t) -1, src, nwc);
  if (ret > 0)
    {
      size_t dlen = ret + 1;

      if (type == HEAP_NOTHEAP)
	*dst_p = (char *) calloc (dlen, sizeof (char));
      else
	*dst_p = (char *) ccalloc ((cygheap_types) type, dlen, sizeof (char));
      if (!*dst_p)
	return 0;
      ret = sys_wcstombs (*dst_p, dlen, src, nwc);
    }
  return ret;
}

/* sys_cp_mbstowcs is actually most of the time called as sys_mbstowcs with
   a 0 codepage.  If cp is not 0, the codepage is evaluated and used for the
   conversion.  This is so that fhandler_console can switch to an alternate
   charset, which is the charset returned by GetConsoleCP ().  Most of the
   time this is used for box and line drawing characters. */
size_t __stdcall
sys_cp_mbstowcs (UINT cp, PWCHAR dst, size_t dlen, const char *src, size_t nms)
{
  wchar_t *ptr = dst;
  char *pmbs = (char *) src;
  size_t count = 0;
  size_t len = dlen;
  int bytes;
  mbstate_t ps;
  char charsetbuf[32];
  char *charset = __locale_charset ();
  mbtowc_p f_mbtowc = __mbtowc;

  if (cp)
    f_mbtowc = __set_charset_from_codepage (cp, charset = charsetbuf);

  memset (&ps, 0, sizeof ps);
  if (dst == NULL)
    len = (size_t)-1;
  while (len > 0 && nms > 0)
    {
      /* ASCII SO.  Convert following UTF-8 sequence (if not UTF-8 anyway). */
      if (*pmbs == 0x0e && *charset != 'U'/*TF-8*/)
	{
	  pmbs++;
	  --nms;
	  bytes = __utf8_mbtowc (_REENT, ptr, pmbs, nms, charset, &ps);
	  if (bytes < 0)
	    {
	      /* Invalid UTF-8 sequence?  Treat the ASCII SO character as
	         stand-alone ASCII SO char. */
	      bytes = 1;
	      if (dst)
	      	*ptr = 0x0e;
	      memset (&ps, 0, sizeof ps);
	      break;
	    }
	  if (bytes == 0)
	    break;
	  if (ps.__count == 4) /* First half of a surrogate. */
	    {
	      wchar_t *ptr2 = dst ? ptr + 1 : NULL;
	      int bytes2 = __utf8_mbtowc (_REENT, ptr2, pmbs + bytes,
					  nms - bytes, charset, &ps);
	      if (bytes2 < 0)
		break;
	      pmbs += bytes2;
	      nms -= bytes2;
	      ++count;
	      ptr = dst ? ptr + 1 : NULL;
	      --len;
	    }
	}
      else
	bytes = f_mbtowc (_REENT, ptr, pmbs, nms, charset, &ps);
      if (bytes > 0)
        {
          pmbs += bytes;
          nms -= bytes;
          ++count;
	  ptr = dst ? ptr + 1 : NULL;
          --len;
        }
      else
	{
	  if (bytes == 0)
	    ++count;
	  break;
	}
    }

  if (count && dst)
    {
      count = (count < dlen) ? count : dlen - 1;
      dst[count] = L'\0';
    }

  return count;
}

/* Same as sys_wcstombs_alloc, just backwards. */
size_t __stdcall
sys_mbstowcs_alloc (PWCHAR *dst_p, int type, const char *src, size_t nms)
{
  size_t ret;

  ret = sys_mbstowcs (NULL, (size_t) -1, src, nms);
  if (ret > 0)
    {
      size_t dlen = ret + 1;

      if (type == HEAP_NOTHEAP)
	*dst_p = (PWCHAR) calloc (dlen, sizeof (WCHAR));
      else
	*dst_p = (PWCHAR) ccalloc ((cygheap_types) type, dlen, sizeof (WCHAR));
      if (!*dst_p)
	return 0;
      ret = sys_mbstowcs (*dst_p, dlen, src, nms);
    }
  return ret;
}

static WCHAR hex_wchars[] = L"0123456789abcdef";

NTSTATUS NTAPI
RtlInt64ToHexUnicodeString (ULONGLONG value, PUNICODE_STRING dest,
			    BOOLEAN append)
{
  USHORT len = append ? dest->Length : 0;
  if (dest->MaximumLength - len < 16 * (int) sizeof (WCHAR))
    return STATUS_BUFFER_OVERFLOW;
  PWCHAR end = (PWCHAR) ((PBYTE) dest->Buffer + len);
  register PWCHAR p = end + 16;
  while (p-- > end)
    {
      *p = hex_wchars[value & 0xf];
      value >>= 4;
    }
  dest->Length += 16 * sizeof (WCHAR);
  return STATUS_SUCCESS;
}
