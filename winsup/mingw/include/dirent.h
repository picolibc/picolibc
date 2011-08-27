/*
 * DIRENT.H (formerly DIRLIB.H)
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 */
#ifndef _DIRENT_H_
#define _DIRENT_H_

/* All the headers include this file. */
#include <_mingw.h>

#include <io.h>

#ifndef RC_INVOKED

#ifdef __cplusplus
extern "C" {
#endif

struct dirent
{
	long		d_ino;		/* Always zero. */
	unsigned short	d_reclen;	/* Always zero. */
	unsigned short	d_namlen;	/* Length of name in d_name. */
	char		d_name[FILENAME_MAX]; /* File name. */
};

/*
 * This opaque data type represents the private structure
 * through which a directory stream is referenced.
 */
typedef struct __dirstream_t DIR;

DIR* __cdecl __MINGW_NOTHROW opendir (const char*);
struct dirent* __cdecl __MINGW_NOTHROW readdir (DIR*);
int __cdecl __MINGW_NOTHROW closedir (DIR*);
void __cdecl __MINGW_NOTHROW rewinddir (DIR*);
long __cdecl __MINGW_NOTHROW telldir (DIR*);
void __cdecl __MINGW_NOTHROW seekdir (DIR*, long);


/* wide char versions */

struct _wdirent
{
	long		d_ino;		/* Always zero. */
	unsigned short	d_reclen;	/* Always zero. */
	unsigned short	d_namlen;	/* Length of name in d_name. */
	wchar_t		d_name[FILENAME_MAX]; /* File name. */
};

/*
 * This opaque data type represents the private structure
 * through which a wide directory stream is referenced.
 */
typedef struct __wdirstream_t _WDIR;

_WDIR* __cdecl __MINGW_NOTHROW _wopendir (const wchar_t*);
struct _wdirent*  __cdecl __MINGW_NOTHROW _wreaddir (_WDIR*);
int __cdecl __MINGW_NOTHROW _wclosedir (_WDIR*);
void __cdecl __MINGW_NOTHROW _wrewinddir (_WDIR*);
long __cdecl __MINGW_NOTHROW _wtelldir (_WDIR*);
void __cdecl __MINGW_NOTHROW _wseekdir (_WDIR*, long);


#ifdef	__cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* Not _DIRENT_H_ */
