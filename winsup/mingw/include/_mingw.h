/*
 * _mingw.h
 *
 * Mingw specific macros included by ALL include files. 
 *
 * This file is part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Mumit Khan  <khan@xraylith.wisc.edu>
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef __MINGW_H
#define __MINGW_H

/* These are defined by the user (or the compiler)
   to specify how identifiers are imported from a DLL.

   __DECLSPEC_SUPPORTED    Defined if dllimport attribute is supported.
   __MINGW_IMPORT          The attribute definition to specify imported
                           variables/functions. 
   _CRTIMP                 As above.  For MS compatibility.  
   __MINGW32_VERSION       Runtime version.
   __MINGW32_MAJOR_VERSION Runtime major version.
   __MINGW32_MINOR_VERSION Runtime minor version.
   __MINGW32_BUILD_DATE    Runtime build date.
   
   Other macros:

   __int64                 define to be long long. Using a typedef can
                           tweak bugs in the C++ parser.
  
   All headers should include this first, and then use __DECLSPEC_SUPPORTED
   to choose between the old ``__imp__name'' style or __MINGW_IMPORT
   style declarations.  */

#ifndef __GNUC__
# ifndef __MINGW_IMPORT
#  define __MINGW_IMPORT  __declspec(dllimport)
# endif
# ifndef _CRTIMP
#  define _CRTIMP  __declspec(dllimport)
# endif
# define __DECLSPEC_SUPPORTED
#else /* __GNUC__ */
# ifdef __declspec
#  ifndef __MINGW_IMPORT
   /* Note the extern. This is needed to work around GCC's
      limitations in handling dllimport attribute.  */
#   define __MINGW_IMPORT  extern __attribute__((dllimport))
#  endif
#  ifndef _CRTIMP
#   ifdef __USE_CRTIMP
#    define _CRTIMP  __attribute__((dllimport))
#   else  
#    define _CRTIMP
#   endif
#  endif
#  define __DECLSPEC_SUPPORTED
# else /* __declspec */
#  undef __DECLSPEC_SUPPORTED
#  undef __MINGW_IMPORT
#  ifndef _CRTIMP
#   define _CRTIMP
#  endif
# endif /* __declspec */
# ifndef __cdecl
#  define __cdecl __attribute__((cdecl))
# endif
# ifndef __stdcall
#  define __stdcall __attribute__((stdcall))
# endif
# ifndef __int64
#  define __int64 long long
# endif
# ifndef __int32
#  define __int32 long
# endif
# ifndef __int16
#  define __int16 int
# endif
# ifndef __int8
#  define __int8 char
# endif
# ifndef __small
#  define __small char
# endif
# ifndef __hyper
#  define __hyper long long
# endif
#endif /* __GNUC__ */

#ifdef __cplusplus
#define __CRT_INLINE inline
#else
#define __CRT_INLINE extern __inline__
#endif

#define __MINGW32_VERSION 3.2
#define __MINGW32_MAJOR_VERSION 3
#define __MINGW32_MINOR_VERSION 2

#endif /* __MINGW_H */
