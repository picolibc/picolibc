/* inttypes.h - fixed size integer types

   Copyright 2003, 2005, 2009, 2010, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _INTTYPES_H
#define _INTTYPES_H

#include <stdint.h>
#define __need_wchar_t
#include <stddef.h>
#include <bits/wordsize.h>

/* C99 requires that in C++ the following macros should be defined only
   if requested. */
#if !defined (__cplusplus) || defined (__STDC_FORMAT_MACROS) \
    || defined (__INSIDE_CYGWIN__)

#if __WORDSIZE == 64
#define __PRI64 "l"
#define __PRIFAST "l"
#define __PRIPTR "l"
#else
#define __PRI64 "ll"
#define __PRIFAST
#define __PRIPTR
#endif

/* fprintf() macros for signed integers */

#define PRId8 "d"
#define PRId16 "d"
#define PRId32 "d"
#define PRId64 __PRI64 "d"

#define PRIdLEAST8 "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 __PRI64 "d"

#define PRIdFAST8 "d"
#define PRIdFAST16 __PRIFAST "d"
#define PRIdFAST32 __PRIFAST "d"
#define PRIdFAST64 __PRI64 "d"

#define PRIdMAX __PRI64 "d"
#define PRIdPTR __PRIPTR "d"

#define PRIi8 "i"
#define PRIi16 "i"
#define PRIi32 "i"
#define PRIi64 __PRI64 "i"

#define PRIiLEAST8 "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 __PRI64 "i"

#define PRIiFAST8 "i"
#define PRIiFAST16 __PRIFAST "i"
#define PRIiFAST32 __PRIFAST "i"
#define PRIiFAST64 __PRI64 "i"

#define PRIiMAX __PRI64 "i"
#define PRIiPTR __PRIPTR "i"

/* fprintf() macros for unsigned integers */

#define PRIo8 "o"
#define PRIo16 "o"
#define PRIo32 "o"
#define PRIo64 __PRI64 "o"

#define PRIoLEAST8 "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 __PRI64 "o"

#define PRIoFAST8 "o"
#define PRIoFAST16 __PRIFAST "o"
#define PRIoFAST32 __PRIFAST "o"
#define PRIoFAST64 __PRI64 "o"

#define PRIoMAX __PRI64 "o"
#define PRIoPTR __PRIPTR "o"

#define PRIu8 "u"
#define PRIu16 "u"
#define PRIu32 "u"
#define PRIu64 __PRI64 "u"

#define PRIuLEAST8 "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 __PRI64 "u"

#define PRIuFAST8 "u"
#define PRIuFAST16 __PRIFAST "u"
#define PRIuFAST32 __PRIFAST "u"
#define PRIuFAST64 __PRI64 "u"

#define PRIuMAX __PRI64 "u"
#define PRIuPTR __PRIPTR "u"

#define PRIx8 "x"
#define PRIx16 "x"
#define PRIx32 "x"
#define PRIx64 __PRI64 "x"

#define PRIxLEAST8 "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 __PRI64 "x"

#define PRIxFAST8 "x"
#define PRIxFAST16 __PRIFAST "x"
#define PRIxFAST32 __PRIFAST "x"
#define PRIxFAST64 __PRI64 "x"

#define PRIxMAX __PRI64 "x"
#define PRIxPTR __PRIPTR "x"

#define PRIX8 "X"
#define PRIX16 "X"
#define PRIX32 "X"
#define PRIX64 __PRI64 "X"

#define PRIXLEAST8 "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 __PRI64 "X"

#define PRIXFAST8 "X"
#define PRIXFAST16 __PRIFAST "X"
#define PRIXFAST32 __PRIFAST "X"
#define PRIXFAST64 __PRI64 "X"

#define PRIXMAX __PRI64 "X"
#define PRIXPTR __PRIPTR "X"

/* fscanf() macros for signed integers */

#if __WORDSIZE == 64
#define __SCN64 "l"
#define __SCNFAST "l"
#define __SCNPTR "l"
#else
#define __SCN64 "ll"
#define __SCNFAST
#define __SCNPTR
#endif

