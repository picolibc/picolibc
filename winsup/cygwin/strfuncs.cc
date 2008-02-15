/* strfuncs.cc: misc funcs that don't belong anywhere else

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <winbase.h>
#include <winnls.h>
#include <ntdll.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"

codepage_type current_codepage = ansi_cp;
UINT active_codepage = 0;

UINT
get_cp ()
{
  if (!active_codepage)
    codepage_init ("ansi");
  return active_codepage;
}

/* tlen is always treated as the maximum buffer size, including the '\0'
   character.  sys_wcstombs will always return a 0-terminated result, no
   matter what. */
int __stdcall
sys_wcstombs (char *tgt, int tlen, const PWCHAR src, int slen)
{
  int ret;

  ret = WideCharToMultiByte (get_cp (), 0, src, slen, tgt, tlen, NULL, NULL);
  if (ret && tgt)
    {
      ret = (ret < tlen) ? ret : tlen - 1;
      tgt[ret] = '\0';
    }
  return ret;
}

/* Allocate a buffer big enough for the string, always including the
   terminating '\0'.  The buffer pointer is returned in *tgt_p, the return
   value is the number of bytes written to the buffer, as usual.
   The "type" argument determines where the resulting buffer is stored.
   It's either one of the cygheap_types values, or it's "HEAP_NOTHEAP".
   In the latter case the allocation uses simple calloc.

   Note that this code is shared by cygserver (which requires it via
   __small_vsprintf) and so when built there plain calloc is the
   only choice.  */
int __stdcall
sys_wcstombs_alloc (char **tgt_p, int type, const PWCHAR src, int slen)
{
  int ret;

  ret = WideCharToMultiByte (get_cp (), 0, src, slen, NULL, 0,NULL, NULL);
  if (ret)
    {
      size_t tlen = (slen == -1 ? ret : ret + 1);

      if (type == HEAP_NOTHEAP)
	*tgt_p = (char *) calloc (tlen, sizeof (char));
      else
	*tgt_p = (char *) ccalloc ((cygheap_types) type, tlen, sizeof (char));
      if (!*tgt_p)
	return 0;
      ret = sys_wcstombs (*tgt_p, tlen, src, slen);
    }
  return ret;
}

int __stdcall
sys_mbstowcs (PWCHAR tgt, const char *src, int len)
{
  int res = MultiByteToWideChar (get_cp (), 0, src, -1, tgt, len);
  return res;
}

/* Same as sys_wcstombs_alloc, just backwards. */
int __stdcall
sys_mbstowcs_alloc (PWCHAR *tgt_p, int type, const char *src)
{
  int ret;

  ret = MultiByteToWideChar (get_cp (), 0, src, -1, NULL, 0);
  if (ret)
    {
      if (type == HEAP_NOTHEAP)
	*tgt_p = (PWCHAR) calloc (ret, sizeof (WCHAR));
      else
	*tgt_p = (PWCHAR) ccalloc ((cygheap_types) type, ret, sizeof (WCHAR));
      if (!*tgt_p)
	return 0;
      ret = sys_mbstowcs (*tgt_p, src, ret);
    }
  return ret;
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
