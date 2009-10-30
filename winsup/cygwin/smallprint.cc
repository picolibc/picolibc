/* smallprint.cc: small print routines for WIN32

   Copyright 1996, 1998, 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009
   Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "ntdll.h"
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>

#define LLMASK	(0xffffffffffffffffULL)
#define LMASK	(0xffffffff)

#define rnarg(dst, base, dosign, len, pad) __rn ((dst), (base), (dosign), va_arg (ap, long), len, pad, LMASK)
#define rnargLL(dst, base, dosign, len, pad) __rn ((dst), (base), (dosign), va_arg (ap, unsigned long long), len, pad, LLMASK)

static const char hex_str[] = "0123456789ABCDEF";

static char __fastcall *
__rn (char *dst, int base, int dosign, long long val, int len, int pad, unsigned long long mask)
{
  /* longest number is ULLONG_MAX, 18446744073709551615, 20 digits */
  unsigned long long uval = 0;
  char res[20];
  int l = 0;

  if (dosign && val < 0)
    {
      *dst++ = '-';
      uval = -val;
    }
  else if (dosign > 0 && val > 0)
    {
      *dst++ = '+';
      uval = val;
    }
  else
    uval = val;

  uval &= mask;

  do
    {
      res[l++] = hex_str[uval % base];
      uval /= base;
    }
  while (uval);

  while (len-- > l)
    *dst++ = pad;

  while (l > 0)
    *dst++ = res[--l];

  return dst;
}

int
__small_vsprintf (char *dst, const char *fmt, va_list ap)
{
  char tmp[NT_MAX_PATH];
  char *orig = dst;
  const char *s;
  PWCHAR w;
  UNICODE_STRING uw, *us;

  DWORD err = GetLastError ();

  while (*fmt)
    {
      int i, n = 0x7fff;
      bool l_opt = false;
      if (*fmt != '%')
	*dst++ = *fmt++;
      else
	{
	  int len = 0;
	  char pad = ' ';
	  int addsign = -1;

	  switch (*++fmt)
	  {
	    case '+':
	      addsign = 1;
	      fmt++;
	      break;
	    case '%':
	      *dst++ = *fmt++;
	      continue;
	  }

	  for (;;)
	    {
	      char c = *fmt++;
	      switch (c)
		{
		case '0':
		  if (len == 0)
		    {
		      pad = '0';
		      continue;
		    }
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		  len = len * 10 + (c - '0');
		  continue;
		case 'l':
		  l_opt = true;
		  continue;
		case 'c':
		  {
		    int c = va_arg (ap, int);
		    if (c > ' ' && c <= 127)
		      *dst++ = c;
		    else
		      {
			*dst++ = '0';
			*dst++ = 'x';
			dst = __rn (dst, 16, 0, c, len, pad, LMASK);
		      }
		  }
		  break;
		case 'C':
		  {
		    WCHAR wc = (WCHAR) va_arg (ap, int);
		    char buf[4], *c;
		    sys_wcstombs (buf, 4, &wc, 1);
		    for (c = buf; *c; ++c)
		      if (isprint (*c))
			*dst++ = *c;
		      else
			{
			  *dst++ = '0';
			  *dst++ = 'x';
			  dst = __rn (dst, 16, 0, *c, len, pad, LMASK);
			}
		  }
		case 'E':
		  strcpy (dst, "Win32 error ");
		  dst = __rn (dst + sizeof ("Win32 error"), 10, 0, err, len, pad, LMASK);
		  break;
		case 'd':
		  dst = rnarg (dst, 10, addsign, len, pad);
		  break;
		case 'D':
		  dst = rnargLL (dst, 10, addsign, len, pad);
		  break;
		case 'u':
		  dst = rnarg (dst, 10, 0, len, pad);
		  break;
		case 'U':
		  dst = rnargLL (dst, 10, 0, len, pad);
		  break;
		case 'o':
		  dst = rnarg (dst, 8, 0, len, pad);
		  break;
		case 'p':
		  *dst++ = '0';
		  *dst++ = 'x';
		  /* fall through */
		case 'x':
		  dst = rnarg (dst, 16, 0, len, pad);
		  break;
		case 'X':
		  dst = rnargLL (dst, 16, 0, len, pad);
		  break;
		case 'P':
		  if (!GetModuleFileName (NULL, tmp, NT_MAX_PATH))
		    s = "cygwin program";
		  else
		    s = tmp;
		  goto fillin;
		case '.':
		  n = strtol (fmt, (char **) &fmt, 10);
		  if (*fmt++ != 's')
		    goto endfor;
		case 's':
		  s = va_arg (ap, char *);
		  if (s == NULL)
		    s = "(null)";
		fillin:
		  for (i = 0; *s && i < n; i++)
		    if (l_opt && ((*(unsigned char *)s <= 0x1f && *s != '\n')
				  || *(unsigned char *)s >= 0x7f))
		      {
		      	*dst++ = '\\';
			*dst++ = 'x';
			*dst++ = hex_str[*(unsigned char *)s >> 4];
			*dst++ = hex_str[*(unsigned char *)s++ & 0xf];
		      }
		    else
		      *dst++ = *s++;
		  break;
		case 'W':
		  w = va_arg (ap, PWCHAR);
		  RtlInitUnicodeString (us = &uw, w ?: L"(null)");
		  goto wfillin;
		case 'S':
		  us = va_arg (ap, PUNICODE_STRING);
		  if (!us)
		    RtlInitUnicodeString (us = &uw, L"(null)");
		wfillin:
		  if (l_opt)
		    {
		      for (USHORT i = 0; i < us->Length / sizeof (WCHAR); ++i)
			{
			  WCHAR w = us->Buffer[i];
			  if ((w <= 0x1f && w != '\n') || w >= 0x7f)
			    {
			      *dst++ = '\\';
			      *dst++ = 'x';
			      *dst++ = hex_str[(w >> 12) & 0xf];
			      *dst++ = hex_str[(w >>  8) & 0xf];
			      *dst++ = hex_str[(w >>  4) & 0xf];
			      *dst++ = hex_str[w & 0xf];
			    }
			  else
			    *dst++ = w;
			}
		    }
		  else if (sys_wcstombs (tmp, NT_MAX_PATH, us->Buffer,
				    us->Length / sizeof (WCHAR)))
		    {
		      s = tmp;
		      goto fillin;
		    }
		  break;
		default:
		  *dst++ = '?';
		  *dst++ = fmt[-1];
		}
	    endfor:
	      break;
	    }
	}
    }
  *dst = 0;
  SetLastError (err);
  return dst - orig;
}

