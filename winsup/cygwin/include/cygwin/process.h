/* cygwin/process.h.  Define spawn family of functions as provided by Cygwin.
   The original file of this name is a MS/DOS invention.
   
This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __PROCESS_H_
#define __PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* For the exec functions, include unistd.h. */

int spawnl(int mode, const char *path, const char *argv0, ...);
int spawnle(int mode, const char *path, const char *argv0, ... /*, char * const *envp */);
int spawnlp(int mode, const char *path, const char *argv0, ...);
int spawnlpe(int mode, const char *path, const char *argv0, ... /*, char * const *envp */);

int spawnv(int mode, const char *path, const char * const *argv);
int spawnve(int mode, const char *path, const char * const *argv, const char * const *envp);
int spawnvp(int mode, const char *path, const char * const *argv);
int spawnvpe(int mode, const char *path, const char * const *argv, const char * const *envp);

int cwait(int *, int, int);

#define _P_WAIT		1
#define _P_NOWAIT	2
#define _P_OVERLAY	3
#define _P_NOWAITO	4
#define _P_DETACH	5

#define WAIT_CHILD 1

#ifdef __cplusplus
}
#endif

#endif
