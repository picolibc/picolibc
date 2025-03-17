/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
#include <stdlib.h>
#include <locale.h>
#include "mbctype.h"
#include <wchar.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include "local.h"
#include "../ctype/local.h"

int
__ascii_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
  wchar_t dummy;
  unsigned char *t = (unsigned char *)s;
  unsigned char c;

  (void) state;
  if (pwc == NULL)
    pwc = &dummy;

  if (s == NULL)
    return 0;

  if (n == 0)
    return -2;

  c = *t;

  if (c >= 0x80)
    return -1;

  *pwc = (wchar_t)c;

  if (*t == '\0')
    return 0;

  return 1;
}

#ifdef _MB_CAPABLE

/* we override the mbstate_t __count field for more complex encodings and use it store a state value */
#define __state __count

int
__utf8_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
  wchar_t dummy;
  unsigned char *t = (unsigned char *)s;
  int ch;
  int i = 0;

  if (pwc == NULL)
    pwc = &dummy;

  if (s == NULL)
    return 0;

  if (n == 0)
    return -2;

  if (state->__count == 0)
    ch = t[i++];
  else
    ch = state->__value.__wchb[0];

  if (ch == '\0')
    {
      *pwc = 0;
      state->__count = 0;
      return 0; /* s points to the null character */
    }

  if (ch <= 0x7f)
    {
      /* single-byte sequence */
      state->__count = 0;
      *pwc = ch;
      return 1;
    }
  if (ch >= 0xc0 && ch <= 0xdf)
    {
      if (ch == 0xc0)
        return -1;
      /* two-byte sequence */
      state->__value.__wchb[0] = ch;
      if (state->__count == 0)
	state->__count = 1;
      else if (n < (size_t)-1)
	++n;
      if (n < 2)
	return -2;
      ch = t[i++];
      if (ch < 0x80 || ch > 0xbf)
        return -1;
      if (state->__value.__wchb[0] < 0xc2)
	{
	  /* overlong UTF-8 sequence */
	  return -1;
	}
      state->__count = 0;
      *pwc = ((wchar_t)(state->__value.__wchb[0] & 0x1f) << 6)
          |    (wchar_t)(ch & 0x3f);
      return i;
    }
  if (ch >= 0xe0 && ch <= 0xef)
    {
      /* three-byte sequence */
      wchar_t tmp;
      state->__value.__wchb[0] = ch;
      if (state->__count == 0)
	state->__count = 1;
      else if (n < (size_t)-1)
	++n;
      if (n < 2)
	return -2;
      ch = (state->__count == 1) ? t[i++] : state->__value.__wchb[1];
      if (state->__value.__wchb[0] == 0xe0 && ch < 0xa0)
	{
	  /* overlong UTF-8 sequence */
	  return -1;
	}
      if (ch < 0x80 || ch > 0xbf)
	{
	  return -1;
	}
      state->__value.__wchb[1] = ch;
      if (state->__count == 1)
	state->__count = 2;
      else if (n < (size_t)-1)
	++n;
      if (n < 3)
	return -2;
      ch = t[i++];
      if (ch < 0x80 || ch > 0xbf)
	{
	  return -1;
	}
      state->__count = 0;
      tmp = ((wchar_t)(state->__value.__wchb[0] & 0x0f) << 12)
          |    ((wchar_t)(state->__value.__wchb[1] & 0x3f) << 6)
	|     (wchar_t)(ch & 0x3f);
      /* Check for surrogates */
      if (0xd800 <= tmp && tmp <= 0xdfff)
        {
          return -1;
        }
      *pwc = tmp;
      return i;
    }
  if (ch >= 0xf0 && ch <= 0xf4)
    {
      /* four-byte sequence */
      uint32_t tmp;
      state->__value.__wchb[0] = ch;
      if (state->__count == 0)
	state->__count = 1;
      else if (n < (size_t)-1)
	++n;
      if (n < 2)
	return -2;
      ch = (state->__count == 1) ? t[i++] : state->__value.__wchb[1];
      if ((state->__value.__wchb[0] == 0xf0 && ch < 0x90)
	  || (state->__value.__wchb[0] == 0xf4 && ch >= 0x90))
	{
	  /* overlong UTF-8 sequence or result is > 0x10ffff */
	  return -1;
	}
      if (ch < 0x80 || ch > 0xbf)
	{
	  return -1;
	}
      state->__value.__wchb[1] = ch;
      if (state->__count == 1)
	state->__count = 2;
      else if (n < (size_t)-1)
	++n;
      if (n < 3)
	return -2;
      ch = (state->__count == 2) ? t[i++] : state->__value.__wchb[2];
      if (ch < 0x80 || ch > 0xbf)
	{
	  return -1;
	}
      state->__value.__wchb[2] = ch;
      if (state->__count == 2)
	state->__count = 3;
      else if (n < (size_t)-1)
	++n;
#if __SIZEOF_WCHAR_T__ == 2
      if (state->__count == 3)
	{
	  /* On systems which have wchar_t being UTF-16 values, the value
	     doesn't fit into a single wchar_t in this case.  So what we
	     do here is to store the state with a special value of __count
	     and return the first half of a surrogate pair.  The first
	     three bytes of a UTF-8 sequence are enough to generate the
	     first half of a UTF-16 surrogate pair.  As return value we
	     choose to return the number of bytes actually read up to
	     here.
	     The second half of the surrogate pair is returned in case we
	     recognize the special __count value of four, and the next
	     byte is actually a valid value.  See below. */
            tmp = (uint32_t)((state->__value.__wchb[0] & (uint32_t) 0x07) << 18)
                |   (uint32_t)((state->__value.__wchb[1] & (uint32_t) 0x3f) << 12)
                |   (uint32_t)((state->__value.__wchb[2] & (uint32_t) 0x3f) << 6);
	  state->__count = 4;
	  *pwc = 0xd800 | ((tmp - 0x10000) >> 10);
	  return i;
	}
#endif
      if (n < 4)
	return -2;
      ch = t[i++];
      if (ch < 0x80 || ch > 0xbf)
	{
	  return -1;
	}
      tmp = (((uint32_t)state->__value.__wchb[0] & 0x07) << 18)
        |   (((uint32_t)state->__value.__wchb[1] & 0x3f) << 12)
        |   (((uint32_t)state->__value.__wchb[2] & 0x3f) << 6)
        |   ((uint32_t)ch & 0x3f);
#if __SIZEOF_WCHAR_T__ == 2
      if (state->__count == 4)
	/* Create the second half of the surrogate pair for systems with
	   wchar_t == UTF-16 . */
	*pwc = 0xdc00 | (tmp & 0x3ff);
      else
#endif
	*pwc = tmp;
      state->__count = 0;
      return i;
    }

  return -1;
}

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define __ucs2le_mbtowc __ucs2_mbtowc
#define __ucs2be_mbtowc __ucs2swap_mbtowc
#define __ucs4le_mbtowc __ucs4_mbtowc
#define __ucs4be_mbtowc __ucs4swap_mbtowc
#else
#define __ucs2le_mbtowc __ucs2swap_mbtowc
#define __ucs2be_mbtowc __ucs2_mbtowc
#define __ucs4le_mbtowc __ucs4swap_mbtowc
#define __ucs4be_mbtowc __ucs4_mbtowc
#endif