int
__small_sprintf (char *dst, const char *fmt, ...)
{
  int r;
  va_list ap;
  va_start (ap, fmt);
  r = __small_vsprintf (dst, fmt, ap);
  va_end (ap);
  return r;
}

void
small_printf (const char *fmt, ...)
{
  char buf[16384];
  va_list ap;
  DWORD done;
  int count;

#if 0	/* Turn on to force console errors */
  extern SECURITY_ATTRIBUTES sec_none;
  HANDLE h = CreateFileA ("CONOUT$", GENERIC_READ|GENERIC_WRITE,
		   FILE_SHARE_WRITE | FILE_SHARE_WRITE, &sec_none,
		   OPEN_EXISTING, 0, 0);
  if (h)
    SetStdHandle (STD_ERROR_HANDLE, h);
#endif

  va_start (ap, fmt);
  count = __small_vsprintf (buf, fmt, ap);
  va_end (ap);

  WriteFile (GetStdHandle (STD_ERROR_HANDLE), buf, count, &done, NULL);
  FlushFileBuffers (GetStdHandle (STD_ERROR_HANDLE));
}

#ifdef DEBUGGING
static HANDLE NO_COPY console_handle = NULL;
void
console_printf (const char *fmt, ...)
{
  char buf[16384];
  va_list ap;
  DWORD done;
  int count;

  if (!console_handle)
    console_handle = CreateFileA ("CON", GENERIC_WRITE,
				  FILE_SHARE_READ | FILE_SHARE_WRITE,
				  NULL, OPEN_EXISTING, 0, 0);

  if (console_handle == INVALID_HANDLE_VALUE)
    console_handle = GetStdHandle (STD_ERROR_HANDLE);

  va_start (ap, fmt);
  count = __small_vsprintf (buf, fmt, ap);
  va_end (ap);

  WriteFile (console_handle, buf, count, &done, NULL);
  FlushFileBuffers (console_handle);
}
#endif

#define wrnarg(dst, base, dosign, len, pad) __wrn ((dst), (base), (dosign), va_arg (ap, long), len, pad, LMASK)
#define wrnargLL(dst, base, dosign, len, pad) __wrn ((dst), (base), (dosign), va_arg (ap, unsigned long long), len, pad, LLMASK)

static PWCHAR __fastcall
__wrn (PWCHAR dst, int base, int dosign, long long val, int len, int pad, unsigned long long mask)
{
  /* longest number is ULLONG_MAX, 18446744073709551615, 20 digits */
  unsigned long long uval = 0;
  WCHAR res[20];
  int l = 0;

  if (dosign && val < 0)
    {
      *dst++ = L'-';
      uval = -val;
    }
  else if (dosign > 0 && val > 0)
    {
      *dst++ = L'+';
      uval = val;
    }
  else
    uval = val;

  uval &= mask;

  do
    {
      res[l++] = hex_str[uval % base];
      uval /= base;
    }
  while (uval);

  while (len-- > l)
    *dst++ = pad;

  while (l > 0)
    *dst++ = res[--l];

  return dst;
}

