/* smallprint.cc: small print routines for WIN32

   Copyright 1996, 1998, 2000, 2001, 2002, 2003, 2005, 2006,
	     2007, 2008, 2009, 2012, 2013, 2014
   Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "ntdll.h"
#include "sync.h"
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>

#define LLMASK	(0xffffffffffffffffULL)
#define LMASK	(0xffffffff)

#define rnarg(dst, base, dosign, len, pad) __rn ((dst), (base), (dosign), va_arg (ap, int32_t), len, pad, LMASK)
#define rnargLL(dst, base, dosign, len, pad) __rn ((dst), (base), (dosign), va_arg (ap, uint64_t), len, pad, LLMASK)

static const char hex_str[] = "0123456789ABCDEF";

class tmpbuf
{
  static WCHAR buf[NT_MAX_PATH];
  static muto lock;
  bool locked;
public:
  operator WCHAR * ()
  {
    if (!locked)
      {
	lock.init ("smallprint_buf")->acquire ();
	locked = true;
      }
    return buf;
  }
  operator char * () {return (char *) ((WCHAR *) *this);}

  tmpbuf (): locked (false) {};
  ~tmpbuf ()
  {
    if (locked)
      lock.release ();
  }
};

WCHAR tmpbuf::buf[NT_MAX_PATH];
muto tmpbuf::lock;

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

/*
  Meaning of format conversion specifiers.  If 'l' isn't explicitely mentioned,
  it's ignored!

   c       char
   C       WCHAR/wchar_t
   d       signed int, 4 byte
  ld       signed long, 4 byte on 32 bit, 8 byte on 64 bit
   D       signed long long, 8 byte
   E       GetLastError
   o       octal unsigned int, 4 byte
  lo       octal unsigned long, 4 byte on 32 bit, 8 byte on 64 bit
   O       octal unsigned long long, 8 byte
   p       address
   P       process name
   R       return value, 4 byte.
  lR       return value, 4 byte on 32 bit, 8 byte on 64 bit.
   s       char *
  ls       char * w/ non-ASCII tweaking
   S       PUNICODE_STRING
  lS       PUNICODE_STRING w/ non-ASCII tweaking
   u       unsigned int, 4 byte
  lu       unsigned long, 4 byte on 32 bit, 8 byte on 64 bit
   U       unsigned long long, 8 byte
   W       PWCHAR/wchar_t *
  lW       PWCHAR/wchar_t * w/ non-ASCII tweaking
   x       hex unsigned int, 4 byte
  lx       hex unsigned long, 4 byte on 32 bit, 8 byte on 64 bit
   X       hex unsigned long long, 8 byte
   y       0x hex unsigned int, 4 byte
  ly       0x hex unsigned long, 4 byte on 32 bit, 8 byte on 64 bit
   Y       0x hex unsigned long long, 8 byte
*/

int
__small_vsprintf (char *dst, const char *fmt, va_list ap)
{
  tmpbuf tmp;
  char *orig = dst;
  const char *s;
  PWCHAR w;
  UNICODE_STRING uw, *us;
  int base = 0;

  DWORD err = GetLastError ();

  intptr_t Rval = 0;
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
		    unsigned char c = (va_arg (ap, int) & 0xff);
		    if (isprint (c) || pad != '0')
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
		      *dst++ = *c;
		  }
		  break;
		case 'E':
		  strcpy (dst, "Win32 error ");
		  dst = __rn (dst + sizeof ("Win32 error"), 10, 0, err, len, pad, LMASK);
		  break;
		case 'R':
		  {
#ifdef __x86_64__
		    if (l_opt)
		      Rval = va_arg (ap, int64_t);
		    else
#endif
		      Rval = va_arg (ap, int32_t);
		    dst = __rn (dst, 10, addsign, Rval, len, pad, LMASK);
		  }
		  break;
		case 'd':
		  base = 10;
#ifdef __x86_64__
		  if (l_opt)
		    goto gen_decimalLL;
#endif
		  goto gen_decimal;
		case 'u':
		  base = 10;
		  addsign = 0;
#ifdef __x86_64__
		  if (l_opt)
		    goto gen_decimalLL;
#endif
		  goto gen_decimal;
		case 'o':
		  base = 8;
		  addsign = 0;
#ifdef __x86_64__
		  if (l_opt)
		    goto gen_decimalLL;
