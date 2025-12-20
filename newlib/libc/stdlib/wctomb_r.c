/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <stdint.h>
#include <endian.h>
#include "mbctype.h"
#include "local.h"
#include "../ctype/local.h"

int
__ascii_wctomb (
        char          *s,
        wchar_t        wchar,
        mbstate_t     *state)
{
  (void) state;
  if (s == NULL)
      return 0;

  if (wchar >= 0x80)
      return -1;

  *s = (char) wchar;
  return 1;
}

#ifdef __MB_CAPABLE

/* for some conversions, we use the __count field as a place to store a state value */
#define __state __count

int
__utf8_wctomb (
        char          *s,
        wchar_t        wchar,
        mbstate_t     *state)
{
  (void) state;         /* unused when wchar_t is 32-bits */
  if (s == NULL)
    {
#if __SIZEOF_WCHAR_T__ == 2
      state->__count = 0;       /* UTF-16 encoding is state-dependent */
#endif
      return 0;
    }

#if __SIZEOF_WCHAR_T__ == 2
  if (state->__count == -4)
    {
      state->__count = 0;
      /* Check for low surrogate */
      if (0xdc00 <= (uwchar_t)wchar && (uwchar_t)wchar <= 0xdfff)
        {
            uint32_t tmp;
            /* Second half of a surrogate pair.  Reconstruct the full
               Unicode value and return the trailing three bytes of the
               UTF-8 character. */
            tmp = state->__value.__ucs | (wchar & 0x3ff);
            *s++ = 0xf0 | ((tmp & 0x1c0000) >> 18);
            *s++ = 0x80 | ((tmp &  0x3f000) >> 12);
            *s++ = 0x80 | ((tmp &    0xfc0) >> 6);
            *s   = 0x80 |  (tmp &     0x3f);
            return 4;
        }
      /* Not a low surrogate */
      return -1;
    }
#endif
  if (wchar <= 0x7f)
    {
      *s = wchar;
      return 1;
    }
  if (wchar >= 0x80 && wchar <= 0x7ff)
    {
      *s++ = 0xc0 | ((wchar & 0x7c0) >> 6);
      *s   = 0x80 |  (wchar &  0x3f);
      return 2;
    }
  if (wchar >= 0x800
#if __SIZEOF_WCHAR_T__ > 2
      && (uwchar_t)wchar <= 0xffff
#endif
      )
    {
      if ((uwchar_t) wchar >= 0xd800 && (uwchar_t) wchar <= 0xdfff)
	{
#if __SIZEOF_WCHAR_T__ == 2
          if ((uwchar_t) wchar <= 0xdbff)
            {
              /* First half of a surrogate pair.  Store the state and
                 return 0. */
              state->__value.__ucs = ((uint32_t) (wchar & 0x3ff) << 10) + 0x10000;
              state->__count = -4;
              return 0;
            }
#endif
          /* Unexpected surrogate */
          return -1;
	}
      *s++ = 0xe0 | ((wchar & 0xf000) >> 12);
      *s++ = 0x80 | ((wchar &  0xfc0) >> 6);
      *s   = 0x80 |  (wchar &   0x3f);
      return 3;
    }
#if __SIZEOF_WCHAR_T__ == 4
  if ((uwchar_t)wchar >= (uwchar_t) 0x10000 && (uwchar_t) wchar <= (uwchar_t) 0x10ffff)
    {
      *s++ = 0xf0 | ((wchar & 0x1c0000) >> 18);
      *s++ = 0x80 | ((wchar &  0x3f000) >> 12);
      *s++ = 0x80 | ((wchar &    0xfc0) >> 6);
      *s   = 0x80 |  (wchar &     0x3f);
      return 4;
    }
#endif

  return -1;
}

#ifdef __MB_EXTENDED_CHARSETS_UCS

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define __ucs2le_wctomb __ucs2_wctomb
#define __ucs2be_wctomb __ucs2swap_wctomb
#define __ucs4le_wctomb __ucs4_wctomb
#define __ucs4be_wctomb __ucs4swap_wctomb
#else
#define __ucs2le_wctomb __ucs2swap_wctomb
#define __ucs2be_wctomb __ucs2_wctomb
#define __ucs4le_wctomb __ucs4swap_wctomb
#define __ucs4be_wctomb __ucs4_wctomb
#endif