#define SCNd8 "hhd"
#define SCNd16 "hd"
#define SCNd32 "d"
#define SCNd64 __SCN64 "d"

#define SCNdLEAST8 "hhd"
#define SCNdLEAST16 "hd"
#define SCNdLEAST32 "d"
#define SCNdLEAST64 __SCN64 "d"

#define SCNdFAST8 "hhd"
#define SCNdFAST16 __SCNFAST "d"
#define SCNdFAST32 __SCNFAST "d"
#define SCNdFAST64 __SCN64 "d"

#define SCNdMAX __SCN64 "d"
#define SCNdPTR __SCNPTR "d"

#define SCNi8 "hhi"
#define SCNi16 "hi"
#define SCNi32 "i"
#define SCNi64 __SCN64 "i"

#define SCNiLEAST8 "hhi"
#define SCNiLEAST16 "hi"
#define SCNiLEAST32 "i"
#define SCNiLEAST64 __SCN64 "i"

#define SCNiFAST8 "hhi"
#define SCNiFAST16 __SCNFAST "i"
#define SCNiFAST32 __SCNFAST "i"
#define SCNiFAST64 __SCN64 "i"

#define SCNiMAX __SCN64 "i"
#define SCNiPTR __SCNPTR "i"

/* fscanf() macros for unsigned integers */

#define SCNo8 "hho"
#define SCNo16 "ho"
#define SCNo32 "o"
#define SCNo64 __SCN64 "o"

#define SCNoLEAST8 "hho"
#define SCNoLEAST16 "ho"
#define SCNoLEAST32 "o"
#define SCNoLEAST64 __SCN64 "o"

#define SCNoFAST8 "hho"
#define SCNoFAST16 __SCNFAST "o"
#define SCNoFAST32 __SCNFAST "o"
#define SCNoFAST64 __SCN64 "o"

#define SCNoMAX __SCN64 "o"
#define SCNoPTR __SCNPTR "o"

#define SCNu8 "hhu"
#define SCNu16 "hu"
#define SCNu32 "u"
#define SCNu64 __SCN64 "u"

#define SCNuLEAST8 "hhu"
#define SCNuLEAST16 "hu"
#define SCNuLEAST32 "u"
#define SCNuLEAST64 __SCN64 "u"

#define SCNuFAST8 "hhu"
#define SCNuFAST16 __SCNFAST "u"
#define SCNuFAST32 __SCNFAST "u"
#define SCNuFAST64 __SCN64 "u"

#define SCNuMAX __SCN64 "u"
#define SCNuPTR __SCNPTR "u"

#define SCNx8 "hhx"
#define SCNx16 "hx"
#define SCNx32 "x"
#define SCNx64 __SCN64 "x"

#define SCNxLEAST8 "hhx"
#define SCNxLEAST16 "hx"
#define SCNxLEAST32 "x"
#define SCNxLEAST64 __SCN64 "x"

#define SCNxFAST8 "hhx"
#define SCNxFAST16 __SCNFAST "x"
#define SCNxFAST32 __SCNFAST "x"
#define SCNxFAST64 __SCN64 "x"

#define SCNxMAX __SCN64 "x"
#define SCNxPTR __SCNPTR "x"

#endif /* !__cplusplus || __STDC_FORMAT_MACROS || __INSIDE_CYGWIN__ */

#ifdef __cplusplus
extern "C" {
#endif

#include <_ansi.h>

typedef struct {
  intmax_t quot;
  intmax_t rem;
} imaxdiv_t;

intmax_t _EXFUN(imaxabs, (intmax_t));
imaxdiv_t _EXFUN(imaxdiv, (intmax_t, intmax_t));
intmax_t _EXFUN(strtoimax, (const char *, char **, int));
uintmax_t _EXFUN(strtoumax, (const char *, char **, int));
intmax_t _EXFUN(wcstoimax, (const wchar_t *, wchar_t **, int));
uintmax_t _EXFUN(wcstoumax, (const wchar_t *, wchar_t **, int));

#ifdef __cplusplus
}
#endif

#endif /* _INTTYPES_H */
