/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Jiaxun Yang <jiaxun.yang@flygoat.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *	copyright notice, this list of conditions and the following
 *	disclaimer in the documentation and/or other materials provided
 *	with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *	contributors may be used to endorse or promote products derived
 *	from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MIPS_SEMIHOST_H_
#define _MIPS_SEMIHOST_H_

#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SYS_SEMIHOST_exit    1
#define SYS_SEMIHOST_open    2
#define SYS_SEMIHOST_close   3
#define SYS_SEMIHOST_read    4
#define SYS_SEMIHOST_write   5
#define SYS_SEMIHOST_lseek   6
#define SYS_SEMIHOST_unlink  7
#define SYS_SEMIHOST_fstat   8
#define SYS_SEMIHOST_argc    9
#define SYS_SEMIHOST_argv_sz 10
#define SYS_SEMIHOST_argv    11
#define SYS_SEMIHOST_plog    13
#define SYS_SEMIHOST_assert  14
#define SYS_SEMIHOST_pread   19
#define SYS_SEMIHOST_pwrite  20
#define SYS_SEMIHOST_link    22

enum {
    TARGET_ERRNO_EACCESS = 13,
    TARGET_ERRNO_EAGAIN = 11,
    TARGET_ERRNO_EBADF = 9,
    TARGET_ERRNO_EBADMSG = 77,
    TARGET_ERRNO_EBUSY = 16,
    TARGET_ERRNO_ECONNRESET = 104,
    TARGET_ERRNO_EEXIST = 17,
    TARGET_ERRNO_EFBIG = 27,
    TARGET_ERRNO_EINTR = 4,
    TARGET_ERRNO_EINVAL = 22,
    TARGET_ERRNO_EIO = 5,
    TARGET_ERRNO_EISDIR = 21,
    TARGET_ERRNO_ELOOP = 92,
    TARGET_ERRNO_EMFILE = 24,
    TARGET_ERRNO_EMLINK = 31,
    TARGET_ERRNO_ENAMETOOLONG = 91,
    TARGET_ERRNO_ENETDOWN = 115,
    TARGET_ERRNO_ENETUNREACH = 114,
    TARGET_ERRNO_ENFILE = 23,
    TARGET_ERRNO_ENOBUFS = 105,
    TARGET_ERRNO_ENOENT = 2,
    TARGET_ERRNO_ENOMEM = 12,
    TARGET_ERRNO_ENOSPC = 28,
    TARGET_ERRNO_ENOSR = 63,
    TARGET_ERRNO_ENOTCONN = 128,
    TARGET_ERRNO_ENOTDIR = 20,
    TARGET_ERRNO_ENXIO = 6,
    TARGET_ERRNO_EOVERFLOW = 139,
    TARGET_ERRNO_EPERM = 1,
    TARGET_ERRNO_EPIPE = 32,
    TARGET_ERRNO_ERANGE = 34,
    TARGET_ERRNO_EROFS = 30,
    TARGET_ERRNO_ESPIPE = 29,
    TARGET_ERRNO_ETIMEDOUT = 116,
    TARGET_ERRNO_ETXTBSY = 26,
    TARGET_ERRNO_EWOULDBLOCK = 11,
    TARGET_ERRNO_EXDEV = 18,
};

#define MIPS_O_RDONLY 0x0
#define MIPS_O_WRONLY 0x1
#define MIPS_O_RDWR   0x2
#define MIPS_O_APPEND 0x8
#define MIPS_O_CREAT  0x200
#define MIPS_O_TRUNC  0x400
#define MIPS_O_EXCL   0x800

struct mips_stat {
    int16_t  my_st_dev;
    uint16_t my_st_ino;
    uint32_t my_st_mode;
    uint16_t my_st_nlink;
    uint16_t my_st_uid;
    uint16_t my_st_gid;
    int16_t  my_st_rdev;
    uint64_t my_st_size;
    uint64_t my_st_atime;
    uint64_t my_st_spare1;
    uint64_t my_st_mtime;
    uint64_t my_st_spare2;
    uint64_t my_st_ctime;
    uint64_t my_st_spare3;
    uint64_t my_st_blksize;
    uint64_t my_st_blocks;
    uint64_t my_st_spare4[2];
};

