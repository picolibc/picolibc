/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "msp430-semihost.h"

ssize_t
read(int fd, void *buf, size_t count)
{
        (void) fd;
        (void) buf;
        (void) count;
	return 0;
}

ssize_t
write(int fd, const void *buf, size_t count)
{
	const char *b = buf;
	size_t c = count;

        (void) fd;
	while (c--)
                msp430_putc(*b++, NULL);
	return count;
}

int
open(const char *pathname, int flags, ...)
{
        (void) pathname;
        (void) flags;
	return -1;
}

int
close(int fd)
{
        (void) fd;
	return 0;
}

off_t lseek(int fd, off_t offset, int whence)
{
        (void) fd;
        (void) offset;
        (void) whence;
	return (off_t) -1;
}

_off64_t lseek64(int fd, _off64_t offset, int whence)
{
	return (_off64_t) lseek(fd, (off_t) offset, whence);
}

int
unlink(const char *pathname)
{
        (void) pathname;
	return 0;
}

int
fstat (int fd, struct stat *sbuf)
{
        (void) fd;
        (void) sbuf;
	return -1;
}

int
isatty (int fd)
{
        (void) fd;
	return 1;
}
