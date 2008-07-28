#ifndef PFORMAT_H
/*
 * pformat.h
 *
 * $Id$
 *
 * A private header, defining the `pformat' API; it is to be included
 * in each compilation unit implementing any of the `printf' family of
 * functions, but serves no useful purpose elsewhere.
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 *
 * This is free software.  You may redistribute and/or modify it as you
 * see fit, without restriction of copyright.
 *
 * This software is provided "as is", in the hope that it may be useful,
 * but WITHOUT WARRANTY OF ANY KIND, not even any implied warranty of
 * MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.  At no
 * time will the author accept any form of liability for any damages,
 * however caused, resulting from the use of this software.
 */
#define PFORMAT_H

/* The following macros reproduce definitions from _mingw.h,
 * so that compilation will not choke, if using any compiler
 * other than the MinGW implementation of GCC.
 */
#ifndef __cdecl
# ifdef __GNUC__
#  define __cdecl __attribute__((__cdecl__))
# else
#  define __cdecl
# endif
#endif

#ifndef __MINGW_GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __MINGW_GNUC_PREREQ( major, minor )\
     (__GNUC__ > (major) || (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
# else
#  define __MINGW_GNUC_PREREQ( major, minor )
# endif
#endif

#ifndef  __MINGW_NOTHROW
# if __MINGW_GNUC_PREREQ( 3, 3 )
#  define __MINGW_NOTHROW  __attribute__((__nothrow__))
# else
#  define __MINGW_NOTHROW
# endif
#endif

/* This isn't currently defined therein,
 * but is a potential candidate for inclusion in _mingw.h
 */
#ifdef __MINGW32__
# define __stringify__(NAME)    #NAME
# define __mingw_quoted(NAME)  __stringify__(__mingw_##NAME)
# define __mingw_alias(NAME)   __attribute__((alias(__mingw_quoted(NAME)))) NAME
#else
# define __mingw_alias(NAME)   NAME
#endif

/* The following are the declarations specific to the `pformat' API...
 */
#define PFORMAT_TO_FILE     0x1000
#define PFORMAT_NOLIMIT     0x2000

#ifdef __MINGW32__
 /*
  * Map MinGW specific function names, for use in place of the generic
  * implementation defined equivalent function names.
  */
# define __pformat        __mingw_pformat

# define __printf         __mingw_printf
# define __fprintf        __mingw_fprintf
# define __sprintf        __mingw_sprintf
# define __snprintf       __mingw_snprintf

# define __vprintf        __mingw_vprintf
# define __vfprintf       __mingw_vfprintf
# define __vsprintf       __mingw_vsprintf
# define __vsnprintf      __mingw_vsnprintf

#endif

int __cdecl __pformat( int, void *, int, const char *, va_list ) __MINGW_NOTHROW;

#endif /* !defined PFORMAT_H: $RCSfile$Revision$: end of file */