static int
__ucs2_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
    uint16_t    uchar;

    (void) state;
    if (n < 2)
        return -1;
    memcpy(&uchar, s, 2);
    *pwc = uchar;
    return 2;
}

static int
__ucs2swap_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
    uint16_t    uchar;

    (void) state;
    if (n < 2)
        return -1;
    memcpy(&uchar, s, 2);
    *pwc = __bswap16(uchar);
    return 2;
}

static int
__ucs4_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
    uint32_t    uchar;

    (void) state;
    if (n < 4)
        return -1;
    memcpy(&uchar, s, 4);
    *pwc = uchar;
    return 4;
}

static int
__ucs4swap_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
    uint32_t    uchar;

    (void) state;
    if (n < 4)
        return -1;
    memcpy(&uchar, s, 4);
    *pwc = __bswap32(uchar);
    return 4;
}

#ifdef _MB_EXTENDED_CHARSETS_ISO
static int
___iso_mbtowc (wchar_t *pwc, const char *s, size_t n,
	       enum locale_id id, mbstate_t *state)
{
  wchar_t dummy;
  unsigned char *t = (unsigned char *)s;

  (void) state;
  if (pwc == NULL)
    pwc = &dummy;

  if (s == NULL)
    return 0;

  if (n == 0)
    return -2;

