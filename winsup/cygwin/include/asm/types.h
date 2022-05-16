/* asm/types.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _ASM_TYPES_H
#define _ASM_TYPES_H

#include <asm/bitsperlong.h>

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

/* As on Linux. */
typedef __signed__ long long __s64;
typedef unsigned long long __u64;

#endif /* _ASM_TYPES_H */