static int
__ucs2_wctomb (
        char          *s,
        wchar_t        wchar,
        mbstate_t     *state)
{
    uint16_t    uchar = (uint16_t) wchar;

    (void) state;
    /* Surrogates are invalid in UCS-2 */
    if ((uwchar_t) wchar >= 0xd800 && (uwchar_t) wchar <= 0xdfff)
        return -1;

    memcpy((void *) s, &uchar, 2);
    return 2;
}

static int
__ucs2swap_wctomb (
        char          *s,
        wchar_t        wchar,
        mbstate_t     *state)
{
    uint16_t    uchar = __bswap16((uint16_t) wchar);

    (void) state;
    /* Surrogates are invalid in UCS-2 */
    if ((uwchar_t)wchar >= 0xd800 && (uwchar_t)wchar <= 0xdfff)
        return -1;

    memcpy((void *) s, &uchar, 2);
    return 2;
}

static int
__ucs4_wctomb (
        char          *s,
        wchar_t        wchar,
        mbstate_t     *state)
{
    uint32_t    uchar = (uint32_t) wchar;

    (void) state;
    /* Surrogates are invalid in UCS-4 */
    if ((uwchar_t)wchar >= 0xd800 && (uwchar_t)wchar <= 0xdfff)
        return -1;

    memcpy((void *) s, &uchar, 4);
    return 4;
}

static int
__ucs4swap_wctomb (
        char          *s,
        wchar_t        wchar,
        mbstate_t     *state)
{
    uint32_t    uchar = __bswap32((uint32_t) wchar);

    (void) state;
    /* Surrogates are invalid in UCS-4 */
    if ((uwchar_t)wchar >= 0xd800 && (uwchar_t)wchar <= 0xdfff)
        return -1;

    memcpy((void *) s, &uchar, 4);
    return 4;
}

#endif /* __MB_EXTENDED_CHARSETS_UCS */

#ifdef __MB_EXTENDED_CHARSETS_JIS

static int
__sjis_wctomb (
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  uint16_t jischar = __uc2jp((wint_t) _wchar, JP_SJIS);

  unsigned char char2 = (unsigned char)jischar;
  unsigned char char1 = (unsigned char)(jischar >> 8);

  (void) state;
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
	  return -1;
	}
    }
  *s = (char) jischar;
  return 1;
}

static int
__eucjp_wctomb (
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  uint16_t jischar = __uc2jp((wint_t) _wchar, JP_EUCJP);

  unsigned char char2 = (unsigned char)jischar;
  unsigned char char1 = (unsigned char)(jischar >> 8);

  (void) state;
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
	  return -1;
	}
    }
  *s = (char) jischar;
  return 1;
}

static int
__jis_wctomb (
        char          *s,
        wchar_t        _wchar,
        mbstate_t     *state)
{
  uint16_t jischar = __uc2jp((wint_t) _wchar, JP_JIS);
  unsigned char char2 = (unsigned char)jischar;
  unsigned char char1 = (unsigned char)(jischar >> 8);
  int cnt = 0;

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
#endif /* __MB_EXTENDED_CHARSETS_JIS */

#ifdef __MB_EXTENDED_CHARSETS_ISO

static int
___iso_wctomb (char *s, wchar_t _wchar, enum locale_id id,
	       mbstate_t *state)
{
  wint_t wchar = _wchar;

  (void) state;
  if (s == NULL)
    return 0;

  /* wchars <= 0x9f translate to all ISO charsets directly. */
  if (wchar >= 0xa0)
    {
        unsigned char mb;

        if ((uwchar_t) wchar > __iso_8859_max[id - locale_ISO_8859_2])
            return -1;

        for (mb = 0; mb < 0x60; ++mb)
	    if (__iso_8859_conv[id - locale_ISO_8859_2][mb] == (uwchar_t) wchar)
              {
		*s = (char) (mb + 0xa0);
		return 1;
              }
        return -1;
    }

  if ((size_t)wchar >= 0x100)
    {
      return -1;
    }

  *s = (char) wchar;
  return 1;
}

static int
__iso_8859_1_wctomb (char *s, wchar_t wchar, mbstate_t *state)
{
  (void) state;
  if (s == NULL)
    return 0;

  if (wchar >= 0x100)
    {
      return -1;
    }

  *s = (char) wchar;
  return 1;
}

static int __iso_8859_2_wctomb (char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_2, state);
}