static inline copy_stat(struct stat * restrict statbuf, struct mips_stat *mips_stat)
{
    statbuf->st_dev = mips_stat->my_st_dev;
    statbuf->st_ino = mips_stat->my_st_ino;
    statbuf->st_mode = mips_stat->my_st_mode;
    statbuf->st_nlink = mips_stat->my_st_nlink;
    statbuf->st_uid = mips_stat->my_st_uid;
    statbuf->st_gid = mips_stat->my_st_gid;
    statbuf->st_rdev = mips_stat->my_st_rdev;
    statbuf->st_size = mips_stat->my_st_size;
    statbuf->st_blksize = mips_stat->my_st_blksize;
    statbuf->st_blocks = mips_stat->my_st_blocks;
    statbuf->st_atime = mips_stat->my_st_atime;
    statbuf->st_mtime = mips_stat->my_st_mtime;
    statbuf->st_ctime = mips_stat->my_st_ctime;
}

static inline set_errno(long n)
{
    switch (n) {
    case TARGET_ERRNO_EACCESS:
        errno = EACCES;
        break;
    case TARGET_ERRNO_EAGAIN:
        errno = EAGAIN;
        break;
    case TARGET_ERRNO_EBADF:
        errno = EBADF;
        break;
    case TARGET_ERRNO_EBADMSG:
        errno = EBADMSG;
        break;
    case TARGET_ERRNO_EBUSY:
        errno = EBUSY;
        break;
    case TARGET_ERRNO_ECONNRESET:
        errno = ECONNRESET;
        break;
    case TARGET_ERRNO_EEXIST:
        errno = EEXIST;
        break;
    case TARGET_ERRNO_EFBIG:
        errno = EFBIG;
        break;
    case TARGET_ERRNO_EINTR:
        errno = EINTR;
        break;
    case TARGET_ERRNO_EINVAL:
        errno = EINVAL;
        break;
    case TARGET_ERRNO_EIO:
        errno = EIO;
        break;
    case TARGET_ERRNO_EISDIR:
        errno = EISDIR;
        break;
    case TARGET_ERRNO_ELOOP:
        errno = ELOOP;
        break;
    case TARGET_ERRNO_EMFILE:
        errno = EMFILE;
        break;
    case TARGET_ERRNO_EMLINK:
        errno = EMLINK;
        break;
    case TARGET_ERRNO_ENAMETOOLONG:
        errno = ENAMETOOLONG;
        break;
    case TARGET_ERRNO_ENETDOWN:
        errno = ENETDOWN;
        break;
    case TARGET_ERRNO_ENETUNREACH:
        errno = ENETUNREACH;
        break;
    case TARGET_ERRNO_ENFILE:
        errno = ENFILE;
        break;
    case TARGET_ERRNO_ENOBUFS:
        errno = ENOBUFS;
        break;
    case TARGET_ERRNO_ENOENT:
        errno = ENOENT;
        break;
    case TARGET_ERRNO_ENOMEM:
        errno = ENOMEM;
        break;
    case TARGET_ERRNO_ENOSPC:
        errno = ENOSPC;
        break;
    case TARGET_ERRNO_ENOSR:
        errno = ENOSR;
        break;
    case TARGET_ERRNO_ENOTCONN:
        errno = ENOTCONN;
        break;
    case TARGET_ERRNO_ENOTDIR:
        errno = ENOTDIR;
        break;
    case TARGET_ERRNO_ENXIO:
        errno = ENXIO;
        break;
    case TARGET_ERRNO_EOVERFLOW:
        errno = EOVERFLOW;
        break;
    case TARGET_ERRNO_EPERM:
        errno = EPERM;
        break;
    case TARGET_ERRNO_EPIPE:
        errno = EPIPE;
        break;
    case TARGET_ERRNO_ERANGE:
        errno = ERANGE;
        break;
    case TARGET_ERRNO_EROFS:
        errno = EROFS;
        break;
    case TARGET_ERRNO_ESPIPE:
        errno = ESPIPE;
        break;
    case TARGET_ERRNO_ETIMEDOUT:
        errno = ETIMEDOUT;
        break;
    case TARGET_ERRNO_ETXTBSY:
        errno = ETXTBSY;
        break;
    case TARGET_ERRNO_EXDEV:
        errno = EXDEV;
        break;
    default:
        errno = EINVAL;
        break;
    }
}

/* Assembler consider sdbbp R2+ */
#if __mips_isa_rev < 2
#define MIPS_ISA_LEVEL "mips32r2"
#else
#define MIPS_ISA_LEVEL "arch=default"
#endif

