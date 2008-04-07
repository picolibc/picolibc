/* miscfuncs.h: main Cygwin header file.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _MISCFUNCS_H
#define _MISCFUNCS_H
int winprio_to_nice (DWORD) __attribute__ ((regparm (1)));
DWORD nice_to_winprio (int &) __attribute__ ((regparm (1)));

bool __stdcall create_pipe (PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD)
  __attribute__ ((regparm (3)));
#define CreatePipe create_pipe

extern "C" int low_priority_sleep (DWORD) __attribute__ ((regparm (1)));
#define SLEEP_0_STAY_LOW INFINITE

void backslashify (const char *, char *, bool);
void slashify (const char *, char *, bool);
#define isslash(c) ((c) == '/')

/* multibyte stuff */
bool is_cp_multibyte (UINT cp);
const unsigned char *next_char (UINT cp, const unsigned char *str,
				const unsigned char *end);

/* Memory checking */
int __stdcall check_invalid_virtual_addr (const void *s, unsigned sz) __attribute__ ((regparm(2)));

ssize_t check_iovec (const struct iovec *, int, bool) __attribute__ ((regparm(3)));
#define check_iovec_for_read(a, b) check_iovec ((a), (b), false)
#define check_iovec_for_write(a, b) check_iovec ((a), (b), true)
#endif /*_MISCFUNCS_H*/
