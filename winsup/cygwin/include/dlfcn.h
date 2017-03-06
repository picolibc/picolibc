/* dlfcn.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _DLFCN_H
#define _DLFCN_H

#include <sys/cdefs.h>
#include <limits.h>

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


#if __GNU_VISIBLE
typedef struct Dl_info Dl_info;

struct Dl_info
{
   char        dli_fname[PATH_MAX];  /* Filename of defining object */
   void       *dli_fbase;            /* Load address of that object */
   const char *dli_sname;            /* Name of nearest lower symbol */
   void       *dli_saddr;            /* Exact value of nearest symbol */
};

extern int dladdr (const void *addr, Dl_info *info);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DLFCN_H */
