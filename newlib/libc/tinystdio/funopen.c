/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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

#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio-bufio.h>

FILE *
funopen (const void *cookie,
         ssize_t (*readfn)(void *cookie, void *buf, size_t n),
         ssize_t (*writefn)(void *cookie, const void *buf, size_t n),
         __off_t (*seekfn)(void *cookie, __off_t off, int whence),
         int (*closefn)(void *cookie))
{
	struct __file_bufio *bf;
        char *buf;
        int open_flags = 0;

        if (readfn)
            open_flags |= __SRD;
        if (writefn)
            open_flags |= __SWR;

	/* Allocate file structure and necessary buffers */
	bf = calloc(1, sizeof(struct __file_bufio) + BUFSIZ);

	if (bf == NULL)
            return NULL;

        buf = (char *) (bf + 1);

        *bf = (struct __file_bufio)
            FDEV_SETUP_BUFIO_PTR(cookie, buf, BUFSIZ, readfn, writefn, seekfn, closefn, open_flags, __BFALL);

	return (FILE *) bf;
}
