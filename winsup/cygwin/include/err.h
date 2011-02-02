/* err.h

   Copyright 2003, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _ERR_H
#define _ERR_H

#include <sys/cdefs.h>
#include <stdarg.h>

__BEGIN_DECLS

extern void warn (const char *fmt, ...);
extern void warnx (const char *fmt, ...);

extern void err (int eval, const char *fmt, ...) __attribute__ ((__noreturn__));
extern void errx (int eval, const char *fmt, ...) __attribute__ ((__noreturn__));

extern void vwarn (const char *fmt, va_list ap);
extern void vwarnx (const char *fmt, va_list ap);

extern void verr (int eval, const char *fmt, va_list ap) __attribute__ ((__noreturn__));
extern void verrx (int eval, const char *fmt, va_list ap) __attribute__ ((__noreturn__));

__END_DECLS

#endif /* _ERR_H */