  if (*t >= 0xa0)
    {
      if (id > locale_ISO_8859_1)
	{
	  *pwc = __iso_8859_conv[id - locale_ISO_8859_2][*t - 0xa0];
	  if (*pwc == 0) /* Invalid character */
	    {
	      return -1;
	    }
	  return 1;
	}
    }

  *pwc = (wchar_t) *t;

  if (*t == '\0')
    return 0;

  return 1;
}

static int
__iso_8859_1_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_1, state);
}

static int
__iso_8859_2_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_2, state);
}

static int
__iso_8859_3_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_3, state);
}

static int
__iso_8859_4_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_4, state);
}

static int
__iso_8859_5_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_5, state);
}

static int
__iso_8859_6_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_6, state);
}

static int
__iso_8859_7_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_7, state);
}

static int
__iso_8859_8_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_8, state);
}

static int
__iso_8859_9_mbtowc (wchar_t *pwc, const char *s, size_t n,
		     mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_9, state);
}

static int
__iso_8859_10_mbtowc (wchar_t *pwc, const char *s, size_t n,
		      mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_10, state);
}

static int
__iso_8859_11_mbtowc (wchar_t *pwc, const char *s, size_t n,
		      mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_11, state);
}

static int
__iso_8859_13_mbtowc (wchar_t *pwc, const char *s, size_t n,
		      mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_13, state);
}

static int
__iso_8859_14_mbtowc (wchar_t *pwc, const char *s, size_t n,
		      mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_14, state);
}

static int
__iso_8859_15_mbtowc (wchar_t *pwc, const char *s, size_t n,
		      mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_15, state);
}

static int
__iso_8859_16_mbtowc (wchar_t *pwc, const char *s, size_t n,
		      mbstate_t *state)
{
  return ___iso_mbtowc (pwc, s, n, locale_ISO_8859_16, state);
}

#endif /* _MB_EXTENDED_CHARSETS_ISO */

#ifdef _MB_EXTENDED_CHARSETS_WINDOWS

static int
___cp_mbtowc (wchar_t *pwc, const char *s, size_t n,
	      enum locale_id id, mbstate_t *state)
{
  wchar_t dummy;
  unsigned char *t = (unsigned char *)s;

  (void) state;
  if (pwc == NULL)
    pwc = &dummy;

  if (s == NULL)
    return 0;

  if (n == 0)
    return -2;

  if (*t >= 0x80)
    {
        *pwc = __cp_conv[id - locale_WINDOWS_BASE][*t - 0x80];
        if (*pwc == 0) /* Invalid character */
            return -1;
        return 1;
    }

  *pwc = (wchar_t)*t;

  if (*t == '\0')
    return 0;

  return 1;
}

static int
__cp_437_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP437, state);
}

static int
__cp_720_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP720, state);
}

static int
__cp_737_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP737, state);
}

static int
__cp_775_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP775, state);
}

static int
__cp_850_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP850, state);
}

static int
__cp_852_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP852, state);
}

static int
__cp_855_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP855, state);
}

static int
__cp_857_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP857, state);
}

static int
__cp_858_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP858, state);
}

static int
__cp_862_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP862, state);
}

