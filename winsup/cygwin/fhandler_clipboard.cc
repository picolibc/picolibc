/* fhandler_dev_clipboard: code to access /dev/clipboard

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2011 Red Hat, Inc

   Written by Charles Wilson (cwilson@ece.gatech.edu)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <wchar.h>
#include <wingdi.h>
#include <winuser.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"

/*
 * Robert Collins:
 * FIXME: should we use GetClipboardSequenceNumber to tell if the clipboard has
 * changed? How does /dev/clipboard operate under (say) linux?
 */

static const NO_COPY WCHAR *CYGWIN_NATIVE = L"CYGWIN_NATIVE_CLIPBOARD";
/* this is MT safe because windows format id's are atomic */
static int cygnativeformat;

fhandler_dev_clipboard::fhandler_dev_clipboard ()
  : fhandler_base (), pos (0), membuffer (NULL), msize (0),
  eof (true)
{
  /* FIXME: check for errors and loop until we can open the clipboard */
  OpenClipboard (NULL);
  cygnativeformat = RegisterClipboardFormatW (CYGWIN_NATIVE);
  CloseClipboard ();
}

/*
 * Special clipboard dup to duplicate input and output
 * handles.
 */

int
fhandler_dev_clipboard::dup (fhandler_base * child)
{
  fhandler_dev_clipboard *fhc = (fhandler_dev_clipboard *) child;

  if (!fhc->open (get_flags (), 0))
    system_printf ("error opening clipboard, %E");
  return 0;
}

int
fhandler_dev_clipboard::open (int flags, mode_t)
{
  set_flags (flags | O_TEXT);
  eof = false;
  pos = 0;
  if (membuffer)
    free (membuffer);
  membuffer = NULL;
  if (!cygnativeformat)
    cygnativeformat = RegisterClipboardFormatW (CYGWIN_NATIVE);
  nohandle (true);
  set_open_status ();
  return 1;
}

static int
set_clipboard (const void *buf, size_t len)
{
  HGLOBAL hmem;
  void *clipbuf;
  /* Native CYGWIN format */
  if (OpenClipboard (NULL))
    {
      hmem = GlobalAlloc (GMEM_MOVEABLE, len + sizeof (size_t));
      if (!hmem)
	{
	  __seterrno ();
	  CloseClipboard ();
	  return -1;
	}
      clipbuf = GlobalLock (hmem);
      memcpy ((unsigned char *) clipbuf + sizeof (size_t), buf, len);
      *(size_t *) (clipbuf) = len;
      GlobalUnlock (hmem);
      EmptyClipboard ();
      if (!cygnativeformat)
	cygnativeformat = RegisterClipboardFormatW (CYGWIN_NATIVE);
      HANDLE ret = SetClipboardData (cygnativeformat, hmem);
      CloseClipboard ();
      /* According to MSDN, hmem must not be free'd after transferring the
	 data to the clipboard via SetClipboardData. */
      /* GlobalFree (hmem); */
      if (!ret)
	{
	  __seterrno ();
	  return -1;
	}
    }

  /* CF_TEXT/CF_OEMTEXT for copying to wordpad and the like */
  len = sys_mbstowcs (NULL, 0, (const char *) buf, len);
  if (!len)
    {
      set_errno (EILSEQ);
      return -1;
    }
  if (OpenClipboard (NULL))
    {
      hmem = GlobalAlloc (GMEM_MOVEABLE, (len + 1) * sizeof (WCHAR));
      if (!hmem)
	{
	  __seterrno ();
	  CloseClipboard ();
	  return -1;
	}
      clipbuf = GlobalLock (hmem);
      sys_mbstowcs ((PWCHAR) clipbuf, len + 1, (const char *) buf);
      GlobalUnlock (hmem);
      HANDLE ret = SetClipboardData (CF_UNICODETEXT, hmem);
      CloseClipboard ();
      /* According to MSDN, hmem must not be free'd after transferring the
	 data to the clipboard via SetClipboardData. */
      /* GlobalFree (hmem); */
      if (!ret)
	{
	  __seterrno ();
	  return -1;
	}
    }
  return 0;
}

