/* shortcut.h: Hader file for shortcut.c

   Copyright 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __cplusplus
extern "C" {
#endif

/* The header written to a shortcut by Cygwin or U/WIN. */
#define SHORTCUT_HDR_SIZE	76

extern char shortcut_header[];
extern BOOL shortcut_initalized;

extern void create_shortcut_header ();

int check_shortcut (const char *path, DWORD fileattr, HANDLE h,
		    char *contents, int *error, unsigned *pflags);

#ifdef __cplusplus
};
#endif
