/* smallprint.cc: small print routines for WIN32

   Copyright 1996, 1998, 2000, 2001, 2002, 2003, 2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "ntdll.h"
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#define LLMASK	(0xffffffffffffffffULL)
#define LMASK	(0xffffffff)

#define rnarg(dst, base, dosign, len, pad) __rn ((dst), (base), (dosign), va_arg (ap, long), len, pad, LMASK)
#define rnargLL(dst, base, dosign, len, pad) __rn ((dst), (base), (dosign), va_arg (ap, unsigned long long), len, pad, LLMASK)

static char __fastcall *
__rn (char *dst, int base, int dosign, long long val, int len, int pad, unsigned long long mask)
{
  /* longest number is ULLONG_MAX, 18446744073709551615, 20 digits */
  unsigned long long uval = 0;
  char res[20];
  static const char str[] = "0123456789ABCDEF";
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
      res[l++] = str[uval % base];
      uval /= base;
    }
  while (uval);

  while (len-- > l)
    *dst++ = pad;

  while (l > 0)
    *dst++ = res[--l];

  return dst;
}

extern "C" int
__small_vsprintf (char *dst, const char *fmt, va_list ap)
{
  char tmp[PATH_MAX];
  char *orig = dst;
  const char *s;
  PWCHAR w;
  UNICODE_STRING uw, *us;

  DWORD err = GetLastError ();

  while (*fmt)
    {
      int i, n = 0x7fff;
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
		  if (!GetModuleFileName (NULL, tmp, PATH_MAX))
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
		    *dst++ = *s++;
		  break;
		case 'W':
		  w = va_arg (ap, PWCHAR);
		  RtlInitUnicodeString (&uw, w);
		  us = &uw;
		  goto wfillin;
		case 'S':
		  us = va_arg (ap, PUNICODE_STRING);
		wfillin:
		  {
		    char *tmp;

		    if (!sys_wcstombs_alloc (&tmp, PATH_MAX, us->Buffer,
					     us->Length / sizeof (WCHAR)))
		      {
			s = "invalid UNICODE_STRING";
			goto fillin;
		      }
		    for (i = 0; *tmp && i < n; i++)
		      *dst++ = *tmp++;
		    free (tmp);
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

extern "C" int
__small_sprintf (char *dst, const char *fmt, ...)
{
  int r;
  va_list ap;
  va_start (ap, fmt);
  r = __small_vsprintf (dst, fmt, ap);
  va_end (ap);
  return r;
}

extern "C" void
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
extern "C" void
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
