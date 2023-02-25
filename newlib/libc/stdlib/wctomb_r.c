#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "mbctype.h"
#include "local.h"

int
_wctomb_r (struct _reent *r,
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  return __WCTOMB (r, s, _wchar, state);
}

int
__ascii_wctomb (struct _reent *r,
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  /* Avoids compiler warnings about comparisons that are always false
     due to limited range when sizeof(wchar_t) is 2 but sizeof(wint_t)
     is 4, as is the case on cygwin.  */
  wint_t wchar = _wchar;

  if (s == NULL)
    return 0;
 
#ifdef __CYGWIN__
  if ((size_t)wchar >= 0x80)
#else
  if ((size_t)wchar >= 0x100)
#endif
    {
      _REENT_ERRNO(r) = EILSEQ;
      return -1;
    }

  *s = (char) wchar;
  return 1;
}

#ifdef _MB_CAPABLE
/* for some conversions, we use the __count field as a place to store a state value */
#define __state __count

int
__utf8_wctomb (struct _reent *r,
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  wint_t wchar = _wchar;
  int ret = 0;

  if (s == NULL)
    return 0; /* UTF-8 encoding is not state-dependent */

  if (sizeof (wchar_t) == 2 && state->__count == -4
      && (wchar < 0xdc00 || wchar > 0xdfff))
    {
      /* There's a leftover lone high surrogate.  Write out the CESU-8 value
	 of the surrogate and proceed to convert the given character.  Note
	 to return extra 3 bytes. */
      wchar_t tmp;
      tmp = (state->__value.__wchb[0] << 16 | state->__value.__wchb[1] << 8)
	    - (0x10000 >> 10 | 0xd80d);
      *s++ = 0xe0 | ((tmp & 0xf000) >> 12);
      *s++ = 0x80 | ((tmp &  0xfc0) >> 6);
      *s++ = 0x80 |  (tmp &   0x3f);
      state->__count = 0;
      ret = 3;
    }
  if (wchar <= 0x7f)
    {
      *s = wchar;
      return ret + 1;
    }
  if (wchar >= 0x80 && wchar <= 0x7ff)
    {
      *s++ = 0xc0 | ((wchar & 0x7c0) >> 6);
      *s   = 0x80 |  (wchar &  0x3f);
      return ret + 2;
    }
  if (wchar >= 0x800 && wchar <= 0xffff)
    {
      /* No UTF-16 surrogate handling in UCS-4 */
      if (sizeof (wchar_t) == 2 && wchar >= 0xd800 && wchar <= 0xdfff)
	{
	  wint_t tmp;
	  if (wchar <= 0xdbff)
	    {
	      /* First half of a surrogate pair.  Store the state and
	         return ret + 0. */
	      tmp = ((wchar & 0x3ff) << 10) + 0x10000;
	      state->__value.__wchb[0] = (tmp >> 16) & 0xff;
	      state->__value.__wchb[1] = (tmp >> 8) & 0xff;
	      state->__count = -4;
	      *s = (0xf0 | ((tmp & 0x1c0000) >> 18));
	      return ret;
	    }
	  if (state->__count == -4)
	    {
	      /* Second half of a surrogate pair.  Reconstruct the full
		 Unicode value and return the trailing three bytes of the
		 UTF-8 character. */
	      tmp = (state->__value.__wchb[0] << 16)
		    | (state->__value.__wchb[1] << 8)
		    | (wchar & 0x3ff);
	      state->__count = 0;
	      *s++ = 0xf0 | ((tmp & 0x1c0000) >> 18);
	      *s++ = 0x80 | ((tmp &  0x3f000) >> 12);
	      *s++ = 0x80 | ((tmp &    0xfc0) >> 6);
	      *s   = 0x80 |  (tmp &     0x3f);
	      return 4;
	    }
	  /* Otherwise translate into CESU-8 value. */
	}
      *s++ = 0xe0 | ((wchar & 0xf000) >> 12);
      *s++ = 0x80 | ((wchar &  0xfc0) >> 6);
      *s   = 0x80 |  (wchar &   0x3f);
      return ret + 3;
    }
  if (wchar >= 0x10000 && wchar <= 0x10ffff)
    {
      *s++ = 0xf0 | ((wchar & 0x1c0000) >> 18);
      *s++ = 0x80 | ((wchar &  0x3f000) >> 12);
      *s++ = 0x80 | ((wchar &    0xfc0) >> 6);
      *s   = 0x80 |  (wchar &     0x3f);
      return 4;
    }

  _REENT_ERRNO(r) = EILSEQ;
  return -1;
}

