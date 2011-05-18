/* error.h: GNU error reporting functions

   Copyright 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _ERROR_H
#define _ERROR_H

#ifdef __cplusplus
extern "C"
{
#endif

void error (int, int, const char *, ...);
void error_at_line (int, int, const char *, unsigned int, const char *, ...);

extern unsigned int error_message_count;
extern int error_one_per_line;
extern void (*error_print_progname) (void);

#ifdef __cplusplus
}
#endif

#endif /* _ERROR_H */