#endif
		  goto gen_decimal;
		case 'y':
		  *dst++ = '0';
		  *dst++ = 'x';
		  /*FALLTHRU*/
		case 'x':
		  base = 16;
		  addsign = 0;
#ifdef __x86_64__
		  if (l_opt)
		    goto gen_decimalLL;
#endif
gen_decimal:
		  dst = rnarg (dst, base, addsign, len, pad);
		  break;
		case 'D':
		  base = 10;
		  goto gen_decimalLL;
		case 'U':
		  base = 10;
		  addsign = 0;
		  goto gen_decimalLL;
		case 'O':
		  base = 8;
		  addsign = 0;
		  goto gen_decimalLL;
		case 'Y':
		  *dst++ = '0';
		  *dst++ = 'x';
		  /*FALLTHRU*/
		case 'X':
		  base = 16;
		  addsign = 0;
gen_decimalLL:
		  dst = rnargLL (dst, base, addsign, len, pad);
		  break;
		case 'p':
		  *dst++ = '0';
		  *dst++ = 'x';
#ifdef __x86_64__
		  dst = rnargLL (dst, 16, 0, len, pad);
#else
		  dst = rnarg (dst, 16, 0, len, pad);
#endif
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
  if (Rval < 0)
    {
      dst = stpcpy (dst, ", errno ");
      dst = __rn (dst, 10, false, get_errno (), 0, 0, LMASK);
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
  tmpbuf tmp;
  PWCHAR orig = dst;
  const char *s;
  PWCHAR w;
  UNICODE_STRING uw, *us;
  int base = 0;

  DWORD err = GetLastError ();

  intptr_t Rval = 0;
  while (*fmt)
    {
      unsigned int n = 0x7fff;
#ifdef __x86_64__
      bool l_opt = false;
#endif
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
#ifdef __x86_64__
		  l_opt = true;
#endif
		  continue;
		case L'c':
		case L'C':
		  *dst++ = va_arg (ap, unsigned);
		  break;
		case L'E':
		  wcscpy (dst, L"Win32 error ");
		  dst = __wrn (dst + sizeof ("Win32 error"), 10, 0, err, len, pad, LMASK);
		  break;
		case 'R':
		  {
#ifdef __x86_64__
		    if (l_opt)
		      Rval = va_arg (ap, int64_t);
		    else
#endif
		      Rval = va_arg (ap, int32_t);
		    dst = __wrn (dst, 10, addsign, Rval, len, pad, LMASK);
		  }
		  break;
		case L'd':
		  base = 10;
#ifdef __x86_64__
		  if (l_opt)
		    goto gen_decimalLL;
#endif
		  goto gen_decimal;
		case 'u':
		  base = 10;
		  addsign = 0;
#ifdef __x86_64__
		  if (l_opt)
		    goto gen_decimalLL;
#endif
		  goto gen_decimal;
		case 'o':
		  base = 8;
		  addsign = 0;
#ifdef __x86_64__
		  if (l_opt)
		    goto gen_decimalLL;
#endif
		  goto gen_decimal;
		case 'y':
		  *dst++ = '0';
		  *dst++ = 'x';
		  /*FALLTHRU*/
		case 'x':
		  base = 16;
		  addsign = 0;
#ifdef __x86_64__
		  if (l_opt)
		    goto gen_decimalLL;
#endif
gen_decimal:
		  dst = wrnarg (dst, base, addsign, len, pad);
		  break;
		case 'D':
		  base = 10;
		  goto gen_decimalLL;
		case 'U':
		  base = 10;
		  addsign = 0;
		  goto gen_decimalLL;
		case 'O':
		  base = 8;
		  addsign = 0;
		  goto gen_decimalLL;
		case 'Y':
		  *dst++ = '0';
		  *dst++ = 'x';
		  /*FALLTHRU*/
		case 'X':
		  base = 16;
		  addsign = 0;
gen_decimalLL:
		  dst = wrnargLL (dst, base, addsign, len, pad);
		  break;
		case L'p':
		  *dst++ = L'0';
		  *dst++ = L'x';
#ifdef __x86_64__
		  dst = wrnargLL (dst, 16, 0, len, pad);
#else
		  dst = wrnarg (dst, 16, 0, len, pad);
#endif
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
  if (Rval < 0)     
    {
      dst = wcpcpy (dst, L", errno ");
      dst = __wrn (dst, 10, false, get_errno (), 0, 0, LMASK);
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