static int __iso_8859_3_wctomb (char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_3, state);
}

static int __iso_8859_4_wctomb (char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_4, state);
}

static int __iso_8859_5_wctomb (char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_5, state);
}

static int __iso_8859_6_wctomb (char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_6, state);
}

static int __iso_8859_7_wctomb (char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_7, state);
}

static int __iso_8859_8_wctomb (char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_8, state);
}

static int __iso_8859_9_wctomb (char *s, wchar_t _wchar,
			 mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_9, state);
}

static int __iso_8859_10_wctomb (char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_10, state);
}

static int __iso_8859_11_wctomb (char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_11, state);
}

static int __iso_8859_13_wctomb (char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_13, state);
}

static int __iso_8859_14_wctomb (char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_14, state);
}

static int __iso_8859_15_wctomb (char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_15, state);
}

static int __iso_8859_16_wctomb (char *s, wchar_t _wchar,
			  mbstate_t *state)
{
  return ___iso_wctomb (s, _wchar, locale_ISO_8859_16, state);
}

#endif /* __MB_EXTENDED_CHARSETS_ISO */

#ifdef __MB_EXTENDED_CHARSETS_WINDOWS

#ifdef __AVR__
/* The avr compiler gets confused by the use of __cp_conv below */
#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wanalyzer-out-of-bounds"
#endif
#endif

static int
___cp_wctomb (char *s, wchar_t _wchar, int cp_idx,
	      mbstate_t *state)
{
  wint_t wchar = _wchar;

  (void) state;
  if (s == NULL)
    return 0;

  if (wchar >= 0x80)
    {
      if (cp_idx >= 0)
	{
	  unsigned char mb;

          if ((uwchar_t) wchar > __cp_max[cp_idx])
            return -1;

	  for (mb = 0; mb < 0x80; ++mb)
            if (__cp_conv[cp_idx][mb] == (uwchar_t) wchar)
	      {
		*s = (char) (mb + 0x80);
		return 1;
	      }
	  return -1;
        }
    }

  if ((size_t)wchar >= 0x100)
    {
      return -1;
    }

  *s = (char) wchar;
  return 1;
}

static int
__cp_437_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 0, state);
}

static int
__cp_720_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 1, state);
}

static int
__cp_737_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 2, state);
}

static int
__cp_775_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 3, state);
}

static int
__cp_850_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 4, state);
}

static int
__cp_852_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 5, state);
}

static int
__cp_855_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 6, state);
}

static int
__cp_857_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 7, state);
}

static int
__cp_858_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 8, state);
}

static int
__cp_862_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 9, state);
}

static int
__cp_866_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 10, state);
}

static int
__cp_874_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 11, state);
}

static int
__cp_1125_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 12, state);
}

static int
__cp_1250_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 13, state);
}

static int
__cp_1251_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 14, state);
}

static int
__cp_1252_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 15, state);
}

static int
__cp_1253_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 16, state);
}

static int
__cp_1254_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 17, state);
}

static int
__cp_1255_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 18, state);
}

static int
__cp_1256_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 19, state);
}

static int
__cp_1257_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 20, state);
}

static int
__cp_1258_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 21, state);
}

static int
__cp_20866_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 22, state);
}

static int
__cp_21866_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 23, state);
}

static int
__cp_101_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 24, state);
}

static int
__cp_102_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 25, state);
}

static int
__cp_103_wctomb (char *s, wchar_t _wchar, mbstate_t *state)
{
  return ___cp_wctomb (s, _wchar, 26, state);
}

#endif /* __MB_EXTENDED_CHARSETS_WINDOWS */

