/* Copyright (c) 2016 Phoenix Systems
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.*/

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <machine/types.h>
#include <machine/_types.h>
#include <stdint.h>
#include <sys/_types.h>
#include <sys/_null.h>

#include <phoenix/types.h>

typedef __uint32_t		__ino_t ;
typedef __uint64_t		__ino64_t;
typedef __uint64_t		__off64_t;
typedef __uint32_t		__key_t;
typedef __uint32_t		__useconds_t;
typedef __uint32_t		__daddr_t;
typedef char *			caddr_t;
typedef __uint32_t		__nlink_t;
typedef __uint8_t		__u_char;
typedef unsigned short	__u_short;
typedef unsigned		__u_int;
typedef unsigned long	__u_long;

typedef _CLOCK_T_		clock_t;
typedef _ssize_t		ssize_t;
typedef __ino_t			ino_t;
typedef __ino64_t		ino64_t;
typedef __off64_t		off64_t;
typedef __off_t			off_t;
typedef __loff_t		loff_t;
typedef __key_t			key_t;
typedef __suseconds_t	suseconds_t;
typedef __useconds_t	useconds_t;
typedef __daddr_t		daddr_t;
typedef char *			caddr_t;
typedef __nlink_t		nlink_t;
typedef pid_t			id_t;
typedef __u_char		u_char;
typedef __u_short		u_short;
typedef __u_int			u_int;
typedef __u_long		u_long;
typedef __uint8_t		u_int8_t;
typedef __uint16_t		u_int16_t;
typedef __uint32_t		u_int32_t;
typedef __uint64_t		u_int64_t;
typedef __uint8_t		uint8_t;
typedef __uint16_t		uint16_t;
typedef __uint32_t		uint32_t;
typedef __uint64_t		uint64_t;
typedef __int8_t		int8_t;
typedef __int16_t		int16_t;
typedef __int32_t		int32_t;
typedef __int64_t		int64_t;

typedef	__uint32_t		__blksize_t;
typedef	__int64_t		__blkcnt_t;
typedef __int64_t		sbintime_t;
typedef	__blkcnt_t		blkcnt_t;

#define __time_t_defined
#define __gid_t_defined
#define __uid_t_defined
#define __ssize_t_defined
#define __key_t_defined
#define __off_t_defined
#define __off64_t_defined

static inline unsigned int dev_major (unsigned long int dev)
{
	return ((dev >> 16) & 0xffff);
}

static inline unsigned int dev_minor (unsigned long int dev)
{
	return (dev & 0xffff);
}

static inline unsigned long int dev_makedev (unsigned int major, unsigned int minor)
{
	return (minor & 0xffff) | ((major & 0xffff) << 16);
}

/* Access the functions with their traditional names. */
# define major(dev)			dev_major(dev)
# define minor(dev)			dev_minor(dev)
# define makedev(maj, min)	dev_makedev(maj, min)

#endif
