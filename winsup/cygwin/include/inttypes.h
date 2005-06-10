/* inttypes.h - fixed size integer types

   Copyright 2003 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _INTTYPES_H
#define _INTTYPES_H

#include <stdint.h>

/* fprintf() macros for signed integers */

#define PRId8 "d"
#define PRId16 "d"
#define PRId32 "ld"
#define PRId64 "lld"

#define PRIdLEAST8 "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "ld"
#define PRIdLEAST64 "lld"

#define PRIdFAST8 "d"
#define PRIdFAST16 "ld"
#define PRIdFAST32 "ld"
#define PRIdFAST64 "lld"

#define PRIdMAX "lld"
#define PRIdPTR "ld"

#define PRIi8 "i"
#define PRIi16 "i"
#define PRIi32 "li"
#define PRIi64 "lli"

#define PRIiLEAST8 "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 "li"
#define PRIiLEAST64 "lli"

#define PRIiFAST8 "i"
#define PRIiFAST16 "li"
#define PRIiFAST32 "li"
#define PRIiFAST64 "lli"

#define PRIiMAX "lli"
#define PRIiPTR "li"

/* fprintf() macros for unsigned integers */

#define PRIo8 "o"
#define PRIo16 "o"
#define PRIo32 "lo"
#define PRIo64 "llo"

#define PRIoLEAST8 "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 "lo"
#define PRIoLEAST64 "llo"

#define PRIoFAST8 "o"
#define PRIoFAST16 "lo"
#define PRIoFAST32 "lo"
#define PRIoFAST64 "llo"

#define PRIoMAX "llo"
#define PRIoPTR "lo"

#define PRIu8 "u"
#define PRIu16 "u"
#define PRIu32 "lu"
#define PRIu64 "llu"

#define PRIuLEAST8 "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "lu"
#define PRIuLEAST64 "llu"

#define PRIuFAST8 "u"
#define PRIuFAST16 "lu"
#define PRIuFAST32 "lu"
#define PRIuFAST64 "llu"

#define PRIuMAX "llu"
#define PRIuPTR "lu"

#define PRIx8 "x"
#define PRIx16 "x"
#define PRIx32 "lx"
#define PRIx64 "llx"

#define PRIxLEAST8 "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 "lx"
#define PRIxLEAST64 "llx"

#define PRIxFAST8 "x"
#define PRIxFAST16 "lx"
#define PRIxFAST32 "lx"
#define PRIxFAST64 "llx"

#define PRIxMAX "llx"
#define PRIxPTR "lx"

#define PRIX8 "X"
#define PRIX16 "X"
#define PRIX32 "lX"
#define PRIX64 "llX"

#define PRIXLEAST8 "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 "lX"
#define PRIXLEAST64 "llX"

#define PRIXFAST8 "X"
#define PRIXFAST16 "lX"
#define PRIXFAST32 "lX"
#define PRIXFAST64 "llX"

#define PRIXMAX "llX"
#define PRIXPTR "lX"

/* fscanf() macros for signed integers */

#define SCNd8 "hhd"
#define SCNd16 "hd"
#define SCNd32 "ld"
#define SCNd64 "lld"

#define SCNdLEAST8 "hhd"
#define SCNdLEAST16 "hd"
#define SCNdLEAST32 "ld"
#define SCNdLEAST64 "lld"

#define SCNdFAST8 "hhd"
#define SCNdFAST16 "ld"
#define SCNdFAST32 "ld"
#define SCNdFAST64 "lld"

#define SCNdMAX "lld"
#define SCNdPTR "ld"

#define SCNi8 "hhi"
#define SCNi16 "hi"
#define SCNi32 "li"
#define SCNi64 "lli"

#define SCNiLEAST8 "hhi"
#define SCNiLEAST16 "hi"
#define SCNiLEAST32 "li"
#define SCNiLEAST64 "lli"

#define SCNiFAST8 "hhi"
#define SCNiFAST16 "li"
#define SCNiFAST32 "li"
#define SCNiFAST64 "lli"

#define SCNiMAX "lli"
#define SCNiPTR "li"

/* fscanf() macros for unsigned integers */

#define SCNo8 "hho"
#define SCNo16 "ho"
#define SCNo32 "lo"
#define SCNo64 "llo"

#define SCNoLEAST8 "hho"
#define SCNoLEAST16 "ho"
#define SCNoLEAST32 "lo"
#define SCNoLEAST64 "llo"

#define SCNoFAST8 "hho"
#define SCNoFAST16 "lo"
#define SCNoFAST32 "lo"
#define SCNoFAST64 "llo"

#define SCNoMAX "llo"
#define SCNoPTR "lo"

#define SCNu8 "hhu"
#define SCNu16 "hu"
#define SCNu32 "lu"
#define SCNu64 "llu"

#define SCNuLEAST8 "hhu"
#define SCNuLEAST16 "hu"
#define SCNuLEAST32 "lu"
#define SCNuLEAST64 "llu"

#define SCNuFAST8 "hhu"
#define SCNuFAST16 "lu"
#define SCNuFAST32 "lu"
#define SCNuFAST64 "llu"

#define SCNuMAX "llu"
#define SCNuPTR "lu"

#define SCNx8 "hhx"
#define SCNx16 "hx"
#define SCNx32 "lx"
#define SCNx64 "llx"

#define SCNxLEAST8 "hhx"
#define SCNxLEAST16 "hx"
#define SCNxLEAST32 "lx"
#define SCNxLEAST64 "llx"

#define SCNxFAST8 "hhx"
#define SCNxFAST16 "lx"
#define SCNxFAST32 "lx"
#define SCNxFAST64 "llx"

#define SCNxMAX "llx"
#define SCNxPTR "lx"

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

#if 0 /* Not yet defined */
intmax_t _EXFUN(wcstoimax, (const wchar_t *, wchar_t **, int));
uintmax_t _EXFUN(wcstoumax, (const wchar_t *, wchar_t **, int));
#endif

#ifdef __cplusplus
}
#endif

#endif /* _INTTYPES_H */
