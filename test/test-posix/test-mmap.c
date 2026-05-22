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

/*
 * test-mmap.c — tests for mmap, munmap, mprotect, msync, and posix_madvise.
 *
 * This test is only compiled when has_os_posix is true (i.e., the Linux OS
 * library is linked).  mmap is a kernel VM syscall with no meaningful
 * bare-metal or semihost equivalent, so no fake stub is used.
 *
 * Test structure:
 *   1. MAP_ANONYMOUS|MAP_PRIVATE  — anonymous private mapping, write/read
 *   2. mprotect                   — change protection on anonymous mapping
 *   3. posix_madvise              — hints on anonymous mapping
 *   4. Error conditions           — mmap(len=0), posix_madvise(bad advice)
 *   5. MAP_SHARED file-backed     — write through mapping, verify write-back
 *      (only when TESTS_ENABLE_POSIX_IO is defined)
 *   6. msync                      — MS_SYNC on MAP_SHARED mapping
 *      (only when TESTS_ENABLE_POSIX_IO is defined)
 */

#define _DEFAULT_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "MMAP.TXT"
#endif

/*
 * Standard check() macro: prints a message and exits 1 on failure.
 * Always removes the test file before exiting so subsequent runs are clean.
 */
#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            (void)remove(TEST_FILE_NAME);              \
            exit(1);                                   \
        }                                              \
    } while (0)

/*
 * check_mmap(): verify that an mmap() call succeeded.
 * If it failed with ENOMEM the system is resource-constrained; skip (77)
 * rather than fail so the test suite does not report a spurious failure.
 * Any other failure is a real error.
 */
#define check_mmap(ptr, message)                                                   \
    do {                                                                           \
        if ((ptr) == MAP_FAILED) {                                                 \
            if (errno == ENOMEM) {                                                 \
                printf("mmap: out of memory, skipping\n");                         \
                (void)remove(TEST_FILE_NAME);                                      \
                exit(77);                                                          \
            }                                                                      \
            printf("%s: mmap returned MAP_FAILED (errno %d)\n", (message), errno); \
            (void)remove(TEST_FILE_NAME);                                          \
            exit(1);                                                               \
        }                                                                          \
    } while (0)

/* Use a fixed page size; sysconf(_SC_PAGESIZE) is not always available. */
#define PAGE_SIZE ((size_t)4096)