const wctomb_p __wctomb[locale_END - locale_BASE] = {
    [locale_C - locale_BASE] = __ascii_wctomb,
    [locale_UTF_8 - locale_BASE] = __utf8_wctomb,
#ifdef __MB_EXTENDED_CHARSETS_UCS
    [locale_UCS_2 - locale_BASE] = __ucs2_wctomb,
    [locale_UCS_2LE - locale_BASE] = __ucs2le_wctomb,
    [locale_UCS_2BE - locale_BASE] = __ucs2be_wctomb,
    [locale_UCS_4 - locale_BASE] = __ucs4_wctomb,
    [locale_UCS_4LE - locale_BASE] = __ucs4le_wctomb,
    [locale_UCS_4BE - locale_BASE] = __ucs4be_wctomb,
#endif
#ifdef __MB_EXTENDED_CHARSETS_ISO
    [locale_ISO_8859_1 - locale_BASE] = __iso_8859_1_wctomb,
    [locale_ISO_8859_2 - locale_BASE] = __iso_8859_2_wctomb,
    [locale_ISO_8859_3 - locale_BASE] = __iso_8859_3_wctomb,
    [locale_ISO_8859_4 - locale_BASE] = __iso_8859_4_wctomb,
    [locale_ISO_8859_5 - locale_BASE] = __iso_8859_5_wctomb,
    [locale_ISO_8859_6 - locale_BASE] = __iso_8859_6_wctomb,
    [locale_ISO_8859_7 - locale_BASE] = __iso_8859_7_wctomb,
    [locale_ISO_8859_8 - locale_BASE] = __iso_8859_8_wctomb,
    [locale_ISO_8859_9 - locale_BASE] = __iso_8859_9_wctomb,
    [locale_ISO_8859_10 - locale_BASE] = __iso_8859_10_wctomb,
    [locale_ISO_8859_11 - locale_BASE] = __iso_8859_11_wctomb,
    [locale_ISO_8859_13 - locale_BASE] = __iso_8859_13_wctomb,
    [locale_ISO_8859_14 - locale_BASE] = __iso_8859_14_wctomb,
    [locale_ISO_8859_15 - locale_BASE] = __iso_8859_15_wctomb,
    [locale_ISO_8859_16 - locale_BASE] = __iso_8859_16_wctomb,
#endif
#ifdef __MB_EXTENDED_CHARSETS_WINDOWS
    [locale_CP437 - locale_BASE] = __cp_437_wctomb,
    [locale_CP720 - locale_BASE] = __cp_720_wctomb,
    [locale_CP737 - locale_BASE] = __cp_737_wctomb,
    [locale_CP775 - locale_BASE] = __cp_775_wctomb,
    [locale_CP850 - locale_BASE] = __cp_850_wctomb,
    [locale_CP852 - locale_BASE] = __cp_852_wctomb,
    [locale_CP855 - locale_BASE] = __cp_855_wctomb,
    [locale_CP857 - locale_BASE] = __cp_857_wctomb,
    [locale_CP858 - locale_BASE] = __cp_858_wctomb,
    [locale_CP862 - locale_BASE] = __cp_862_wctomb,
    [locale_CP866 - locale_BASE] = __cp_866_wctomb,
    [locale_CP874 - locale_BASE] = __cp_874_wctomb,
    [locale_CP1125 - locale_BASE] = __cp_1125_wctomb,
    [locale_CP1250 - locale_BASE] = __cp_1250_wctomb,
    [locale_CP1251 - locale_BASE] = __cp_1251_wctomb,
    [locale_CP1252 - locale_BASE] = __cp_1252_wctomb,
    [locale_CP1253 - locale_BASE] = __cp_1253_wctomb,
    [locale_CP1254 - locale_BASE] = __cp_1254_wctomb,
    [locale_CP1255 - locale_BASE] = __cp_1255_wctomb,
    [locale_CP1256 - locale_BASE] = __cp_1256_wctomb,
    [locale_CP1257 - locale_BASE] = __cp_1257_wctomb,
    [locale_CP1258 - locale_BASE] = __cp_1258_wctomb,
    [locale_KOI8_R - locale_BASE] = __cp_20866_wctomb,
    [locale_KOI8_U - locale_BASE] = __cp_21866_wctomb,
    [locale_GEORGIAN_PS - locale_BASE] = __cp_101_wctomb,
    [locale_PT154 - locale_BASE] = __cp_102_wctomb,
    [locale_KOI8_T - locale_BASE] = __cp_103_wctomb,
#endif
#ifdef __MB_EXTENDED_CHARSETS_JIS
    [locale_JIS - locale_BASE] = __jis_wctomb,
    [locale_EUCJP - locale_BASE] = __eucjp_wctomb,
    [locale_SJIS - locale_BASE] = __sjis_wctomb,
#endif
};

#endif /* __MB_CAPABLE */
