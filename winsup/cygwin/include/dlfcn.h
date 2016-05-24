/* dlfcn.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _DLFCN_H
#define _DLFCN_H

#ifdef __cplusplus
extern "C" {
#endif

/* declarations used for dynamic linking support routines */
extern void *dlopen (const char *, int);
extern void *dlsym (void *, const char *);
extern int dlclose (void *);
extern char *dlerror (void);

/* specific to CYGWIN */
#define FORK_RELOAD	1
#define FORK_NO_RELOAD	0

extern void dlfork (int);

/* following doesn't exist in Win32 API .... */
#define RTLD_DEFAULT    NULL

/* valid values for mode argument to dlopen */
#define RTLD_LOCAL	0	/* Symbols in this dlopen'ed obj are not     */
				/* visible to other dlopen'ed objs.          */
#define RTLD_LAZY	1	/* Lazy function call binding.               */
#define RTLD_NOW	2	/* Immediate function call binding.          */
#define RTLD_GLOBAL	4	/* Symbols in this dlopen'ed obj are visible */
				/* to other dlopen'ed objs.                  */
/* Non-standard GLIBC extensions */
#define RTLD_NODELETE	8	/* Don't unload lib in dlcose.               */
#define RTLD_NOLOAD    16	/* Don't load lib, just return handle if lib */
				/* is already loaded, NULL otherwise.        */
#define RTLD_DEEPBIND  32	/* Place lookup scope so that this lib is    */
				/* preferred over global scope.  */

#ifdef __cplusplus
}
#endif

#endif /* _DLFCN_H */
