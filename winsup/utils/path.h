/* path.h

   Copyright 2001, 2002, 2003 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

char *cygpath (const char *s, ...);
char *cygpath_rel (const char *cwd, const char *s, ...);
bool is_exe (HANDLE);
bool is_symlink (HANDLE);
bool readlink (HANDLE, char *, int);
int get_word (HANDLE, int);
int get_dword (HANDLE, int);

#define SYMLINK_MAX 4095  /* PATH_MAX - 1 */
