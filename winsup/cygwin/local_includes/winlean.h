/* winlean.h - Standard "lean" windows include

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _WINLEAN_H
#define _WINLEAN_H 1
#define WIN32_LEAN_AND_MEAN 1

/* The following macros have to be defined, otherwise the autoload mechanism
   in autoload.cc leads to "multiple definition" errors.  The macros control
   the declarations of symbols in the Mingw64 w32api headers.  If they are
   not defined, a DECLSPEC_IMPORT will be added to the symbol declarations.
   This leads to a definition of the symbols in the sources using the
   autoloaded symbols, which in turn clashes with the definition in the
   w32api library exporting the symbols. */
#define _ADVAPI32_
#define _AUTHZ_
#define _DSGETDCAPI_
#define _GDI32_
#define _KERNEL32_
#define _NORMALIZE_
#define _OLE32_
#define _SHELL32_
#define _SPOOL32_
#define _USER32_
#define _USERENV_
#define _WINMM_
#define WINIMPM
#define WINSOCK_API_LINKAGE

/* Windows headers define a couple of annoyingly intrusive macros for the
   sole purpose of inline documentation.  Since they are defined without
   respect for the namespace and not undef'ed anymore, they tend to collide
   with otherwise innocent definitions in the application.  We check if they
   exist and if not, we undef them again after including the Windows headers. */
#ifndef IN
#define __undef_IN
#endif
#ifndef OUT
#define __undef_OUT
#endif
#ifndef OPTIONAL
#define __undef_OPTIONAL
#endif
#ifndef NOTHING
#define __undef_NOTHING
#endif
#ifndef CRITICAL
#define __undef_CRITICAL
#endif

#include <windows.h>
#include <wincrypt.h>
#include <lmcons.h>
#include <ntdef.h>

#ifdef __undef_IN
#undef IN
#endif
#ifdef __undef_OUT
#undef OUT
#endif
#ifdef __undef_OPTIONAL
#undef OPTIONAL
#endif
#ifdef __undef_NOTHING
#undef NOTHING
#endif
#ifdef __undef_CRITICAL
#undef CRITICAL
#endif

/* Filesystem flags not yet supported by Mingw-w64 headers. */
#ifndef FILE_RETURNS_CLEANUP_RESULT_INFO
#define FILE_RETURNS_CLEANUP_RESULT_INFO        0x00000200
#endif
#ifndef FILE_SUPPORTS_POSIX_UNLINK_RENAME
#define FILE_SUPPORTS_POSIX_UNLINK_RENAME       0x00000400
#endif
#ifndef FILE_SUPPORTS_INTEGRITY_STREAMS
#define FILE_SUPPORTS_INTEGRITY_STREAMS         0x04000000
#endif
#ifndef FILE_SUPPORTS_BLOCK_REFCOUNTING
#define FILE_SUPPORTS_BLOCK_REFCOUNTING         0x08000000
#endif
#ifndef FILE_SUPPORTS_SPARSE_VDL
#define FILE_SUPPORTS_SPARSE_VDL                0x10000000
#endif
#ifndef FILE_DAX_VOLUME
#define FILE_DAX_VOLUME                         0x20000000
#endif
#ifndef FILE_SUPPORTS_GHOSTING
#define FILE_SUPPORTS_GHOSTING                  0x40000000
#endif

/* So-called "Microsoft Account" SIDs (S-1-11-...) have a netbios domain name
   "MicrosoftAccounts".  The new "Application Container SIDs" (S-1-15-...)
   have a netbios domain name "APPLICATION PACKAGE AUTHORITY"

   The problem is, DNLEN is 15, but these domain names have a length of 16
   resp. 29 chars :-P  So we override DNLEN here to be 31, so that calls
   to LookupAccountSid/Name don't fail if the buffer is based on DNLEN.
   Hope that's enough for a while... */
#undef DNLEN
#define DNLEN 31

/* When Terminal Services are installed, the GetWindowsDirectory function
   does not return the system installation dir, but a user specific directory
   instead.  That's not what we have in mind when calling GetWindowsDirectory
   from within Cygwin.  So we redefine GetWindowsDirectory to something
   invalid here to avoid that it's called accidentally in Cygwin.  Don't
   use this function.  Use GetSystemWindowsDirectoryW. */
#define GetWindowsDirectoryW dont_use_GetWindowsDirectory
#define GetWindowsDirectoryA dont_use_GetWindowsDirectory

#endif /*_WINLEAN_H*/