int
main(void)
{
    void *addr;
    int   ret;

    /* ------------------------------------------------------------------
     * Test 1: MAP_ANONYMOUS | MAP_PRIVATE
     * The most fundamental mmap use-case: an anonymous private mapping
     * backed by zero pages.  No file descriptor required.
     * ------------------------------------------------------------------ */
    addr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    check_mmap(addr, "anonymous mmap");

    /* Write a pattern and read it back through the same pointer. */
    memset(addr, 0xAB, PAGE_SIZE);
    check(*(unsigned char *)addr == 0xAB, "anonymous mmap: first byte should be 0xAB");
    check(((unsigned char *)addr)[PAGE_SIZE - 1] == 0xAB,
          "anonymous mmap: last byte should be 0xAB");

    ret = munmap(addr, PAGE_SIZE);
    check(ret == 0, "munmap anonymous mapping");

    /* ------------------------------------------------------------------
     * Test 2: mprotect
     * Change protection on an anonymous mapping from RW → R → RW.
     * ------------------------------------------------------------------ */
    addr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    check_mmap(addr, "mprotect test: mmap");

    ret = mprotect(addr, PAGE_SIZE, PROT_READ);
    check(ret == 0, "mprotect PROT_READ");

    ret = mprotect(addr, PAGE_SIZE, PROT_READ | PROT_WRITE);
    check(ret == 0, "mprotect PROT_READ|PROT_WRITE");

    ret = munmap(addr, PAGE_SIZE);
    check(ret == 0, "munmap after mprotect");

    /* ------------------------------------------------------------------
     * Test 3: posix_madvise
     * Provide hints on an anonymous mapping.  posix_madvise() returns an
     * error code directly (not -1/errno), per POSIX.
     * ------------------------------------------------------------------ */
    addr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    check_mmap(addr, "posix_madvise test: mmap");

    ret = posix_madvise(addr, PAGE_SIZE, POSIX_MADV_SEQUENTIAL);
    check(ret == 0, "posix_madvise POSIX_MADV_SEQUENTIAL");

    ret = posix_madvise(addr, PAGE_SIZE, POSIX_MADV_WILLNEED);
    check(ret == 0, "posix_madvise POSIX_MADV_WILLNEED");

    ret = posix_madvise(addr, PAGE_SIZE, POSIX_MADV_DONTNEED);
    check(ret == 0, "posix_madvise POSIX_MADV_DONTNEED");

    ret = munmap(addr, PAGE_SIZE);
    check(ret == 0, "munmap after posix_madvise");

    /* ------------------------------------------------------------------
     * Test 4: Error conditions
     * ------------------------------------------------------------------ */

    /* mmap with length 0 must fail with EINVAL (POSIX). */
    errno = 0;
    addr = mmap(NULL, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    check(addr == MAP_FAILED, "mmap(len=0) should return MAP_FAILED");
    check(errno == EINVAL, "mmap(len=0) should set errno to EINVAL");

    /* posix_madvise with an unrecognised advice value must return EINVAL
     * directly (not set errno), per POSIX. */
    addr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    check_mmap(addr, "posix_madvise error test: mmap");

    ret = posix_madvise(addr, PAGE_SIZE, 9999);
    check(ret == EINVAL, "posix_madvise(bad advice) should return EINVAL");

    ret = munmap(addr, PAGE_SIZE);
    check(ret == 0, "munmap after posix_madvise error test");

#ifdef TESTS_ENABLE_POSIX_IO
    /* ------------------------------------------------------------------
     * Test 5: MAP_SHARED file-backed mapping
     * Write through the mapping and verify the modification is visible
     * after munmap/close by re-reading the file.
     * ------------------------------------------------------------------ */
    {
        int  fd;
        char buf[8];

        (void)remove(TEST_FILE_NAME);

        fd = open(TEST_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
        check(fd >= 0, "MAP_SHARED test: open");
        check(write(fd, "abcdefgh", 8) == 8, "MAP_SHARED test: write");

        addr = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        check_mmap(addr, "MAP_SHARED test: mmap");

        /* Modify one byte through the mapping. */
        ((char *)addr)[3] = 'j';

        ret = munmap(addr, 8);
        check(ret == 0, "MAP_SHARED test: munmap");

        ret = close(fd);
        check(ret == 0, "MAP_SHARED test: close");

        /* Re-open and verify the write was flushed to the file. */
        fd = open(TEST_FILE_NAME, O_RDONLY);
        check(fd >= 0, "MAP_SHARED test: re-open");
        check(read(fd, buf, 8) == 8, "MAP_SHARED test: re-read");
        close(fd);

        check(buf[3] == 'j', "MAP_SHARED test: modified byte should be 'j'");
        check(strncmp(buf, "abcjefgh", 8) == 0, "MAP_SHARED test: file contents after munmap");
    }

    /* ------------------------------------------------------------------
     * Test 6: msync on a MAP_SHARED mapping
     * Verify that msync(MS_SYNC) succeeds and the data is visible after
     * munmap.
     * ------------------------------------------------------------------ */
    {
        int  fd;
        char buf[8];

        (void)remove(TEST_FILE_NAME);

        fd = open(TEST_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
        check(fd >= 0, "msync test: open");
        check(write(fd, "ABCDEFGH", 8) == 8, "msync test: write");

        addr = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        check_mmap(addr, "msync test: mmap");

        ((char *)addr)[0] = 'Z';

        ret = msync(addr, 8, MS_SYNC);
        check(ret == 0, "msync MS_SYNC");

        ret = munmap(addr, 8);
        check(ret == 0, "msync test: munmap");

        ret = close(fd);
        check(ret == 0, "msync test: close");

        fd = open(TEST_FILE_NAME, O_RDONLY);
        check(fd >= 0, "msync test: re-open");
        check(read(fd, buf, 8) == 8, "msync test: re-read");
        close(fd);

        check(buf[0] == 'Z', "msync test: first byte should be 'Z' after msync");
    }

    (void)remove(TEST_FILE_NAME);
#endif /* TESTS_ENABLE_POSIX_IO */

    printf("mmap test passed\n");
    exit(0);
}