int
__small_vswprintf (PWCHAR dst, const WCHAR *fmt, va_list ap)
{
  WCHAR tmp[NT_MAX_PATH];
  PWCHAR orig = dst;
  const char *s;
  PWCHAR w;
  UNICODE_STRING uw, *us;

  DWORD err = GetLastError ();

  while (*fmt)
    {
      unsigned int n = 0x7fff;
      if (*fmt != L'%')
	*dst++ = *fmt++;
      else
	{
	  int len = 0;
	  WCHAR pad = L' ';
	  int addsign = -1;

	  switch (*++fmt)
	  {
	    case L'+':
	      addsign = 1;
	      fmt++;
	      break;
	    case L'%':
	      *dst++ = *fmt++;
	      continue;
	  }

	  for (;;)
	    {
	      char c = *fmt++;
	      switch (c)
		{
		case L'0':
		  if (len == 0)
		    {
		      pad = L'0';
		      continue;
		    }
		case L'1' ... L'9':
		  len = len * 10 + (c - L'0');
		  continue;
		case L'l':
		  continue;
		case L'c':
		case L'C':
		  {
		    unsigned int c = va_arg (ap, unsigned int);
		    if (c > L' ' && c <= 127)
		      *dst++ = c;
		    else
		      {
			*dst++ = L'0';
			*dst++ = L'x';
			dst = __wrn (dst, 16, 0, c, len, pad, LMASK);
		      }
		  }
		  break;
		case L'E':
		  wcscpy (dst, L"Win32 error ");
		  dst = __wrn (dst + sizeof ("Win32 error"), 10, 0, err, len, pad, LMASK);
		  break;
		case L'd':
		  dst = wrnarg (dst, 10, addsign, len, pad);
		  break;
		case L'D':
		  dst = wrnargLL (dst, 10, addsign, len, pad);
		  break;
		case L'u':
		  dst = wrnarg (dst, 10, 0, len, pad);
		  break;
		case L'U':
		  dst = wrnargLL (dst, 10, 0, len, pad);
		  break;
		case L'o':
		  dst = wrnarg (dst, 8, 0, len, pad);
		  break;
		case L'p':
		  *dst++ = L'0';
		  *dst++ = L'x';
		  /* fall through */
		case L'x':
		  dst = wrnarg (dst, 16, 0, len, pad);
		  break;
		case L'X':
		  dst = wrnargLL (dst, 16, 0, len, pad);
		  break;
		case L'P':
		  if (!GetModuleFileNameW (NULL, tmp, NT_MAX_PATH))
		    RtlInitUnicodeString (us = &uw, L"cygwin program");
		  else
		    RtlInitUnicodeString (us = &uw, tmp);
		  goto fillin;
		case L'.':
		  n = wcstoul (fmt, (wchar_t **) &fmt, 10);
		  if (*fmt++ != L's')
		    goto endfor;
		case L's':
		  s = va_arg (ap, char *);
		  if (s == NULL)
		    s = "(null)";
		  sys_mbstowcs (tmp, NT_MAX_PATH, s, n < 0x7fff ? (int) n : -1);
		  RtlInitUnicodeString (us = &uw, tmp);
		  goto fillin;
		  break;
		case L'W':
		  w = va_arg (ap, PWCHAR);
		  RtlInitUnicodeString (us = &uw, w ?: L"(null)");
		  goto fillin;
		case L'S':
		  us = va_arg (ap, PUNICODE_STRING);
		  if (!us)
		    RtlInitUnicodeString (us = &uw, L"(null)");
		fillin:
		  if (us->Length / sizeof (WCHAR) < n)
		    n = us->Length / sizeof (WCHAR);
		  w = us->Buffer;
		  for (unsigned int i = 0; i < n; i++)
		    *dst++ = *w++;
		  break;
		default:
		  *dst++ = L'?';
		  *dst++ = fmt[-1];
		}
	    endfor:
	      break;
	    }
	}
    }
  *dst = L'\0';
  SetLastError (err);
  return dst - orig;
}

int
__small_swprintf (PWCHAR dst, const WCHAR *fmt, ...)
{
  int r;
  va_list ap;
  va_start (ap, fmt);
  r = __small_vswprintf (dst, fmt, ap);
  va_end (ap);
  return r;
}
