/* Copyright (c) 2009 Corinna Vinschen <corinna@vinschen.de> */
#include <wchar.h>
#include "local.h"

#ifdef _MB_CAPABLE
#include "sb_charsets.h"

#if defined(_MB_EXTENDED_CHARSETS_ISO) || defined(_MB_EXTENDED_CHARSETS_WINDOWS)
/* Handle one to four decimal digits.  Return -1 in any other case. */
static int
__micro_atoi (const char *s)
{
  int ret = 0;

  if (!*s)
    return -1;
  while (*s)
    {
      if (*s < '0' || *s > '9' || ret >= 10000)
	return -1;
      ret = 10 * ret + (*s++ - '0');
    }
  return ret;
}
#endif

#ifdef _MB_EXTENDED_CHARSETS_ISO
int
__iso_8859_val_index (int val)
{
  if (val >= 2 && val <= 16)
    {
      val -= 2;
      if (val > 10)
	--val;
      return (int) val;
    }
  return -1;
}

int
__iso_8859_index (const char *charset_ext)
{
  return __iso_8859_val_index (__micro_atoi (charset_ext));
}
#endif /* _MB_EXTENDED_CHARSETS_ISO */

#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
int
__cp_val_index (int val)
{
  int cp_idx;
  switch (val)
    {
    case 437:
      cp_idx = 0;
      break;
    case 720:
      cp_idx = 1;
      break;
    case 737:
      cp_idx = 2;
      break;
    case 775:
      cp_idx = 3;
      break;
    case 850:
      cp_idx = 4;
      break;
    case 852:
      cp_idx = 5;
      break;
    case 855:
      cp_idx = 6;
      break;
    case 857:
      cp_idx = 7;
      break;
    case 858:
      cp_idx = 8;
      break;
    case 862:
      cp_idx = 9;
      break;
    case 866:
      cp_idx = 10;
      break;
    case 874:
      cp_idx = 11;
      break;
    case 1125:
      cp_idx = 12;
      break;
    case 1250:
      cp_idx = 13;
      break;
    case 1251:
      cp_idx = 14;
      break;
    case 1252:
      cp_idx = 15;
      break;
    case 1253:
      cp_idx = 16;
      break;
    case 1254:
      cp_idx = 17;
      break;
    case 1255:
      cp_idx = 18;
      break;
    case 1256:
      cp_idx = 19;
      break;
    case 1257:
      cp_idx = 20;
      break;
    case 1258:
      cp_idx = 21;
      break;
    case 20866:
      cp_idx = 22;
      break;
    case 21866:
      cp_idx = 23;
      break;
    case 101:
      cp_idx = 24;
      break;
    case 102:
      cp_idx = 25;
      break;
    case 103:
      cp_idx = 26;
      break;
    default:
      cp_idx = -1;
      break;
    }
  return cp_idx;
}

int
__cp_index (const char *charset_ext)
{
  return __cp_val_index (__micro_atoi (charset_ext));
}

#endif /* _MB_EXTENDED_CHARSETS_WINDOWS */
#endif /* _MB_CAPABLE */
