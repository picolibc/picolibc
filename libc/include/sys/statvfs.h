/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _SYS_STATVFS_H
#define _SYS_STATVFS_H

#include <sys/cdefs.h>
#include <sys/_types.h>

_BEGIN_STD_C

#ifndef _FSBLKCNT_T_DECLARED /* for statvfs() */
typedef __fsblkcnt_t fsblkcnt_t;
typedef __fsfilcnt_t fsfilcnt_t;
#define _FSBLKCNT_T_DECLARED
#endif

struct statvfs {
    unsigned long f_bsize;   /* file system block size */
    unsigned long f_frsize;  /* fundamental file system block size */
    fsblkcnt_t    f_blocks;  /* total number of blocks in file system */
    fsblkcnt_t    f_bfree;   /* total number of free blocks */
    fsblkcnt_t    f_bavail;  /* number of free blocks available to non-privileged process */
    fsfilcnt_t    f_files;   /* total number of file serial numbers */
    fsfilcnt_t    f_ffree;   /* total number of free file serial numbers */
    fsfilcnt_t    f_favail;  /* number of file serial numbers available to non-privileged process */
    unsigned long f_fsid;    /* file system ID */
    unsigned long f_flag;    /* bit mask of f_flag values */
    unsigned long f_namemax; /* maximum filename length */
};

/* f_flag bit values */
#define ST_RDONLY 0x0001 /* read-only file system */
#define ST_NOSUID 0x0002 /* does not support setuid/setgid semantics */

#if __POSIX_VISIBLE >= 200112
int statvfs(const char * __restrict __path, struct statvfs * __restrict __buf);
int fstatvfs(int __fd, struct statvfs *__buf);
#endif

_END_STD_C

#endif /* _SYS_STATVFS_H */