static int
__cp_866_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP866, state);
}

static int
__cp_874_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP874, state);
}

static int
__cp_1125_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1125, state);
}

static int
__cp_1250_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1250, state);
}

static int
__cp_1251_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1251, state);
}

static int
__cp_1252_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1252, state);
}

static int
__cp_1253_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1253, state);
}

static int
__cp_1254_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1254, state);
}

static int
__cp_1255_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1255, state);
}

static int
__cp_1256_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1256, state);
}

static int
__cp_1257_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1257, state);
}

static int
__cp_1258_mbtowc (wchar_t *pwc, const char *s, size_t n,
		  mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_CP1258, state);
}

static int
__cp_20866_mbtowc (wchar_t *pwc, const char *s, size_t n,
		   mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_KOI8_R, state);
}

static int
__cp_21866_mbtowc (wchar_t *pwc, const char *s, size_t n,
		   mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_KOI8_U, state);
}

static int
__cp_101_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_GEORGIAN_PS, state);
}

static int
__cp_102_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_PT154, state);
}

static int
__cp_103_mbtowc (wchar_t *pwc, const char *s, size_t n,
		 mbstate_t *state)
{
  return ___cp_mbtowc (pwc, s, n, locale_KOI8_T, state);
}

#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */

#ifdef _MB_EXTENDED_CHARSETS_JIS

typedef enum __packed { ESCAPE, DOLLAR, BRACKET, AT, B, J,
               NUL, JIS_CHAR, OTHER, JIS_C_NUM } JIS_CHAR_TYPE;
typedef enum __packed { ASCII, JIS, A_ESC, A_ESC_DL, JIS_1, J_ESC, J_ESC_BR,
               INV, JIS_S_NUM } JIS_STATE;
typedef enum __packed { COPY_A, COPY_J1, COPY_J2, MAKE_A, NOOP, EMPTY, ERROR } JIS_ACTION;

/**************************************************************************************
 * state/action tables for processing JIS encoding
 * Where possible, switches to JIS are grouped with proceding JIS characters and switches
 * to ASCII are grouped with preceding JIS characters.  Thus, maximum returned length
 * is 2 (switch to JIS) + 2 (JIS characters) + 2 (switch back to ASCII) = 6.
 *************************************************************************************/

static JIS_STATE JIS_state_table[JIS_S_NUM][JIS_C_NUM] = {
/*              ESCAPE   DOLLAR    BRACKET   AT       B       J        NUL      JIS_CHAR  OTHER */
/* ASCII */   { A_ESC,   ASCII,    ASCII,    ASCII,   ASCII,  ASCII,   ASCII,   ASCII,    ASCII },
/* JIS */     { J_ESC,   JIS_1,    JIS_1,    JIS_1,   JIS_1,  JIS_1,   INV,     JIS_1,    INV },
/* A_ESC */   { ASCII,   A_ESC_DL, ASCII,    ASCII,   ASCII,  ASCII,   ASCII,   ASCII,    ASCII },
/* A_ESC_DL */{ ASCII,   ASCII,    ASCII,    JIS,     JIS,    ASCII,   ASCII,   ASCII,    ASCII },
/* JIS_1 */   { INV,     JIS,      JIS,      JIS,     JIS,    JIS,     INV,     JIS,      INV },
/* J_ESC */   { INV,     INV,      J_ESC_BR, INV,     INV,    INV,     INV,     INV,      INV },
/* J_ESC_BR */{ INV,     INV,      INV,      INV,     ASCII,  ASCII,   INV,     INV,      INV },
};

