/* strfuncs.cc: string functions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/param.h>
#include <wchar.h>
#include <ntdll.h>
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"

/* Transform characters invalid for Windows filenames to the Unicode private
   use area in the U+f0XX range.  The affected characters are all control
   chars 1 <= c <= 31, as well as the characters " * : < > ? |.  The backslash
   is affected as well, but we can't transform it as long as we accept Win32
   paths as input. */
static const WCHAR tfx_chars[] = {
 0xf000 |   0, 0xf000 |   1, 0xf000 |   2, 0xf000 |   3,
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

/* This is the table for the reverse functionality in sys_wcstombs.
   It differs deliberately in two code places (space and dot) to allow
   converting back space and dot on filesystems only supporting DOS
   filenames. */
static const WCHAR tfx_rev_chars[] = {
 0xf000 |   0, 0xf000 |   1, 0xf000 |   2, 0xf000 |   3,
 0xf000 |   4, 0xf000 |   5, 0xf000 |   6, 0xf000 |   7,
 0xf000 |   8, 0xf000 |   9, 0xf000 |  10, 0xf000 |  11,
 0xf000 |  12, 0xf000 |  13, 0xf000 |  14, 0xf000 |  15,
 0xf000 |  16, 0xf000 |  17, 0xf000 |  18, 0xf000 |  19,
 0xf000 |  20, 0xf000 |  21, 0xf000 |  22, 0xf000 |  23,
 0xf000 |  24, 0xf000 |  25, 0xf000 |  26, 0xf000 |  27,
 0xf000 |  28, 0xf000 |  29, 0xf000 |  30, 0xf000 |  31,
 0xf000 | ' ',          '!', 0xf000 | '"',          '#',
	  '$',          '%',          '&',           39,
	  '(',          ')', 0xf000 | '*',          '+',
	  ',',          '-', 0xf000 | '.',          '\\',
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

PWCHAR
transform_chars_af_unix (PWCHAR out, const char *path, __socklen_t len)
{
  len -= sizeof (__sa_family_t);
  for (const unsigned char *p = (const unsigned char *) path; len-- > 0; ++p)
    *out++ = (*p <= 0x7f) ? tfx_chars[*p] : *p;
  return out;
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
__sjis_wctomb (struct _reent *r, char *s, wchar_t wchar, mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 932);
}

extern "C" int
__eucjp_wctomb (struct _reent *r, char *s, wchar_t wchar, mbstate_t *state)
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
__gbk_wctomb (struct _reent *r, char *s, wchar_t wchar, mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 936);
}

extern "C" int
__kr_wctomb (struct _reent *r, char *s, wchar_t wchar, mbstate_t *state)
{
  return __db_wctomb (r,s, wchar, 949);
}

extern "C" int
__big5_wctomb (struct _reent *r, char *s, wchar_t wchar, mbstate_t *state)
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
      size_t cnt = MIN (n, 2);
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
	       mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 932, state);
}

extern "C" int
__eucjp_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
		mbstate_t *state)
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
      size_t cnt = MIN (n, 2);
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
	      mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 936, state);
}

extern "C" int
__kr_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	     mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 949, state);
}

extern "C" int
__big5_mbtowc (struct _reent *r, wchar_t *pwc, const char *s, size_t n,
	       mbstate_t *state)
{
  return __db_mbtowc (r, pwc, s, n, 950, state);
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

   - Three cases have to be distinguished for the return value:

     - dst == NULL; len is ignored, the return value is the number of bytes
       required for the string without the trailing NUL, just like the return
       value of the wcstombs function.

     - dst != NULL, len == (size_t) -1; the return value is the size in bytes
       of the destination string without the trailing NUL.  If the incoming
       wide char string was not NUL-terminated, the target string won't be
       NUL-terminated either.

     - dst != NULL; len != (size_t) -1; the return value is the size in bytes
       of the destination string without the trailing NUL.  The target string
       will be NUL-terminated, no matter what.  If the result is truncated due
       to buffer size, it's a bug in Cygwin and the buffer in the calling
       function should be raised.
*/
static size_t __reg3
sys_wcstombs (char *dst, size_t len, const wchar_t *src, size_t nwc,
	      bool is_path)
{
  char buf[10];
  char *ptr = dst;
  wchar_t *pwcs = (wchar_t *) src;
  size_t n = 0;
  mbstate_t ps;
  save_errno save;
  wctomb_p f_wctomb = __WCTOMB;

  if (f_wctomb == __ascii_wctomb)
    f_wctomb = __utf8_wctomb;
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
      if (is_path && (pw & 0xff00) == 0xf000
	  && (((cwc = (pw & 0xff)) <= 0x7f && tfx_rev_chars[cwc] >= 0xf000)
	      || (cwc >= 0x80 && MB_CUR_MAX > 1)))
	{
	  buf[0] = (char) cwc;
	  bytes = 1;
	}
      else
	{
	  bytes = f_wctomb (_REENT, buf, pw, &ps);
	  if (bytes == -1 && f_wctomb != __utf8_wctomb)
	    {
	      /* Convert chars invalid in the current codepage to a sequence
		 ASCII CAN; UTF-8 representation of invalid char. */
	      buf[0] = 0x18; /* ASCII CAN */
	      bytes = __utf8_wctomb (_REENT, buf + 1, pw, &ps);
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
		  bytes += __utf8_wctomb (_REENT, buf + bytes, *pwcs, &ps);
		  nwc--;
		}
	    }
	}
      if (n + bytes <= len)
	{
	  if (dst)
	    {
	      for (int i = 0; i < bytes; ++i)
		*ptr++ = buf[i];
	    }
	  if (*pwcs++ == 0x00)
	    break;
	  n += bytes;
	}
      else
	break;
    }
  if (n && dst && len != (size_t) -1)
    {
      n = (n < len) ? n : len - 1;
      dst[n] = '\0';
    }

  return n;
}