#define UHI_CALL_CODE     "1"
#define UHI_CALL_CODE_RAW 1

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
/* False positive for register variables */
#pragma GCC diagnostic ignored "-Wanalyzer-use-of-uninitialized-value"
#endif

static inline long
mips_semihost0(long n)
{
    register long r2 __asm__("$2");
    register long r25 __asm__("$25") = n;
    register long r3 __asm__("$3");

    __asm__ __volatile__(".set	push   \n"
                         ".set	" MIPS_ISA_LEVEL " \n"
                         "addu $2,$0,%2 \n"
                         "sdbbp " UHI_CALL_CODE "   \n"
                         ".set	pop	\n"
                         : "=&r"(r2), "=r"(r3)
                         : "ir"(UHI_CALL_CODE_RAW), "0"(r2), "r"(r25)
                         : "memory");

    if (r2 == -1)
        set_errno(r3);

    return r2;
}

static inline long
mips_semihost1(long n, long a)
{
    register long r2 __asm__("$2");
    register long r25 __asm__("$25") = n;
    register long r4 __asm__("$4") = a;
    register long r3 __asm__("$3");

    __asm__ __volatile__(".set	push   \n"
                         ".set	" MIPS_ISA_LEVEL " \n"
                         "addu $2,$0,%2 \n"
                         "sdbbp " UHI_CALL_CODE "   \n"
                         ".set	pop	\n"
                         : "=&r"(r2), "=r"(r3)
                         : "ir"(UHI_CALL_CODE_RAW), "0"(r2), "r"(r25), "r"(r4)
                         : "memory");

    if (r2 == -1)
        set_errno(r3);

    return r2;
}

static inline long
mips_semihost2(long n, long a, long b)
{
    register long r2 __asm__("$2");
    register long r25 __asm__("$25") = n;
    register long r4 __asm__("$4") = a;
    register long r5 __asm__("$5") = b;
    register long r3 __asm__("$3");

    __asm__ __volatile__(".set	push   \n"
                         ".set	" MIPS_ISA_LEVEL " \n"
                         "addu $2,$0,%2 \n"
                         "sdbbp " UHI_CALL_CODE "   \n"
                         ".set	pop	\n"
                         : "=&r"(r2), "=r"(r3)
                         : "ir"(UHI_CALL_CODE_RAW), "0"(r2), "r"(r25), "r"(r4), "r"(r5)
                         : "memory");

    if (r2 == -1)
        set_errno(r3);

    return r2;
}

static inline long
mips_semihost3(long n, long a, long b, long c)
{
    register long r2 __asm__("$2");
    register long r25 __asm__("$25") = n;
    register long r4 __asm__("$4") = a;
    register long r5 __asm__("$5") = b;
    register long r6 __asm__("$6") = c;
    register long r3 __asm__("$3");

    __asm__ __volatile__(".set	push   \n"
                         ".set	" MIPS_ISA_LEVEL " \n"
                         "addu $2,$0,%2 \n"
                         "sdbbp " UHI_CALL_CODE "   \n"
                         ".set	pop	\n"
                         : "=&r"(r2), "=r"(r3)
                         : "ir"(UHI_CALL_CODE_RAW), "0"(r2), "r"(r25), "r"(r4), "r"(r5), "r"(r6)
                         : "memory");

    if (r2 == -1)
        set_errno(r3);

    return r2;
}

static inline long
mips_semihost4(long n, long a, long b, long c, long d)
{
    register long r2 __asm__("$2");
    register long r25 __asm__("$25") = n;
    register long r4 __asm__("$4") = a;
    register long r5 __asm__("$5") = b;
    register long r6 __asm__("$6") = c;
    register long r7 __asm__("$7") = d;
    register long r3 __asm__("$3");

    __asm__ __volatile__(".set	push   \n"
                         ".set	" MIPS_ISA_LEVEL " \n"
                         "addu $2,$0,%2 \n"
                         "sdbbp " UHI_CALL_CODE "   \n"
                         ".set	pop	\n"
                         : "=&r"(r2), "=r"(r3)
                         : "ir"(UHI_CALL_CODE_RAW), "0"(r2), "r"(r25), "r"(r4), "r"(r5), "r"(r6),
                           "r"(r7)
                         : "memory");

    if (r2 == -1)
        set_errno(r3);

    return r2;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif /* _MIPS_SEMIHOST_H_ */
