/* cygerrno.h: main Cygwin header file.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGERRNO_H
#define _CYGERRNO_H
#include <errno.h>
#include "regparm.h"

void __reg3 seterrno_from_win_error (const char *file, int line, DWORD code);
void __reg3 seterrno_from_nt_status (const char *file, int line, NTSTATUS status);
int __reg2 geterrno_from_win_error (DWORD code = GetLastError (), int deferrno = 13 /*EACCESS*/);
int __reg2 geterrno_from_nt_status (NTSTATUS status, int deferrno = 13 /*EACCESS*/);

inline void __attribute__ ((always_inline))
seterrno (const char *file, int line)
{
  seterrno_from_win_error (file, line, GetLastError ());
}

#define __seterrno() seterrno (__FILE__, __LINE__)
#define __seterrno_from_win_error(val) seterrno_from_win_error (__FILE__, __LINE__, val)
#define __seterrno_from_nt_status(status) seterrno_from_nt_status (__FILE__, __LINE__, status)

extern inline int
__set_errno (const char *fn, int ln, int val)
{
  debug_printf ("%s:%d setting errno %d", fn, ln, val);
  return errno = _impure_ptr->_errno = val;
}
#define set_errno(val) __set_errno (__PRETTY_FUNCTION__, __LINE__, (val))

#define get_errno()  (errno)
extern "C" void __stdcall set_sig_errno (int e);

class save_errno
  {
    int saved;
  public:
    save_errno () {saved = get_errno ();}
    save_errno (int what) {saved = get_errno (); set_errno (what); }
    void set (int what) {set_errno (what); saved = what;}
    void reset () {saved = get_errno ();}
    ~save_errno () {errno = _impure_ptr->_errno = saved;}
  };

extern const char *__sp_fn;
extern int __sp_ln;
#endif /*_CYGERRNO_H*/