size_t __reg3
sys_wcstombs (char *dst, size_t len, const wchar_t * src, size_t nwc)
{
  return sys_wcstombs (dst, len, src, nwc, true);
}

size_t __reg3
sys_wcstombs_no_path (char *dst, size_t len, const wchar_t * src, size_t nwc)
{
  return sys_wcstombs (dst, len, src, nwc, false);
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
static size_t __reg3
sys_wcstombs_alloc (char **dst_p, int type, const wchar_t *src, size_t nwc,
		bool is_path)
{
  size_t ret;

  ret = sys_wcstombs (NULL, (size_t) -1, src, nwc, is_path);
  if (ret > 0)
    {
      size_t dlen = ret + 1;

      if (type == HEAP_NOTHEAP)
	*dst_p = (char *) calloc (dlen, sizeof (char));
      else
	*dst_p = (char *) ccalloc ((cygheap_types) type, dlen, sizeof (char));
      if (!*dst_p)
	return 0;
      ret = sys_wcstombs (*dst_p, dlen, src, nwc, is_path);
    }
  return ret;
}

size_t __reg3
sys_wcstombs_alloc (char **dst_p, int type, const wchar_t *src, size_t nwc)
{
  return sys_wcstombs_alloc (dst_p, type, src, nwc, true);
}

size_t __reg3
sys_wcstombs_alloc_no_path (char **dst_p, int type, const wchar_t *src,
		size_t nwc)
{
  return sys_wcstombs_alloc (dst_p, type, src, nwc, false);
}

/* sys_cp_mbstowcs is actually most of the time called as sys_mbstowcs with
   a 0 codepage.  If cp is not 0, the codepage is evaluated and used for the
   conversion.  This is so that fhandler_console can switch to an alternate
   charset, which is the charset returned by GetConsoleCP ().  Most of the
   time this is used for box and line drawing characters. */
size_t __reg3
sys_cp_mbstowcs (mbtowc_p f_mbtowc, wchar_t *dst, size_t dlen,
		 const char *src, size_t nms)
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
	  if (nms > 2 && pmbs[1] >= 0xc2 && pmbs[1] <= 0xf4
	      && f_mbtowc != __utf8_mbtowc)
	    {
	      bytes = __utf8_mbtowc (_REENT, ptr, (const char *) pmbs + 1,
				     nms - 1, &ps);
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
						  nms - bytes, &ps);
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
				  &ps)) < 0)
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

size_t __reg3
sys_mbstowcs (wchar_t * dst, size_t dlen, const char *src, size_t nms)
{
  mbtowc_p f_mbtowc = __MBTOWC;
  if (f_mbtowc == __ascii_mbtowc)
    f_mbtowc = __utf8_mbtowc;
  return sys_cp_mbstowcs (f_mbtowc, dst, dlen, src, nms);
}

/* Same as sys_wcstombs_alloc, just backwards. */
size_t __reg3
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

/* Copy string, until c or <nul> is encountered.
   NUL-terminate the destination string (s1).
   Return pointer to terminating byte in dst string.  */
char * __stdcall
strccpy (char *__restrict s1, const char **__restrict s2, char c)
{
  while (**s2 && **s2 != c)
    *s1++ = *((*s2)++);
  *s1 = 0;

  return s1;
}

