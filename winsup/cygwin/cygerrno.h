/* cygerrno.h: main Cygwin header file.

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

void __stdcall seterrno_from_win_error (const char *file, int line, DWORD code) __attribute__ ((regparm(3)));
void __stdcall seterrno (const char *, int line) __attribute__ ((regparm(2)));
int __stdcall geterrno_from_win_error (DWORD code, int deferrno) __attribute__ ((regparm(2)));

#define __seterrno() seterrno (__FILE__, __LINE__)
#define __seterrno_from_win_error(val) seterrno_from_win_error (__FILE__, __LINE__, val)

#define set_errno(val) (_impure_ptr->_errno = (val))
#define get_errno()  (_impure_ptr->_errno)
extern "C" void __stdcall set_sig_errno (int e);

class save_errno
  {
    int saved;
  public:
    save_errno () {saved = get_errno ();}
    save_errno (int what) {saved = get_errno (); set_errno (what); }
    void set (int what) {set_errno (what); saved = what;}
    void reset () {saved = get_errno ();}
    ~save_errno () {set_errno (saved);}
  };

extern const char *__sp_fn;
extern int __sp_ln;
