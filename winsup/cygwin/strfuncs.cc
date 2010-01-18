/* strfuncs.cc: misc funcs that don't belong anywhere else

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <wchar.h>
#include <winnls.h>
#include <ntdll.h>
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"

/* Transform characters invalid for Windows filenames to the Unicode private
   use area in the U+f0XX range.  The affected characters are all control
   chars 1 <= c <= 31, as well as the characters " * : < > ? |.  The backslash
   is affected as well, but we can't transform it as long as we accept Win32
   paths as input.
   The reverse functionality is in function sys_cp_wcstombs. */
static const WCHAR tfx_chars[] = {
            0, 0xf000 |   1, 0xf000 |   2, 0xf000 |   3,
 0xf000 |   4, 0xf000 |   5, 0xf000 |   6, 0xf000 |   7,
 0xf000 |   8, 0xf000 |   9, 0xf000 |  10, 0xf000 |  11,
 0xf000 |  12, 0xf000 |  13, 0xf000 |  14, 0xf000 |  15,
 0xf000 |  16, 0xf000 |  17, 0xf000 |  18, 0xf000 |  19,
 0xf000 |  20, 0xf000 |  21, 0xf000 |  22, 0xf000 |  23,
 0xf000 |  24, 0xf000 |  25, 0xf000 |  26, 0xf000 |  27,
 0xf000 |  28, 0xf000 |  29, 0xf000 |  30, 0xf000 |  31,
          ' ',          '!', 0xf000 | '"',          '#',
          '$',          '%',          '&',           39,
          '(',          ')', 0xf000 | '*',          '+',
          ',',          '-',          '.',          '\\',
          '0',          '1',          '2',          '3',
          '4',          '5',          '6',          '7',
          '8',          '9', 0xf000 | ':',          ';',
 0xf000 | '<',          '=', 0xf000 | '>', 0xf000 | '?',
          '@',          'A',          'B',          'C',
          'D',          'E',          'F',          'G',
          'H',          'I',          'J',          'K',
          'L',          'M',          'N',          'O',
          'P',          'Q',          'R',          'S',
          'T',          'U',          'V',          'W',
          'X',          'Y',          'Z',          '[',
          '\\',          ']',          '^',          '_',
          '`',          'a',          'b',          'c',
          'd',          'e',          'f',          'g',
          'h',          'i',          'j',          'k',
          'l',          'm',          'n',          'o',
          'p',          'q',          'r',          's',
          't',          'u',          'v',          'w',
          'x',          'y',          'z',          '{',
 0xf000 | '|',          '}',          '~',          127
};

void
transform_chars (PWCHAR path, PWCHAR path_end)
{
  for (; path <= path_end; ++path)
    if (*path < 128)
      *path = tfx_chars[*path];
}

/* The SJIS, JIS and eucJP conversion in newlib does not use UTF as
   wchar_t character representation.  That's unfortunate for us since
   we require UTF for the OS.  What we do here is to have our own
   implementation of the base functions for the conversion using
   the MulitByteToWideChar/WideCharToMultiByte functions. */

/* FIXME: We can't support JIS (ISO-2022-JP) at all right now.  It's a
   stateful charset encoding.  The translation from mbtowc to
   MulitByteToWideChar is quite complex.  Given that we support SJIS and
   eucJP, the both most used Japanese charset encodings, this shouldn't
   be such a big problem. */

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
  int ret = WideCharToMultiByte (cp, WC_NO_BEST_FIT_CHARS, &wchar, 1, s,
				 2, NULL, &def_used);
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
  /* FIXME: See comment at start of file. */
  return __ascii_wctomb (r, s, wchar, charset, state);
}

