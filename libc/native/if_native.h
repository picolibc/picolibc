/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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

#ifndef _IF_NATIVE_H_
#define _IF_NATIVE_H_

/*
 * Construct an interface between the native C library and
 * picolibc. Declare new structs using only GCC types that can pass
 * information between the two libaries.
 */

struct if_timespec {
    __uint64_t tv_sec;
    __uint64_t tv_nsec;
};

struct if_stat {
    /* The next two fields contain the ID of the device
       containing the filesystem where the file resides */
    __uint32_t         st_dev_major; /* Major ID */
    __uint32_t         st_dev_minor; /* Minor ID */

    __uint64_t         st_ino; /* Inode number */

    __uint16_t         st_mode; /* File type and mode */

    __uint32_t         st_nlink; /* Number of hard links */

    __uint32_t         st_uid; /* User ID of owner */
    __uint32_t         st_gid; /* Group ID of owner */

    /* If this file represents a device, then the next two
       fields contain the ID of the device */
    __uint32_t         st_rdev_major; /* Major ID */
    __uint32_t         st_rdev_minor; /* Minor ID */

    __uint64_t         st_size; /* Total size in bytes */

    __uint32_t         st_blksize; /* Block size for filesystem I/O */
    __uint64_t         st_blocks;  /* Number of 512B blocks allocated */

    struct if_timespec st_atim; /* Last access */
    struct if_timespec st_mtim; /* Last modification */
    struct if_timespec st_ctim; /* Last status change */
};

int __fstat_host(int fd, struct if_stat *if_stat);
int __lstat_host(const char *path, struct if_stat *if_stat);
int __stat_host(const char *path, struct if_stat *if_stat);

#define TIME_FROM_HOST(ti, th)       \
    do {                             \
        (ti).tv_sec = (th).tv_sec;   \
        (ti).tv_nsec = (th).tv_nsec; \
    } while (0)

#define TIME_FROM_IF(tp, ti)         \
    do {                             \
        (tp).tv_sec = (ti).tv_sec;   \
        (tp).tv_nsec = (ti).tv_nsec; \
    } while (0)

#define STAT_FROM_HOST(i, h)                          \
    do {                                              \
        (i)->st_dev_major = (h)->stx_dev_major;       \
        (i)->st_dev_minor = (h)->stx_dev_minor;       \
        (i)->st_ino = (h)->stx_ino;                   \
        (i)->st_mode = (h)->stx_mode;                 \
        (i)->st_nlink = (h)->stx_nlink;               \
        (i)->st_uid = (h)->stx_uid;                   \
        (i)->st_gid = (h)->stx_uid;                   \
        (i)->st_rdev_major = (h)->stx_rdev_major;     \
        (i)->st_rdev_minor = (h)->stx_rdev_minor;     \
        (i)->st_size = (h)->stx_size;                 \
        (i)->st_blksize = (h)->stx_blksize;           \
        (i)->st_blocks = (h)->stx_blocks;             \
        TIME_FROM_HOST((i)->st_atim, (h)->stx_atime); \
        TIME_FROM_HOST((i)->st_mtim, (h)->stx_mtime); \
        TIME_FROM_HOST((i)->st_ctim, (h)->stx_ctime); \
    } while (0)

#define STAT_FROM_IF(p, i)                                                 \
    do {                                                                   \
        (p)->st_dev = __make_dev((i)->st_dev_major, (i)->st_dev_minor);    \
        (p)->st_ino = (i)->st_ino;                                         \
        (p)->st_mode = (i)->st_mode;                                       \
        (p)->st_nlink = (i)->st_nlink;                                     \
        (p)->st_uid = (i)->st_uid;                                         \
        (p)->st_gid = (i)->st_gid;                                         \
        (p)->st_rdev = __make_dev((i)->st_rdev_major, (i)->st_rdev_minor); \
        (p)->st_size = (i)->st_size;                                       \
        (p)->st_blksize = (i)->st_blksize;                                 \
        (p)->st_blocks = (i)->st_blocks;                                   \
        TIME_FROM_IF((p)->st_atim, (i)->st_atim);                          \
        TIME_FROM_IF((p)->st_mtim, (i)->st_mtim);                          \
        TIME_FROM_IF((p)->st_ctim, (i)->st_ctim);                          \
    } while (0)

#endif /* _IF_NATIVE_H_ */