static JIS_ACTION JIS_action_table[JIS_S_NUM][JIS_C_NUM] = {
/*              ESCAPE   DOLLAR    BRACKET   AT       B        J        NUL      JIS_CHAR  OTHER */
/* ASCII */   { NOOP,    COPY_A,   COPY_A,   COPY_A,  COPY_A,  COPY_A,  EMPTY,   COPY_A,  COPY_A},
/* JIS */     { NOOP,    COPY_J1,  COPY_J1,  COPY_J1, COPY_J1, COPY_J1, ERROR,   COPY_J1, ERROR },
/* A_ESC */   { COPY_A,  NOOP,     COPY_A,   COPY_A,  COPY_A,  COPY_A,  COPY_A,  COPY_A,  COPY_A},
/* A_ESC_DL */{ COPY_A,  COPY_A,   COPY_A,   NOOP,    NOOP,    COPY_A,  COPY_A,  COPY_A,  COPY_A},
/* JIS_1 */   { ERROR,   COPY_J2,  COPY_J2,  COPY_J2, COPY_J2, COPY_J2, ERROR,   COPY_J2, ERROR },
/* J_ESC */   { ERROR,   ERROR,    NOOP,     ERROR,   ERROR,   ERROR,   ERROR,   ERROR,   ERROR },
/* J_ESC_BR */{ ERROR,   ERROR,    ERROR,    ERROR,   MAKE_A,  MAKE_A,  ERROR,   ERROR,   ERROR },
};

static int
__sjis_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
  wchar_t dummy;
  wint_t jischar, uchar;
  unsigned char *t = (unsigned char *)s;
  int ch;
  int i = 0;

  if (pwc == NULL)
    pwc = &dummy;

  if (s == NULL)
    return 0;  /* not state-dependent */

  if (n == 0)
    return -2;

  ch = t[i++];
  if (state->__count == 0)
    {
      if (_issjis1 (ch))
	{
	  state->__value.__wchb[0] = ch;
	  state->__count = 1;
	  if (n <= 1)
	    return -2;
	  ch = t[i++];
	}
      else if (!_issjis1b(ch))
        {
          return -1;
        }
    }
  if (state->__count == 1)
    {
      if (_issjis2 (ch))
	{
          jischar = (((wchar_t)state->__value.__wchb[0]) << 8) + (wchar_t)ch;
          uchar = __jp2uc(jischar, JP_SJIS);
	  state->__count = 0;
          if (uchar == WEOF)
            {
              return -1;
            }
          *pwc = (wchar_t) uchar;
	  return i;
	}
      else
	{
	  return -1;
	}
    }

  uchar = __jp2uc((wint_t) *t, JP_SJIS);
  if (uchar == WEOF)
    {
      return -1;
    }

  *pwc = uchar;
  if (*t == '\0')
    return 0;

  return 1;
}

static int
__eucjp_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
  wchar_t dummy;
  wint_t jischar, uchar;
  unsigned char *t = (unsigned char *)s;
  int ch;
  int i = 0;

  if (pwc == NULL)
    pwc = &dummy;

  if (s == NULL)
    return 0;

  if (n == 0)
    return -2;

  ch = t[i++];
  if (state->__count == 0)
    {
      if (_iseucjp1 (ch))
	{
	  state->__value.__wchb[0] = ch;
	  state->__count = 1;
	  if (n <= 1)
	    return -2;
	  ch = t[i++];
	}
    }
  if (state->__count == 1)
    {
      if (_iseucjp2 (ch))
	{
	  if (state->__value.__wchb[0] == 0x8f)
	    {
	      state->__value.__wchb[1] = ch;
	      state->__count = 2;
	      if (n <= (size_t) i)
		return -2;
	      ch = t[i++];
	    }
	  else
	    {
	      jischar = (((wchar_t)state->__value.__wchb[0]) << 8) + (wchar_t)ch;
              uchar = __jp2uc(jischar, JP_EUCJP);
	      state->__count = 0;
              if (uchar == WEOF)
                {
                  return -1;
                }
              *pwc = (wchar_t) uchar;
	      return i;
	    }
	}
      else
	{
	  return -1;
	}
    }
  if (state->__count == 2)
    {
      if (_iseucjp2 (ch))
	{
	  jischar = (((wchar_t)state->__value.__wchb[1]) << 8)
            + (wchar_t)(ch & 0x7f);
          uchar = __jp2uc(jischar, JP_EUCJP);
	  state->__count = 0;
          if (uchar == WEOF)
            {
              return -1;
            }
          *pwc = (wchar_t) uchar;
	  return i;
	}
      else
	{
	  return -1;
	}
    }

  uchar = __jp2uc((wint_t)(wchar_t) *t, JP_EUCJP);

  if (uchar == WEOF)
    {
      return -1;
    }
  *pwc = (wchar_t) uchar;

  if (*t == '\0')
    return 0;

  return 1;
}