extern "C" int
__eucjp_wctomb (struct _reent *r, char *s, wchar_t wchar, const char *charset,
	       mbstate_t *state)
{
  /* Unfortunately, the Windows eucJP codepage 20932 is not really 100%
     compatible to eucJP.  It's a cute approximation which makes it a
     doublebyte codepage.
     The JIS-X-0212 three byte codes (0x8f,0xa1-0xfe,0xa1-0xfe) are folded
     into two byte codes as follows: The 0x8f is stripped, the next byte is
     taken as is, the third byte is mapped into the lower 7-bit area by
     masking it with 0x7f.  So, for instance, the eucJP code 0x8f,0xdd,0xf8
     becomes 0xdd,0x78 in CP 20932.

     To be really eucJP compatible, we have to map the JIS-X-0212 characters
     between CP 20932 and eucJP ourselves. */
  if (s == NULL)
    return 0;

  if (wchar < 0x80)
    {
      *s = (char) wchar;
      return 1;
    }

  BOOL def_used = false;
  int ret = WideCharToMultiByte (20932, WC_NO_BEST_FIT_CHARS, &wchar, 1, s,
				 3, NULL, &def_used);
  if (ret > 0 && !def_used)
    {
      /* CP20932 representation of JIS-X-0212 character? */
      if (ret == 2 && (unsigned char) s[1] <= 0x7f)
	{
	  /* Yes, convert to eucJP three byte sequence */
	  s[2] = s[1] | 0x80;
	  s[1] = s[0];
	  s[0] = 0x8f;
	  ++ret;
	}
      return ret;
    }

  r->_errno = EILSEQ;
  return -1;
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
  return __db_wctomb (r,s, wchar, 949);
}

extern "C" int
__big5_wctomb (struct _reent *r, char *s, wchar_t wchar, const char *charset,
	       mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 950);
}

static int
__db_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n, UINT cp,
	     mbstate_t *state)
{
  wchar_t dummy;
  int ret;

  if (s == NULL)
    return 0;  /* not state-dependent */

  if (n == 0)
    return -2;

  if (pwc == NULL)
    pwc = &dummy;

  if (state->__count == 0)
    {
      if (*(unsigned char *) s < 0x80)
	{
	  *pwc = *(unsigned char *) s;
	  return *s ? 1 : 0;
	}
      size_t cnt = min (n, 2);
      ret = MultiByteToWideChar (cp, MB_ERR_INVALID_CHARS, s, cnt, pwc, 1);
      if (ret)
	return cnt;
      if (n == 1)
	{
	  state->__count = n;
	  state->__value.__wchb[0] = *s;
	  return -2;
	}
      /* These Win32 functions are really crappy.  Assuming n is 2 but the
	 first byte is a singlebyte charcode, the function does not convert
	 that byte and return 1, rather it just returns 0.  So, what we do
	 here is to check if the first byte returns a valid value... */
      else if (MultiByteToWideChar (cp, MB_ERR_INVALID_CHARS, s, 1, pwc, 1))
	return 1;
      r->_errno = EILSEQ;
      return -1;
    }
  state->__value.__wchb[state->__count] = *s;
  ret = MultiByteToWideChar (cp, MB_ERR_INVALID_CHARS,
			     (const char *) state->__value.__wchb, 2, pwc, 1);
  if (!ret)
    {
      r->_errno = EILSEQ;
      return -1;
    }
  state->__count = 0;
  return 1;
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
  /* FIXME: See comment at start of file. */
  return __ascii_mbtowc (r, pwc, s, n, charset, state);
}

extern "C" int
__eucjp_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
		const char *charset, mbstate_t *state)
{
  /* See comment in __eucjp_wctomb above. */
  wchar_t dummy;
  int ret = 0;

  if (s == NULL)
    return 0;  /* not state-dependent */

  if (n == 0)
    return -2;

  if (pwc == NULL)
    pwc = &dummy;

  if (state->__count == 0)
    {
      if (*(unsigned char *) s < 0x80)
	{
	  *pwc = *(unsigned char *) s;
	  return *s ? 1 : 0;
	}
      if (*(unsigned char *) s == 0x8f)	/* JIS-X-0212 lead byte? */
	{
	  /* Yes.  Store sequence in mbstate and handle in the __count != 0
	     case at the end of the function. */
	  size_t i;
	  for (i = 0; i < 3 && i < n; i++)
	    state->__value.__wchb[i] = s[i];
	  if ((state->__count = i) < 3)	/* Incomplete sequence? */
	    return -2;
	  ret = 3;
	  goto jis_x_0212;
	}
      size_t cnt = min (n, 2);
      if (MultiByteToWideChar (20932, MB_ERR_INVALID_CHARS, s, cnt, pwc, 1))
	return cnt;
      if (n == 1)
	{
	  state->__count = 1;
	  state->__value.__wchb[0] = *s;
	  return -2;
	}
      else if (MultiByteToWideChar (20932, MB_ERR_INVALID_CHARS, s, 1, pwc, 1))
	return 1;
      r->_errno = EILSEQ;
      return -1;
    }
  state->__value.__wchb[state->__count++] = *s;
  ret = 1;
jis_x_0212:
  if (state->__value.__wchb[0] == 0x8f)
    {
      if (state->__count == 2)
	{
	  if (n == 1)
	    return -2;
	  state->__value.__wchb[state->__count] = s[1];
	  ret = 2;
	}
      /* Ok, we have a full JIS-X-0212 sequence in mbstate.  Convert it
	 to the CP 20932 representation and feed it to MultiByteToWideChar. */
      state->__value.__wchb[0] = state->__value.__wchb[1];
      state->__value.__wchb[1] = state->__value.__wchb[2] & 0x7f;
    }
  if (!MultiByteToWideChar (20932, MB_ERR_INVALID_CHARS,
			    (const char *) state->__value.__wchb, 2, pwc, 1))
    {
      r->_errno = EILSEQ;
      return -1;
    }
  state->__count = 0;
  return ret;
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
  return __db_mbtowc (r, pwc, s, n, 949, state);
}

