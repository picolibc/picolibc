/* sys/clipboard.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_CLIPBOARD_H_
#define _SYS_CLIPBOARD_H_

/*
 * These definitions are used in fhandler_clipboard.cc
 * as well as in the Cygwin cygutils package, specifically
 * getclip.c and putclip.c.
 */

static const WCHAR *CYGWIN_NATIVE = L"CYGWIN_NATIVE_CLIPBOARD";

typedef struct
{
  union
  {
    struct timespec ts;
    struct
    {
      uint64_t  cb_sec;  // == ts.tv_sec
      uint64_t  cb_nsec; // == ts.tv_nsec
    };
  };
  uint64_t      cb_size;
  char          cb_data[];
} cygcb_t;

#endif