/* Cygwin defines its own doublebyte charset conversion functions 
   because the underlying OS requires wchar_t == UTF-16. */
#ifndef __CYGWIN__
int
__sjis_wctomb (struct _reent *r,
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  wint_t wchar = _wchar;

  unsigned char char2 = (unsigned char)wchar;
  unsigned char char1 = (unsigned char)(wchar >> 8);

  if (s == NULL)
    return 0;  /* not state-dependent */

  if (char1 != 0x00)
    {
    /* first byte is non-zero..validate multi-byte char */
      if (_issjis1(char1) && _issjis2(char2)) 
	{
	  *s++ = (char)char1;
	  *s = (char)char2;
	  return 2;
	}
      else
	{
	  _REENT_ERRNO(r) = EILSEQ;
	  return -1;
	}
    }
  *s = (char) wchar;
  return 1;
}

int
__eucjp_wctomb (struct _reent *r,
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  wint_t wchar = _wchar;
  unsigned char char2 = (unsigned char)wchar;
  unsigned char char1 = (unsigned char)(wchar >> 8);

  if (s == NULL)
    return 0;  /* not state-dependent */

  if (char1 != 0x00)
    {
    /* first byte is non-zero..validate multi-byte char */
      if (_iseucjp1 (char1) && _iseucjp2 (char2)) 
	{
	  *s++ = (char)char1;
	  *s = (char)char2;
	  return 2;
	}
      else if (_iseucjp2 (char1) && _iseucjp2 (char2 | 0x80))
	{
	  *s++ = (char)0x8f;
	  *s++ = (char)char1;
	  *s = (char)(char2 | 0x80);
	  return 3;
	}
      else
	{
	  _REENT_ERRNO(r) = EILSEQ;
	  return -1;
	}
    }
  *s = (char) wchar;
  return 1;
}

int
__jis_wctomb (struct _reent *r,
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  wint_t wchar = _wchar;
  int cnt = 0; 
  unsigned char char2 = (unsigned char)wchar;
  unsigned char char1 = (unsigned char)(wchar >> 8);

  if (s == NULL)
    return 1;  /* state-dependent */

  if (char1 != 0x00)
    {
    /* first byte is non-zero..validate multi-byte char */
      if (_isjis (char1) && _isjis (char2)) 
	{
	  if (state->__state == 0)
	    {
	      /* must switch from ASCII to JIS state */
	      state->__state = 1;
	      *s++ = ESC_CHAR;
	      *s++ = '$';
	      *s++ = 'B';
	      cnt = 3;
	    }
	  *s++ = (char)char1;
	  *s = (char)char2;
	  return cnt + 2;
	}
      _REENT_ERRNO(r) = EILSEQ;
      return -1;
    }
  if (state->__state != 0)
    {
      /* must switch from JIS to ASCII state */
      state->__state = 0;
      *s++ = ESC_CHAR;
      *s++ = '(';
      *s++ = 'B';
      cnt = 3;
    }
  *s = (char)char2;
  return cnt + 1;
}
#endif /* !__CYGWIN__ */

#ifdef _MB_EXTENDED_CHARSETS_ISO
static int
___iso_wctomb (struct _reent *r, char *s, wchar_t _wchar, int iso_idx,
	       mbstate_t *state)
{
  wint_t wchar = _wchar;

  if (s == NULL)
    return 0;

  /* wchars <= 0x9f translate to all ISO charsets directly. */
  if (wchar >= 0xa0)
    {
      if (iso_idx >= 0)
	{
	  unsigned char mb;

	  for (mb = 0; mb < 0x60; ++mb)
	    if (__iso_8859_conv[iso_idx][mb] == wchar)
	      {
		*s = (char) (mb + 0xa0);
		return 1;
	      }
	  _REENT_ERRNO(r) = EILSEQ;
	  return -1;
	}
    }
 
  if ((size_t)wchar >= 0x100)
    {
      _REENT_ERRNO(r) = EILSEQ;
      return -1;
    }

  *s = (char) wchar;
  return 1;
}

int __iso_8859_1_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, -1, state);
}

int __iso_8859_2_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 0, state);
}

int __iso_8859_3_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 1, state);
}

int __iso_8859_4_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 2, state);
}

int __iso_8859_5_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 3, state);
}

int __iso_8859_6_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 4, state);
}

int __iso_8859_7_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 5, state);
}

int __iso_8859_8_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 6, state);
}

int __iso_8859_9_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 7, state);
}

int __iso_8859_10_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 8, state);
}

int __iso_8859_11_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 9, state);
}

int __iso_8859_13_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 10, state);
}

