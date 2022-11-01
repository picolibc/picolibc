/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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

#include "stdio_private.h"
#include <stdlib.h>
#include <unistd.h>

static char write_buf[BUFSIZ];
static char read_buf[BUFSIZ];

static struct __file_bufio __stdin = FDEV_SETUP_POSIX(0, read_buf, BUFSIZ, __SRD, 0);
static struct __file_bufio __stdout = FDEV_SETUP_POSIX(1, write_buf, BUFSIZ, __SWR, __BLBF);

FILE *const __posix_stdin = &__stdin.xfile.cfile.file;
FILE *const __posix_stdout = &__stdout.xfile.cfile.file;

#ifdef __strong_reference
__strong_reference(__posix_stdout, __posix_stderr);
#else
FILE *const __posix_stderr = &__stdout.xfile.cfile.file;
#endif

__weak_reference(__posix_stdin,stdin);
__weak_reference(__posix_stdout,stdout);
__weak_reference(__posix_stderr,stderr);

__attribute__((constructor))
static void posix_init(void)
{
    __bufio_lock_init(&__stdin.xfile.cfile.file);
    __bufio_lock_init(&__stdout.xfile.cfile.file);
}

/*
 * Add a destructor function to get stdout flushed on
 * exit
 */
__attribute__((destructor (101)))
static void posix_exit(void)
{
	fflush(stdout);
}