/* FIXME: arbitrary seeking is not handled */
ssize_t __stdcall
fhandler_dev_clipboard::write (const void *buf, size_t len)
{
  if (!eof)
    {
      /* write to our membuffer */
      size_t cursize = msize;
      void *tempbuffer = realloc (membuffer, cursize + len);
      if (!tempbuffer)
	{
	  debug_printf ("Couldn't realloc() clipboard buffer for write");
	  return -1;
	}
      membuffer = tempbuffer;
      msize = cursize + len;
      memcpy ((unsigned char *) membuffer + cursize, buf, len);

      /* now pass to windows */
      if (set_clipboard (membuffer, msize))
	{
	  /* FIXME: membuffer is now out of sync with pos, but msize
		    is used above */
	  return -1;
	}

      pos = msize;

      eof = false;
      return len;
    }
  else
    {
      /* FIXME: return 0 bytes written, file not open */
      return 0;
    }
}

void __stdcall
fhandler_dev_clipboard::read (void *ptr, size_t& len)
{
  HGLOBAL hglb;
  size_t ret;
  UINT formatlist[2];
  int format;
  size_t plen = len;

  len = 0;
  if (eof)
    return;
  if (!OpenClipboard (NULL))
    return;
  formatlist[0] = cygnativeformat;
  formatlist[1] = CF_UNICODETEXT;
  if ((format = GetPriorityClipboardFormat (formatlist, 2)) <= 0)
    {
      CloseClipboard ();
      return;
    }
  if (!(hglb = GetClipboardData (format)))
    {
      CloseClipboard ();
      return;
    }
  if (format == cygnativeformat)
    {
      unsigned char *buf;

      if (!(buf = (unsigned char *) GlobalLock (hglb)))
	{
	  CloseClipboard ();
	  return;
	}
      size_t buflen = (*(size_t *) buf);
      ret = ((plen > (buflen - pos)) ? (buflen - pos) : plen);
      memcpy (ptr, buf + sizeof (size_t)+ pos , ret);
      pos += ret;
      if (pos + plen - ret >= buflen)
	eof = true;
    }
  else
    {
      int wret;
      PWCHAR buf;

      if (!(buf = (PWCHAR) GlobalLock (hglb)))
	{
	  CloseClipboard ();
	  return;
	}
      size_t glen = GlobalSize (hglb) / sizeof (WCHAR) - 1;
      /* This loop is necessary because the number of bytes returned by
	 sys_wcstombs does not indicate the number of wide chars used for
	 it, so we could potentially drop wide chars. */
      if (glen - pos > plen)
	glen = pos + plen;
      while ((wret = sys_wcstombs (NULL, 0, buf + pos, glen - pos)) != -1
	     && (size_t) wret > plen)
	--glen;
      ret = sys_wcstombs ((char *) ptr, plen, buf + pos, glen - pos);
      pos += ret;
      if (pos + plen - ret >= wcslen (buf))
	eof = true;
    }
  GlobalUnlock (hglb);
  CloseClipboard ();
  len = ret;
}

_off64_t
fhandler_dev_clipboard::lseek (_off64_t offset, int whence)
{
  /* On reads we check this at read time, not seek time.
   * On writes we use this to decide how to write - empty and write, or open, copy, empty
   * and write
   */
  pos = offset;
  /* treat seek like rewind */
  if (membuffer)
    free (membuffer);
  msize = 0;
  return 0;
}

int
fhandler_dev_clipboard::close ()
{
  if (!hExeced)
    {
      eof = true;
      pos = 0;
      if (membuffer)
	{
	  free (membuffer);
	  membuffer = NULL;
	}
      msize = 0;
    }
  return 0;
}

void
fhandler_dev_clipboard::fixup_after_exec ()
{
  if (!close_on_exec ())
    {
      eof = false;
      pos = msize = 0;
      membuffer = NULL;
    }
}