int __iso_8859_14_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 11, state);
}

int __iso_8859_15_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 12, state);
}

int __iso_8859_16_wctomb (struct _reent *r, char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (r, s, _wchar, 13, state);
}

static wctomb_p __iso_8859_wctomb[17] = {
  NULL,
  __iso_8859_1_wctomb,
  __iso_8859_2_wctomb,
  __iso_8859_3_wctomb,
  __iso_8859_4_wctomb,
  __iso_8859_5_wctomb,
  __iso_8859_6_wctomb,
  __iso_8859_7_wctomb,
  __iso_8859_8_wctomb,
  __iso_8859_9_wctomb,
  __iso_8859_10_wctomb,
  __iso_8859_11_wctomb,
  NULL,			/* No ISO 8859-12 */
  __iso_8859_13_wctomb,
  __iso_8859_14_wctomb,
  __iso_8859_15_wctomb,
  __iso_8859_16_wctomb
};

/* val *MUST* be valid!  All checks for validity are supposed to be
   performed before calling this function. */
wctomb_p
__iso_wctomb (int val)
{
  return __iso_8859_wctomb[val];
}
#endif /* _MB_EXTENDED_CHARSETS_ISO */

#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
static int
___cp_wctomb (struct _reent *r, char *s, wchar_t _wchar, int cp_idx,
	      mbstate_t *state)
{
  wint_t wchar = _wchar;

  if (s == NULL)
    return 0;

  if (wchar >= 0x80)
    {
      if (cp_idx >= 0)
	{
	  unsigned char mb;

	  for (mb = 0; mb < 0x80; ++mb)
	    if (__cp_conv[cp_idx][mb] == wchar)
	      {
		*s = (char) (mb + 0x80);
		return 1;
	      }
	  _REENT_ERRNO(r) = EILSEQ;
	  return -1;
	}
    }

  if ((size_t)wchar >= 0x100)
    {
      _REENT_ERRNO(r) = EILSEQ;
      return -1;
    }

  *s = (char) wchar;
  return 1;
}

static int
__cp_437_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 0, state);
}

static int
__cp_720_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 1, state);
}

static int
__cp_737_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 2, state);
}

static int
__cp_775_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 3, state);
}

static int
__cp_850_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 4, state);
}

static int
__cp_852_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 5, state);
}

static int
__cp_855_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 6, state);
}

static int
__cp_857_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 7, state);
}

static int
__cp_858_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 8, state);
}

static int
__cp_862_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 9, state);
}

static int
__cp_866_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 10, state);
}

static int
__cp_874_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 11, state);
}

static int
__cp_1125_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 12, state);
}

static int
__cp_1250_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 13, state);
}

static int
__cp_1251_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 14, state);
}

static int
__cp_1252_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 15, state);
}

static int
__cp_1253_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 16, state);
}

static int
__cp_1254_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 17, state);
}

static int
__cp_1255_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 18, state);
}

static int
__cp_1256_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 19, state);
}

static int
__cp_1257_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 20, state);
}

static int
__cp_1258_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 21, state);
}

static int
__cp_20866_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 22, state);
}

static int
__cp_21866_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 23, state);
}

static int
__cp_101_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 24, state);
}

static int
__cp_102_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 25, state);
}

static int
__cp_103_wctomb (struct _reent *r, char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (r, s, _wchar, 26, state);
}

static wctomb_p __cp_xxx_wctomb[27] = {
  __cp_437_wctomb,
  __cp_720_wctomb,
  __cp_737_wctomb,
  __cp_775_wctomb,
  __cp_850_wctomb,
  __cp_852_wctomb,
  __cp_855_wctomb,
  __cp_857_wctomb,
  __cp_858_wctomb,
  __cp_862_wctomb,
  __cp_866_wctomb,
  __cp_874_wctomb,
  __cp_1125_wctomb,
  __cp_1250_wctomb,
  __cp_1251_wctomb,
  __cp_1252_wctomb,
  __cp_1253_wctomb,
  __cp_1254_wctomb,
  __cp_1255_wctomb,
  __cp_1256_wctomb,
  __cp_1257_wctomb,
  __cp_1258_wctomb,
  __cp_20866_wctomb,
  __cp_21866_wctomb,
  __cp_101_wctomb,
  __cp_102_wctomb,
  __cp_103_wctomb,
};

/* val *MUST* be valid!  All checks for validity are supposed to be
   performed before calling this function. */
wctomb_p
__cp_wctomb (int val)
{
  return __cp_xxx_wctomb[__cp_val_index (val)];
}
#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
#endif /* _MB_CAPABLE */
