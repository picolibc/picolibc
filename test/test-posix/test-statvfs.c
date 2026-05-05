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

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "STATVFS.TXT"
#endif

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            exit(1);                                 \
        }                                            \
    } while (0)

/*
 * Detect fake (ENOSYS) implementations via weak references to the
 * __fake_statvfs / __fake_fstatvfs symbols exported by the semihost
 * fake layer. When the real OS-backed implementation is linked (e.g.
 * liblinux), these weak references remain NULL and not_fake() is true.
 */
#ifdef __PICOLIBC__
__typeof(statvfs)  __fake_statvfs __weak;
__typeof(fstatvfs) __fake_fstatvfs __weak;
#define not_fake(sym) ((sym) != __fake_##sym)
#else
#define not_fake(sym) 1
#endif

int
main(void)
{
#ifndef TESTS_ENABLE_POSIX_IO
    printf("POSIX I/O not enabled, skipping\n");
    return 77;
#endif

    /* Skip gracefully when the implementation is a fake ENOSYS stub */
    if (!not_fake(statvfs)) {
        printf("fake statvfs, skipping\n");
        return 77;
    }

    struct statvfs sv;
    int            ret;

    /* --- statvfs() on the current directory --- */
    memset(&sv, 0, sizeof(sv));
    ret = statvfs(".", &sv);
    check(ret == 0, "statvfs(\".\")");

    /* Basic sanity: block size must be a power of two >= 512 */
    check(sv.f_bsize >= 512, "f_bsize >= 512");
    check((sv.f_bsize & (sv.f_bsize - 1)) == 0, "f_bsize is power of two");

    /* Fragment size must be non-zero and <= block size */
    check(sv.f_frsize > 0, "f_frsize > 0");
    check(sv.f_frsize <= sv.f_bsize, "f_frsize <= f_bsize");

    /* Total blocks must be non-zero */
    check(sv.f_blocks > 0, "f_blocks > 0");

    /* Free and available blocks must be <= total */
    check(sv.f_bfree <= sv.f_blocks, "f_bfree <= f_blocks");
    check(sv.f_bavail <= sv.f_blocks, "f_bavail <= f_blocks");

    /* File serial number counts: total must be non-zero */
    check(sv.f_files > 0, "f_files > 0");
    check(sv.f_ffree <= sv.f_files, "f_ffree <= f_files");
    check(sv.f_favail <= sv.f_files, "f_favail <= f_files");

    /* Maximum filename length must be reasonable */
    check(sv.f_namemax >= 8, "f_namemax >= 8");

    printf("statvfs(\".\"):\n");
    printf("  f_bsize   = %lu\n", (unsigned long)sv.f_bsize);
    printf("  f_frsize  = %lu\n", (unsigned long)sv.f_frsize);
    printf("  f_blocks  = %llu\n", (unsigned long long)sv.f_blocks);
    printf("  f_bfree   = %llu\n", (unsigned long long)sv.f_bfree);
    printf("  f_bavail  = %llu\n", (unsigned long long)sv.f_bavail);
    printf("  f_files   = %llu\n", (unsigned long long)sv.f_files);
    printf("  f_ffree   = %llu\n", (unsigned long long)sv.f_ffree);
    printf("  f_namemax = %lu\n", (unsigned long)sv.f_namemax);

    /* --- fstatvfs() on an open file descriptor --- */
    if (!not_fake(fstatvfs)) {
        printf("fake fstatvfs, skipping fstatvfs test\n");
        exit(0);
    }

    /* Create a temporary file to get a valid fd */
    FILE *f = fopen(TEST_FILE_NAME, "w");
    check(f != NULL, "fopen for fstatvfs test");
    fputs("statvfs test\n", f);
    fclose(f);

    int fd = open(TEST_FILE_NAME, O_RDONLY);
    check(fd >= 0, "open for fstatvfs");

    struct statvfs sv2;
    memset(&sv2, 0, sizeof(sv2));
    ret = fstatvfs(fd, &sv2);
    close(fd);
    (void)remove(TEST_FILE_NAME);

    check(ret == 0, "fstatvfs(fd)");
    check(sv2.f_bsize == sv.f_bsize, "fstatvfs f_bsize matches statvfs");
    check(sv2.f_frsize == sv.f_frsize, "fstatvfs f_frsize matches statvfs");
    check(sv2.f_blocks == sv.f_blocks, "fstatvfs f_blocks matches statvfs");

    printf("fstatvfs(fd): OK\n");

    exit(0);
}
