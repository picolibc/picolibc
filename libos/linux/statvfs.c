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

#include "local-statfs.h"

/*
 * Convert a kernel struct statfs / statfs64 to a POSIX struct statvfs.
 *
 * The kernel statfs f_bsize is the preferred I/O block size (used as both
 * f_bsize and f_frsize in statvfs — Linux does not distinguish them in
 * statfs; both refer to the fundamental block size used for f_blocks et al).
 *
 * f_fsid is collapsed from a two-element int array to a single unsigned long
 * by combining the two halves.
 *
 * f_favail is set equal to f_ffree: Linux statfs has no separate
 * "available to non-privileged" inode count.
 */
int
_statvfsbuf(struct statvfs *buf, const struct __kernel_statfs *kbuf)
{
    buf->f_bsize = (unsigned long)kbuf->f_bsize;
    buf->f_frsize = (unsigned long)(kbuf->f_frsize ? kbuf->f_frsize : kbuf->f_bsize);
    buf->f_blocks = (fsblkcnt_t)kbuf->f_blocks;
    buf->f_bfree = (fsblkcnt_t)kbuf->f_bfree;
    buf->f_bavail = (fsblkcnt_t)kbuf->f_bavail;
    buf->f_files = (fsfilcnt_t)kbuf->f_files;
    buf->f_ffree = (fsfilcnt_t)kbuf->f_ffree;
    buf->f_favail = (fsfilcnt_t)kbuf->f_ffree;
    buf->f_fsid = (unsigned long)((unsigned long)kbuf->f_fsid[0]
                                  | ((unsigned long)(unsigned int)kbuf->f_fsid[1]
                                     << (sizeof(unsigned long) > 4 ? 32 : 0)));
    buf->f_flag = (unsigned long)kbuf->f_flags;
    buf->f_namemax = (unsigned long)kbuf->f_namelen;
    return 0;
}

int
statvfs(const char *path, struct statvfs *buf)
{
    struct __kernel_statfs kbuf;
    int                    ret;

#ifdef LINUX_SYS_statfs_CALL_NEEDS_SIZE
    ret = syscall(LINUX_SYS_statfs_CALL, path, sizeof(kbuf), &kbuf);
#else
    ret = syscall(LINUX_SYS_statfs_CALL, path, &kbuf);
#endif
    if (ret < 0)
        return ret;
    return _statvfsbuf(buf, &kbuf);
}

int
fstatvfs(int fd, struct statvfs *buf)
{
    struct __kernel_statfs kbuf;
    int                    ret;

#ifdef LINUX_SYS_statfs_CALL_NEEDS_SIZE
    ret = syscall(LINUX_SYS_fstatfs_CALL, fd, sizeof(kbuf), &kbuf);
#else
    ret = syscall(LINUX_SYS_fstatfs_CALL, fd, &kbuf);
#endif
    if (ret < 0)
        return ret;
    return _statvfsbuf(buf, &kbuf);
}
