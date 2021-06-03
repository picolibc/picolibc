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

#include <stdlib.h>
#include <string.h>
#include <semihost.h>

#define TEST_FILE_NAME	"SEMISEEK.TXT"
#define TEST_STRING	"hello, world"
#define TEST_STRING_LEN	12
#define TEST_SEEK_POS	2

int
main(void)
{
	int		fd;
	uintptr_t	not_written;
	int		code = 0;
	int		ret;
	uintptr_t	not_read;
	char		buf[TEST_STRING_LEN + 10];

	fd = sys_semihost_open(TEST_FILE_NAME, SH_OPEN_W);
	if (fd < 0) {
		printf("open %s failed\n", TEST_FILE_NAME);
		exit(1);
	}
	not_written = sys_semihost_write(fd, TEST_STRING, TEST_STRING_LEN);
	if (not_written != 0) {
		printf("write failed %ld %d\n", (long) not_written, sys_semihost_errno());
		code = 2;
		goto bail1;
	}
	ret = sys_semihost_close(fd);
	fd = -1;
	if (ret != 0) {
		printf("close failed %d %d\n", ret, sys_semihost_errno());
		code = 3;
		goto bail1;
	}

	fd = sys_semihost_open(TEST_FILE_NAME, SH_OPEN_R);
	if (fd < 0) {
		printf("open %s failed\n", TEST_FILE_NAME);
		code = 4;
		goto bail1;
	}

	ret = sys_semihost_seek(fd, TEST_SEEK_POS);
	if (ret != 0) {
		printf("seek failed %d %d\n", ret, sys_semihost_errno());
		code = 5;
		goto bail1;
	}

	not_read = sys_semihost_read(fd, buf, sizeof(buf));
	if (sizeof (buf) - not_read != TEST_STRING_LEN - TEST_SEEK_POS) {
		printf("read failed got %ld wanted %ld\n", (long) not_read,
		       (long) (sizeof(buf) - TEST_STRING_LEN + TEST_SEEK_POS));
		code = 6;
		goto bail1;
	}
	buf[TEST_STRING_LEN - TEST_SEEK_POS] = '\0';
	if (memcmp(buf, &TEST_STRING[0] + TEST_SEEK_POS, TEST_STRING_LEN - TEST_SEEK_POS) != 0) {
		printf("read bad contents got %s wanted %s\n",
		       buf, &TEST_STRING[0] + TEST_SEEK_POS);
		code = 7;
		goto bail1;
	}
bail1:
	if (fd >= 0)
		(void) sys_semihost_close(fd);
	(void) sys_semihost_remove(TEST_FILE_NAME);
	exit(code);
}
