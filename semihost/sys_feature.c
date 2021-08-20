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

#include "semihost-private.h"
#include <stdbool.h>
#include <string.h>

#define SH_EXT_NUM_BYTES	((SH_EXT_NUM + 7) >> 3)

static bool got_feature_bytes;
static uint8_t feature_bytes[SH_EXT_NUM_BYTES];

static const uint8_t fb_magic[4] = {
	SHFB_MAGIC_0,
	SHFB_MAGIC_1,
	SHFB_MAGIC_2,
	SHFB_MAGIC_3,
};

static void
get_features(void)
{
	if (got_feature_bytes)
		return;
	got_feature_bytes = true;

	int fd = sys_semihost_open(":semihosting-features", 0);
	if (fd == -1)
		return;

	int len = sys_semihost_flen(fd);
	if (len < (int) sizeof(fb_magic))
		goto do_close;

	uint8_t magic[sizeof (fb_magic)];
	if (sys_semihost_read(fd, magic, sizeof (fb_magic)) != 0)
		goto do_close;
	if (memcmp(magic, fb_magic, sizeof (fb_magic)) != 0)
		goto do_close;

	int to_read = len - sizeof(fb_magic);
	if (to_read > (int) sizeof(feature_bytes))
		to_read = sizeof(feature_bytes);

	(void) sys_semihost_read(fd, feature_bytes, to_read);
do_close:
	sys_semihost_close(fd);
}

bool
sys_semihost_feature(uint8_t feature)
{
	get_features();
	uint8_t byte = (feature >> 3);
	uint8_t bit = (feature & 7);
	return (feature_bytes[byte] & (1 << bit)) != 0;
}
