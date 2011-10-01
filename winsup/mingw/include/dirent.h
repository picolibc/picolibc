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

	/* The following exactly mimic the layout of _finddata_t ...
	 */
	unsigned	d_type;		/* File attributes */
	time_t		d_time_create;
	time_t		d_time_access;	/* always midnight local time */
	time_t		d_time_write;
	_fsize_t	d_size;
	/*
	 * ...so that we may map a union of _finddata_t at the
	 * location of d_type (corresponding to _finddata_t.attrib),
	 * and thus map this directly to the _findfirst/_findnext
	 * returned field.
	 */
	char		d_name[FILENAME_MAX]; /* File name. */
};

/*
 * This opaque data type represents the private structure
 * through which a directory stream is referenced.
 */
typedef union __dirstream_t DIR;

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

	/* The following exactly mimic the layout of _wfinddata_t ...
	 */
	unsigned	d_type;		/* File attributes */
    	time_t		d_time_create;	/* -1 for FAT file systems */
    	time_t		d_time_access;	/* -1 for FAT file systems */
    	time_t		d_time_write;
    	_fsize_t	d_size;
	/*
	 * ...so that we may map a union of _wfinddata_t at the
	 * location of d_type (corresponding to _wfinddata_t.attrib),
	 * and thus map this directly to the _wfindfirst/_wfindnext
	 * returned field.
	 */
	wchar_t		d_name[FILENAME_MAX]; /* File name. */
};

/*
 * This opaque data type represents the private structure
 * through which a wide directory stream is referenced.
 */
typedef union __wdirstream_t _WDIR;

_WDIR* __cdecl __MINGW_NOTHROW _wopendir (const wchar_t*);
struct _wdirent*  __cdecl __MINGW_NOTHROW _wreaddir (_WDIR*);
int __cdecl __MINGW_NOTHROW _wclosedir (_WDIR*);
void __cdecl __MINGW_NOTHROW _wrewinddir (_WDIR*);
long __cdecl __MINGW_NOTHROW _wtelldir (_WDIR*);
void __cdecl __MINGW_NOTHROW _wseekdir (_WDIR*, long);


#ifdef	__cplusplus
}
#endif

#if defined(_BSD_SOURCE) || defined(_WIN32)
/*
 * BSD-ish systems define manifest constants for the d_type field;
 * although probably only DT_REG and DT_DIR are useful on Win32, we
 * try to map them as best we can from the _finddata.attrib field.
 *
 * The relevant Microsoft manifest values are:
 *
 *   _A_NORMAL	(0x0000)	normal file: best fit for DT_REG
 *   _A_RDONLY	(0x0001)	read-only: no BSD d_type equivalent
 *   _A_HIDDEN	(0x0002)	hidden entity: no BSD equivalent
 *   _A_SYSTEM	(0x0004)	system entity: no BSD equivalent
 *   _A_VOLID	(0x0008)	volume label: no BSD equivalent
 *   _A_SUBDIR	(0x0010)	directory: best fit for DT_DIR
 *   _A_ARCH	(0x0020)	"dirty": no BSD equivalent
 *
 * Thus, we may immediately define:
 */
#define DT_REG   	_A_NORMAL
#define DT_DIR   	_A_SUBDIR

/* The remaining BSD d_type manifest values have no Win32 equivalents;
 * we will define them artificially, and then we will ensure that our
 * opendir()/readdir() implementation will never assign them; (we will
 * substitute DT_UNKNOWN, but it would be unwise to simply make these
 * equivalent to that, since an application is likely to simply check
 * for d_type equal to any one of these defined types, and thus could
 * mistakenly identify DT_UNKNOWN as being of the tested type):
 */
#define DT_BLK   	(((_A_SUBDIR) << 4) | DT_UNKNOWN)
#define DT_CHR   	(((_A_SUBDIR) << 5) | DT_UNKNOWN)
#define DT_FIFO  	(((_A_SUBDIR) << 6) | DT_UNKNOWN)
#define DT_LNK   	(((_A_SUBDIR) << 7) | DT_UNKNOWN)
#define DT_SOCK  	(((_A_SUBDIR) << 8) | DT_UNKNOWN)

/* No file system entity can ever be simultaneously a volume label
 * and a directory; we will exploit this to unambiguously define:
 */
#define DT_UNKNOWN	(_A_VOLID | _A_SUBDIR)

#endif  /* _BSD_SOURCE */
#endif	/* ! RC_INVOKED */

#endif	/* !defined _DIRENT_H_ */