static int
__jis_mbtowc (
        wchar_t       *pwc,
        const char    *s,
        size_t         n,
        mbstate_t      *state)
{
  wchar_t dummy;
  wint_t jischar, uchar;
  unsigned char *t = (unsigned char *)s;
  JIS_STATE curr_state;
  JIS_ACTION action;
  JIS_CHAR_TYPE ch;
  unsigned char *ptr;
  unsigned int i;
  int curr_ch;

  if (pwc == NULL)
    pwc = &dummy;

  if (s == NULL)
    {
      state->__state = ASCII;
      return 1;  /* state-dependent */
    }

  if (n == 0)
    return -2;

  curr_state = state->__state;
  ptr = t;

  for (i = 0; i < n; ++i)
    {
      curr_ch = t[i];
      switch (curr_ch)
	{
	case ESC_CHAR:
	  ch = ESCAPE;
	  break;
	case '$':
	  ch = DOLLAR;
	  break;
	case '@':
	  ch = AT;
	  break;
	case '(':
	  ch = BRACKET;
	  break;
	case 'B':
	  ch = B;
	  break;
	case 'J':
	  ch = J;
	  break;
	case '\0':
	  ch = NUL;
	  break;
	default:
	  if (_isjis (curr_ch))
	    ch = JIS_CHAR;
	  else
	    ch = OTHER;
	}

      action = JIS_action_table[curr_state][ch];
      curr_state = JIS_state_table[curr_state][ch];

      switch (action)
	{
	case NOOP:
	  break;
	case EMPTY:
	  state->__state = ASCII;
	  *pwc = (wchar_t)0;
	  return 0;
	case COPY_A:
	  state->__state = ASCII;
	  *pwc = (wchar_t)*ptr;
	  return (i + 1);
	case COPY_J1:
	  state->__value.__wchb[0] = t[i];
	  break;
	case COPY_J2:
	  state->__state = JIS;
	  jischar = (((wchar_t)state->__value.__wchb[0]) << 8) + (wchar_t)(t[i]);
          uchar = __jp2uc(jischar, JP_JIS);
          if (uchar == WEOF)
            {
              return -1;
            }
          *pwc = uchar;
	  return (i + 1);
	case MAKE_A:
	  ptr = (unsigned char *)(t + i + 1);
	  break;
	case ERROR:
	default:
	  return -1;
	}

    }

  state->__state = curr_state;
  return -2;  /* n < bytes needed */
}
#endif /* _MB_EXTENDED_CHARSETS_JIS */

