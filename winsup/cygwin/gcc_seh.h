/* gcc_seh.h: GCC exception codes.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

/* From the GCC source file libgcc/unwind-seh.c. */

#define STATUS_USER_DEFINED		(1U << 29)
#define GCC_MAGIC			(('G' << 16) | ('C' << 8) | 'C')
#define GCC_EXCEPTION(TYPE)		\
       (STATUS_USER_DEFINED | ((TYPE) << 24) | GCC_MAGIC)
#define STATUS_GCC_THROW		GCC_EXCEPTION (0)
#define STATUS_GCC_UNWIND		GCC_EXCEPTION (1)
#define STATUS_GCC_FORCED		GCC_EXCEPTION (2)