extern "C" int
__big5_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	       const char *charset, mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 950, state);
}

/* Convert Windows codepage to a setlocale compatible character set code.
   Called from newlib's setlocale() with codepage set to 0, if the
   charset isn't given explicitely in the POSIX compatible locale specifier.
   The function also returns a pointer to the corresponding _mbtowc_r
   function. */
extern "C" mbtowc_p
__set_charset_from_codepage (UINT cp, char *charset)
{
  if (cp == 0)
    cp = GetACP ();
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
    case 20866:
    case 21866:
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
    case 20932:
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
     an ASCII CAN (0x18, Ctrl-X), followed by the UTF-8 representation of the
     character.  The sys_(cp_)mbstowcs function detects ASCII CAN characters
     in the input multibyte string and converts the following multibyte
     sequence in by treating it as an UTF-8 char.  If that fails, the ASCII
     CAN was probably standalone and it gets just copied over as ASCII CAN.

   - The functions always create 0-terminated results, no matter what.
     If the result is truncated due to buffer size, it's a bug in Cygwin
     and the buffer in the calling function should be raised. */
size_t __stdcall
sys_cp_wcstombs (wctomb_p f_wctomb, const char *charset, char *dst, size_t len,
		 const wchar_t *src, size_t nwc)
{
  char buf[10];
  char *ptr = dst;
  wchar_t *pwcs = (wchar_t *) src;
  size_t n = 0;
  mbstate_t ps;
  save_errno save;

  memset (&ps, 0, sizeof ps);
  if (dst == NULL)
    len = (size_t) -1;
  while (n < len && nwc-- > 0)
    {
      wchar_t pw = *pwcs;
      int bytes;
      unsigned char cwc;

      /* Convert UNICODE private use area.  Reverse functionality for the
         ASCII area <= 0x7f (only for path names) is transform_chars above.
	 Reverse functionality for invalid bytes in a multibyte sequence is
	 in sys_cp_mbstowcs below. */
      if ((pw & 0xff00) == 0xf000
	  && (((cwc = (pw & 0xff)) <= 0x7f && tfx_chars[cwc] >= 0xf000)
	      || (cwc >= 0x80 && MB_CUR_MAX > 1)))
	{
	  buf[0] = (char) cwc;
	  bytes = 1;
	}
      else
	{
	  bytes = f_wctomb (_REENT, buf, pw, charset, &ps);
	  if (bytes == -1 && *charset != 'U'/*TF-8*/)
	    {
	      /* Convert chars invalid in the current codepage to a sequence
		 ASCII CAN; UTF-8 representation of invalid char. */
	      buf[0] = 0x18; /* ASCII CAN */
	      bytes = __utf8_wctomb (_REENT, buf + 1, pw, charset, &ps);
	      if (bytes == -1)
		{
		  ++pwcs;
		  ps.__count = 0;
		  continue;
		}
	      ++bytes; /* Add the ASCII CAN to the byte count. */
	      if (ps.__count == -4 && nwc > 0)
		{
		  /* First half of a surrogate pair. */
		  ++pwcs;
		  if ((*pwcs & 0xfc00) != 0xdc00) /* Invalid second half. */
		    {
		      ++pwcs;
		      ps.__count = 0;
		      continue;
		    }
		  bytes += __utf8_wctomb (_REENT, buf + bytes, *pwcs, charset,
					  &ps);
		  nwc--;
		}
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

size_t __stdcall
sys_wcstombs (char *dst, size_t len, const wchar_t * src, size_t nwc)
{
  return sys_cp_wcstombs (cygheap->locale.wctomb, cygheap->locale.charset,
			  dst, len, src, nwc);
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
sys_wcstombs_alloc (char **dst_p, int type, const wchar_t *src, size_t nwc)
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
sys_cp_mbstowcs (mbtowc_p f_mbtowc, const char *charset, wchar_t *dst,
		 size_t dlen, const char *src, size_t nms)
{
  wchar_t *ptr = dst;
  unsigned const char *pmbs = (unsigned const char *) src;
  size_t count = 0;
  size_t len = dlen;
  int bytes;
  mbstate_t ps;
  save_errno save;

  memset (&ps, 0, sizeof ps);
  if (dst == NULL)
    len = (size_t)-1;
  while (len > 0 && nms > 0)
    {
      /* ASCII CAN handling. */
      if (*pmbs == 0x18)
	{
	  /* Sanity check: If this is a lead CAN byte for a following UTF-8
	     sequence, there must be at least two more bytes left, and the
	     next byte must be a valid UTF-8 start byte.  If the charset
	     isn't UTF-8 anyway, try to convert the following bytes as UTF-8
	     sequence. */
	  if (nms > 2 && pmbs[1] >= 0xc2 && pmbs[1] <= 0xf4 && *charset != 'U'/*TF-8*/)
	    {
	      bytes = __utf8_mbtowc (_REENT, ptr, (const char *) pmbs + 1,
				     nms - 1, charset, &ps);
	      if (bytes < 0)
		{
		  /* Invalid UTF-8 sequence?  Treat the ASCII CAN character as
		     stand-alone ASCII CAN char. */
		  bytes = 1;
		  if (dst)
		    *ptr = 0x18;
		  memset (&ps, 0, sizeof ps);
		}
	      else
		{
		  ++bytes; /* Count CAN byte */
		  if (bytes > 1 && ps.__count == 4)
		    {
		      /* First half of a surrogate. */
		      wchar_t *ptr2 = dst ? ptr + 1 : NULL;
		      int bytes2 = __utf8_mbtowc (_REENT, ptr2,
						  (const char *) pmbs + bytes,
						  nms - bytes, charset, &ps);
		      if (bytes2 < 0)
			memset (&ps, 0, sizeof ps);
		      else
			{
			  bytes += bytes2;
			  ++count;
			  ptr = dst ? ptr + 1 : NULL;
			  --len;
			}
		    }
		}
	    }
	  /* Otherwise it's just a simple ASCII CAN. */
	  else
	    {
	      bytes = 1;
	      if (dst)
		*ptr = 0x18;
	    }
	}
      else if ((bytes = f_mbtowc (_REENT, ptr, (const char *) pmbs, nms,
				  charset, &ps)) < 0)
	{
	  /* The technique is based on a discussion here:
	     http://www.mail-archive.com/linux-utf8@nl.linux.org/msg00080.html

	     Invalid bytes in a multibyte secuence are converted to
	     the private use area which is already used to store ASCII
	     chars invalid in Windows filenames.  This technque allows 
	     to store them in a symmetric way. */
	  bytes = 1;
	  if (dst)
	    *ptr = L'\xf000' | *pmbs;
	  memset (&ps, 0, sizeof ps);
	}

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

size_t __stdcall
sys_mbstowcs (wchar_t * dst, size_t dlen, const char *src, size_t nms)
{
  return sys_cp_mbstowcs (cygheap->locale.mbtowc, cygheap->locale.charset,
			  dst, dlen, src, nms);
}

/* Same as sys_wcstombs_alloc, just backwards. */
size_t __stdcall
sys_mbstowcs_alloc (wchar_t **dst_p, int type, const char *src, size_t nms)
{
  size_t ret;

  ret = sys_mbstowcs (NULL, (size_t) -1, src, nms);
  if (ret > 0)
    {
      size_t dlen = ret + 1;

      if (type == HEAP_NOTHEAP)
	*dst_p = (wchar_t *) calloc (dlen, sizeof (wchar_t));
      else
	*dst_p = (wchar_t *) ccalloc ((cygheap_types) type, dlen,
				      sizeof (wchar_t));
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
  wchar_t *end = (PWCHAR) ((PBYTE) dest->Buffer + len);
  register PWCHAR p = end + 16;
  while (p-- > end)
    {
      *p = hex_wchars[value & 0xf];
      value >>= 4;
    }
  dest->Length += 16 * sizeof (WCHAR);
  return STATUS_SUCCESS;
}
