/* fhandler_dev_clipboard: code to access /dev/clipboard

   Copyright 2000 Red Hat, Inc

   Written by Charles Wilson (cwilson@ece.gatech.edu)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <errno.h>
#include "cygerrno.h"
#include "fhandler.h"
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

static BOOL clipboard_eof; // NOT THREAD-SAFE

fhandler_dev_clipboard::fhandler_dev_clipboard (const char *name)
  : fhandler_base (FH_CLIPBOARD, name)
{
  set_cb (sizeof *this);
  clipboard_eof = false;
}

int
fhandler_dev_clipboard::open (const char *, int flags, mode_t)
{
  set_flags (flags);
  clipboard_eof = false;
  return 1;
}

int
fhandler_dev_clipboard::write (const void *, size_t len)
{
  return len;
}

int
fhandler_dev_clipboard::read (void *ptr, size_t len)
{
  HGLOBAL hglb;
  LPSTR lpstr;
  int ret;
 
  if (!clipboard_eof)
    {
      OpenClipboard(0);
      hglb = GetClipboardData(CF_TEXT);
      lpstr = (LPSTR) GlobalLock(hglb);
      if (len < sizeof (lpstr))
        {
          set_errno (EINVAL);
          GlobalUnlock (hglb);
          CloseClipboard ();
          return -1;
        }

      ret = snprintf((char *) ptr, len, "%s", lpstr);
      GlobalUnlock(hglb);
      CloseClipboard();
      set_errno (0);
      clipboard_eof = true;
      return ret;
    }
  else
    {
      return 0;
    }
}

off_t
fhandler_dev_clipboard::lseek (off_t offset, int whence)
{
  return 0;
}

int
fhandler_dev_clipboard::close (void)
{
  clipboard_eof = false;
  return 0;
}

void
fhandler_dev_clipboard::dump ()
{
  paranoid_printf("here, fhandler_dev_clipboard");
}
