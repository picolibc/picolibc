/* smallprint.c: small print routines for WIN32

   Copyright 1996, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>

int __small_sprintf (char *dst, const char *fmt,...);
int __small_vsprintf (char *dst, const char *fmt, va_list ap);

static char *
rn (char *dst, int base, int dosign, int val, int len, int pad)
{
  /* longest number is 4294967295, 10 digits */
  unsigned uval;
  char res[10];
  static const char str[16] = "0123456789ABCDEF";
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
    {
      uval = val;
    }

  do
    {
      res[l++] = str[uval % base];
      uval /= base;
    }
  while (uval);

  while (len -- > l)
    *dst++ = pad;

  while (l > 0)
    {
      *dst++ = res[--l];
    }

  return dst;
}

int
__small_vsprintf (char *dst, const char *fmt, va_list ap)
{
  char tmp[MAX_PATH + 1];
  char *orig = dst;
  const char *s;

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
		    int c = va_arg (ap,int);
		    if (c > ' ' && c <= 127)
		      *dst++ = c;
		    else
		      {
			*dst++ = '0';
			*dst++ = 'x';
			dst = rn (dst, 16, 0, c, len, pad);
		      }
		  }
		  break;
		case 'E':
		  strcpy (dst, "Win32 error ");
		  dst = rn (dst + sizeof ("Win32 error"), 10, 0, GetLastError (), len, pad);
		  break;
		case 'd':
		  dst = rn (dst, 10, addsign, va_arg (ap, int), len, pad);
		  break;
		case 'u':
		  dst = rn (dst, 10, 0, va_arg (ap, int), len, pad);
		  break;
		case 'p':
		  *dst++ = '0';
		  *dst++ = 'x';
		  /* fall through */
		case 'x':
		  dst = rn (dst, 16, 0, va_arg (ap, int), len, pad);
		  break;
		case 'P':
		  if (!GetModuleFileName (NULL, tmp, MAX_PATH))
		    s = "cygwin program";
		  else
		    s = tmp;
		  goto fillin;
		case '.':
		  n = strtol (fmt, (char **)&fmt, 10);
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
  return dst - orig;
}

int
__small_sprintf (char *dst, const char *fmt,...)
{
  int r;
  va_list ap;
  va_start (ap, fmt);
  r = __small_vsprintf (dst, fmt, ap);
  va_end (ap);
  return r;
}

void
small_printf (const char *fmt,...)
{
  char buf[2000];
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

  WriteFile (GetStdHandle (STD_ERROR_HANDLE), buf, count, &done, 0);
  FlushFileBuffers (GetStdHandle (STD_ERROR_HANDLE));
}
