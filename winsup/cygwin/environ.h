/* environ.h: Declarations for environ manipulation

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Initialize the environment */
void environ_init (char **, int);

/* The structure below is used to control conversion to/from posix-style
 * file specs.  Currently, only PATH and HOME are converted, but PATH
 * needs to use a "convert path list" function while HOME needs a simple
 * "convert to posix/win32".  For the simple case, where a calculated length
 * is required, just return MAX_PATH.  *FIXME*
 */
struct win_env
  {
    const char *name;
    size_t namelen;
    char *posix;
    char *native;
    int (*toposix) (const char *, char *);
    int (*towin32) (const char *, char *);
    int (*posix_len) (const char *);
    int (*win32_len) (const char *);
    void add_cache (const char *in_posix, const char *in_native = NULL);
    const char * get_native () {return native ? native + namelen : NULL;}
  };

win_env * __stdcall getwinenv (const char *name, const char *posix = NULL);

void __stdcall update_envptrs ();
char * __stdcall winenv (const char * const *, int);
extern char **__cygwin_environ, ***main_environ;
extern "C" char __stdcall **cur_environ ();
int __stdcall envsize (const char * const *, int debug_print = 0);
