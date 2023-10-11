/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
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

#include "semihost-private.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct timeval __semihost_write_time, __semihost_creat_time;

int
fstat (int fd, struct stat *sbuf )
{
        fd = _map_stdio(fd);

	int size;

        if (fd > 0)
                size = sys_semihost_flen(fd);
        else
                size = -1;

	if (size > 0) {
		sbuf->st_size = size;
		sbuf->st_blksize = 4096;
		sbuf->st_mode = S_IFREG;
	} else {
		sbuf->st_size = 0;
		sbuf->st_blksize = 0;
		sbuf->st_mode = S_IFCHR;
	}
        sbuf->st_ctime = __semihost_creat_time.tv_sec > __semihost_write_time.tv_sec ?
            __semihost_creat_time.tv_sec : __semihost_write_time.tv_sec;
        sbuf->st_mtime = __semihost_write_time.tv_sec;
	return 0;
}