const unsigned char case_folded_lower[] = {
   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32, '!', '"', '#', '$', '%', '&',  39, '(', ')', '*', '+', ',', '-', '.', '/',
 '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
 '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[',  92, ']', '^', '_',
 '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

const unsigned char case_folded_upper[] = {
   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32, '!', '"', '#', '$', '%', '&',  39, '(', ')', '*', '+', ',', '-', '.', '/',
 '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
 '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[',  92, ']', '^', '_',
 '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

const char isalpha_array[] = {
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,   0,   0,   0,   0,   0,
   0,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

extern "C" int __stdcall
cygwin_wcscasecmp (const wchar_t *ws, const wchar_t *wt)
{
  UNICODE_STRING us, ut;

  RtlInitUnicodeString (&us, ws);
  RtlInitUnicodeString (&ut, wt);
  return RtlCompareUnicodeString (&us, &ut, TRUE);
}

extern "C" int __stdcall
cygwin_wcsncasecmp (const wchar_t  *ws, const wchar_t *wt, size_t n)
{
  UNICODE_STRING us, ut;
  size_t ls = 0, lt = 0;

  while (ws[ls] && ls < n)
    ++ls;
  RtlInitCountedUnicodeString (&us, ws, ls * sizeof (WCHAR));
  while (wt[lt] && lt < n)
    ++lt;
  RtlInitCountedUnicodeString (&ut, wt, lt * sizeof (WCHAR));
  return RtlCompareUnicodeString (&us, &ut, TRUE);
}

extern "C" int __stdcall
cygwin_strcasecmp (const char *cs, const char *ct)
{
  UNICODE_STRING us, ut;
  ULONG len;

  len = (strlen (cs) + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&us, (PWCHAR) alloca (len), len);
  us.Length = sys_mbstowcs (us.Buffer, us.MaximumLength, cs) * sizeof (WCHAR);
  len = (strlen (ct) + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&ut, (PWCHAR) alloca (len), len);
  ut.Length = sys_mbstowcs (ut.Buffer, ut.MaximumLength, ct) * sizeof (WCHAR);
  return RtlCompareUnicodeString (&us, &ut, TRUE);
}

extern "C" int __stdcall
cygwin_strncasecmp (const char *cs, const char *ct, size_t n)
{
  UNICODE_STRING us, ut;
  ULONG len;
  size_t ls = 0, lt = 0;

  while (cs[ls] && ls < n)
    ++ls;
  len = (ls + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&us, (PWCHAR) alloca (len), len);
  us.Length = sys_mbstowcs (us.Buffer, ls + 1, cs, ls) * sizeof (WCHAR);
  while (ct[lt] && lt < n)
    ++lt;
  len = (lt + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&ut, (PWCHAR) alloca (len), len);
  ut.Length = sys_mbstowcs (ut.Buffer, lt + 1, ct, lt)  * sizeof (WCHAR);
  return RtlCompareUnicodeString (&us, &ut, TRUE);
}

extern "C" char *
strlwr (char *string)
{
  UNICODE_STRING us;
  size_t len = (strlen (string) + 1) * sizeof (WCHAR);

  us.MaximumLength = len; us.Buffer = (PWCHAR) alloca (len);
  us.Length = sys_mbstowcs (us.Buffer, len, string) * sizeof (WCHAR)
	      - sizeof (WCHAR);
  RtlDowncaseUnicodeString (&us, &us, FALSE);
  sys_wcstombs (string, len / sizeof (WCHAR), us.Buffer);
  return string;
}

extern "C" char *
strupr (char *string)
{
  UNICODE_STRING us;
  size_t len = (strlen (string) + 1) * sizeof (WCHAR);

  us.MaximumLength = len; us.Buffer = (PWCHAR) alloca (len);
  us.Length = sys_mbstowcs (us.Buffer, len, string) * sizeof (WCHAR)
	      - sizeof (WCHAR);
  RtlUpcaseUnicodeString (&us, &us, FALSE);
  sys_wcstombs (string, len / sizeof (WCHAR), us.Buffer);
  return string;
}

/* backslashify: Convert all forward slashes in src path to back slashes
   in dst path.  Add a trailing slash to dst when trailing_slash_p arg
   is set to 1. */

void
backslashify (const char *src, char *dst, bool trailing_slash_p)
{
  const char *start = src;

  while (*src)
    {
      if (*src == '/')
	*dst++ = '\\';
      else
	*dst++ = *src;
      ++src;
    }
  if (trailing_slash_p
      && src > start
      && !isdirsep (src[-1]))
    *dst++ = '\\';
  *dst++ = 0;
}

/* slashify: Convert all back slashes in src path to forward slashes
   in dst path.  Add a trailing slash to dst when trailing_slash_p arg
   is set to 1. */

void
slashify (const char *src, char *dst, bool trailing_slash_p)
{
  const char *start = src;

  while (*src)
    {
      if (*src == '\\')
	*dst++ = '/';
      else
	*dst++ = *src;
      ++src;
    }
  if (trailing_slash_p
      && src > start
      && !isdirsep (src[-1]))
    *dst++ = '/';
  *dst++ = 0;
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
