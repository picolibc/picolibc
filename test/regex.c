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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <stdbool.h>

#define MAX_MATCH	10

struct test {
	const char	*pattern;
	const char	*string;
	int		ret;
	int		nmatch;
	regmatch_t matches[MAX_MATCH];
};

const struct test tests[] = {
	{ .pattern = ".", .string = "xxx",
	  .ret = 0, .nmatch = 1, .matches = { { .rm_so = 0, .rm_eo = 1 } }
	},
	{ .pattern = ".*", .string = "xxx",
	  .ret = 0, .nmatch = 1, .matches = { { .rm_so = 0, .rm_eo = 3 } }
	},
	{ .pattern = "(.)(.)(.)", .string = "xxx",
	  .ret = 0, .nmatch = 4, .matches = {
			{ .rm_so = 0, .rm_eo = 3 },
			{ .rm_so = 0, .rm_eo = 1 },
			{ .rm_so = 1, .rm_eo = 2 },
			{ .rm_so = 2, .rm_eo = 3 },
		},
	},
	{ .pattern = "x[a-c]*y", .string = "fooxaccabybar",
	  .ret = 0, .nmatch = 1, .matches = { { .rm_so = 3, .rm_eo = 10 } }
	},
};

#define NTEST (sizeof(tests)/sizeof(tests[0]))

int
main(void)
{
	unsigned	t;
	int	m;
	int	errors = 0;
	regmatch_t matches[MAX_MATCH];
	regex_t regex;
	int	ret;
	int	run = 0;

	for (t = 0; t < NTEST; t++) {
		ret = regcomp(&regex, tests[t].pattern, REG_EXTENDED);
		if (ret != 0) {
			printf("expression \"%s\" failed to compile: %d\n", tests[t].pattern, ret);
			errors++;
			continue;
		}
		ret = regexec(&regex, tests[t].string, MAX_MATCH, matches, 0);
		regfree(&regex);
		if (ret != tests[t].ret) {
			printf("match \"%s\" with \"%s\" bad result got %d != expect %d\n",
			       tests[t].pattern, tests[t].string, ret, tests[t].ret);
			errors++;
			continue;
		}
		if (ret == 0) {
			for (m = 0; m < MAX_MATCH; m++) {
				if (m < tests[t].nmatch) {
					if (matches[m].rm_so != tests[t].matches[m].rm_so ||
					    matches[m].rm_eo != tests[t].matches[m].rm_eo) {
						printf("match %d wrong range got (%td,%td) expect (%td,%td)\n",
						       m,
						       matches[m].rm_so,
						       matches[m].rm_eo,
						       tests[t].matches[m].rm_so,
						       tests[t].matches[m].rm_eo);
						errors++;
					}
				} else {
					if (matches[m].rm_so != -1) {
						printf("match %d should be unused\n",
						       m);
						errors++;
					}
				}
			}
		}
		++run;
	}
	printf("regex: %d tests %d errors\n", run, errors);
	return errors;
}