const mbtowc_p __mbtowc[locale_END - locale_BASE] = {
    [locale_C - locale_BASE] = __ascii_mbtowc,
    [locale_UTF_8 - locale_BASE] = __utf8_mbtowc,
    [locale_UCS_2 - locale_BASE] = __ucs2_mbtowc,
    [locale_UCS_2LE - locale_BASE] = __ucs2le_mbtowc,
    [locale_UCS_2BE - locale_BASE] = __ucs2be_mbtowc,
    [locale_UCS_4 - locale_BASE] = __ucs4_mbtowc,
    [locale_UCS_4LE - locale_BASE] = __ucs4le_mbtowc,
    [locale_UCS_4BE - locale_BASE] = __ucs4be_mbtowc,
#ifdef _MB_EXTENDED_CHARSETS_ISO
    [locale_ISO_8859_1 - locale_BASE] = __iso_8859_1_mbtowc,
    [locale_ISO_8859_2 - locale_BASE] = __iso_8859_2_mbtowc,
    [locale_ISO_8859_3 - locale_BASE] = __iso_8859_3_mbtowc,
    [locale_ISO_8859_4 - locale_BASE] = __iso_8859_4_mbtowc,
    [locale_ISO_8859_5 - locale_BASE] = __iso_8859_5_mbtowc,
    [locale_ISO_8859_6 - locale_BASE] = __iso_8859_6_mbtowc,
    [locale_ISO_8859_7 - locale_BASE] = __iso_8859_7_mbtowc,
    [locale_ISO_8859_8 - locale_BASE] = __iso_8859_8_mbtowc,
    [locale_ISO_8859_9 - locale_BASE] = __iso_8859_9_mbtowc,
    [locale_ISO_8859_10 - locale_BASE] = __iso_8859_10_mbtowc,
    [locale_ISO_8859_11 - locale_BASE] = __iso_8859_11_mbtowc,
    [locale_ISO_8859_13 - locale_BASE] = __iso_8859_13_mbtowc,
    [locale_ISO_8859_14 - locale_BASE] = __iso_8859_14_mbtowc,
    [locale_ISO_8859_15 - locale_BASE] = __iso_8859_15_mbtowc,
    [locale_ISO_8859_16 - locale_BASE] = __iso_8859_16_mbtowc,
#endif
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
    [locale_CP437 - locale_BASE] = __cp_437_mbtowc,
    [locale_CP720 - locale_BASE] = __cp_720_mbtowc,
    [locale_CP737 - locale_BASE] = __cp_737_mbtowc,
    [locale_CP775 - locale_BASE] = __cp_775_mbtowc,
    [locale_CP850 - locale_BASE] = __cp_850_mbtowc,
    [locale_CP852 - locale_BASE] = __cp_852_mbtowc,
    [locale_CP855 - locale_BASE] = __cp_855_mbtowc,
    [locale_CP857 - locale_BASE] = __cp_857_mbtowc,
    [locale_CP858 - locale_BASE] = __cp_858_mbtowc,
    [locale_CP862 - locale_BASE] = __cp_862_mbtowc,
    [locale_CP866 - locale_BASE] = __cp_866_mbtowc,
    [locale_CP874 - locale_BASE] = __cp_874_mbtowc,
    [locale_CP1125 - locale_BASE] = __cp_1125_mbtowc,
    [locale_CP1250 - locale_BASE] = __cp_1250_mbtowc,
    [locale_CP1251 - locale_BASE] = __cp_1251_mbtowc,
    [locale_CP1252 - locale_BASE] = __cp_1252_mbtowc,
    [locale_CP1253 - locale_BASE] = __cp_1253_mbtowc,
    [locale_CP1254 - locale_BASE] = __cp_1254_mbtowc,
    [locale_CP1255 - locale_BASE] = __cp_1255_mbtowc,
    [locale_CP1256 - locale_BASE] = __cp_1256_mbtowc,
    [locale_CP1257 - locale_BASE] = __cp_1257_mbtowc,
    [locale_CP1258 - locale_BASE] = __cp_1258_mbtowc,
    [locale_KOI8_R - locale_BASE] = __cp_20866_mbtowc,
    [locale_KOI8_U - locale_BASE] = __cp_21866_mbtowc,
    [locale_GEORGIAN_PS - locale_BASE] = __cp_101_mbtowc,
    [locale_PT154 - locale_BASE] = __cp_102_mbtowc,
    [locale_KOI8_T - locale_BASE] = __cp_103_mbtowc,
#endif
#ifdef _MB_EXTENDED_CHARSETS_JIS
    [locale_JIS - locale_BASE] = __jis_mbtowc,
    [locale_EUCJP - locale_BASE] = __eucjp_mbtowc,
    [locale_SJIS - locale_BASE] = __sjis_mbtowc,
#endif
};

#endif /* _MB_CAPABLE */
