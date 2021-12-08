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

/*
 * The following layout of cygcb_t is new with Cygwin 3.3.0.  It aids in the
 * transfer of clipboard contents between 32- and 64-bit Cygwin environments.
 */
typedef struct
{
  union
  {
    /*
     * Note that ts below overlays the struct following it.  On 32-bit Cygwin
     * timespec values have to be converted to|from cygcb_t layout.  On 64-bit
     * Cygwin timespec values perfectly conform to the struct following, so
     * no conversion is needed.
     *
     * We avoid directly using 'struct timespec' or 'size_t' here because they
     * are different sizes on different architectures.  When copy/pasting
     * between 32- and 64-bit Cygwin, the pasted data could appear corrupted,
     * or partially interpreted as a size which can cause an access violation.
     */
    struct timespec ts;  // 8 bytes on 32-bit Cygwin, 16 bytes on 64-bit Cygwin
    struct
    {
      uint64_t  cb_sec;  // 8 bytes everywhere
      uint64_t  cb_nsec; // 8 bytes everywhere
    };
  };
  uint64_t      cb_size; // 8 bytes everywhere
  char          cb_data[];
} cygcb_t;

#endif
