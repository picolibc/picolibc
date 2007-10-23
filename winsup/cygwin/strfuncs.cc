/* strfuncs.cc: misc funcs that don't belong anywhere else

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <winbase.h>
#include <winnls.h>
#include <ntdll.h>

codepage_type current_codepage = ansi_cp;

UINT
get_cp ()
{
  return current_codepage == ansi_cp ? GetACP() : GetOEMCP();
}

/* tlen is always treated as the maximum buffer size, including the '\0'
   character.  sys_wcstombs will always return a 0-terminated result, no
   matter what. */
int __stdcall
sys_wcstombs (char *tgt, int tlen, const WCHAR *src, int slen)
{
  int ret;

  ret = WideCharToMultiByte (get_cp (), 0, src, slen, tgt, tlen, NULL, NULL);
  if (ret)
    {
      ret = (ret < tlen) ? ret : tlen - 1;
      tgt[ret] = '\0';
    }
  return ret;
}

int __stdcall
sys_mbstowcs (WCHAR *tgt, const char *src, int len)
{
  int res = MultiByteToWideChar (get_cp (), 0, src, -1, tgt, len);
  return res;
}

static WCHAR hex_wchars[] = L"0123456789abcdef";

NTSTATUS NTAPI
RtlInt64ToHexUnicodeString (ULONGLONG value, PUNICODE_STRING dest,
			    BOOLEAN append)
{
  USHORT len = append ? dest->Length : 0;
  if (dest->MaximumLength - len < 16 * (int) sizeof (WCHAR))
    return STATUS_BUFFER_OVERFLOW;
  PWCHAR end = (PWCHAR) ((PBYTE) dest->Buffer + len);
  register PWCHAR p = end + 16;
  while (p-- > end)
    {
      *p = hex_wchars[value & 0xf];
      value >>= 4;
    }
  dest->Length += 16 * sizeof (WCHAR);
  return STATUS_SUCCESS;
}
