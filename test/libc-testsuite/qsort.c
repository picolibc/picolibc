/*
 * Copyright © 2005-2020 Rich Felker
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int scmp(const void *a, const void *b)
{
	return strcmp(*(char **)a, *(char **)b);
}

static int icmp(const void *a, const void *b)
{
        long d = *(long*)a - *(long*)b;
	return (d > 0) ? 1 : (d < 0) ? -1 : 0;
}

struct three {
    unsigned char b[3];
};

#define i3(x)                                                    \
        { (unsigned char) (((int32_t)(x)) >> 16), (unsigned char) ((x) >> 8), \
                        (unsigned char) ((x) >> 0) }

static int tcmp(const void *av, const void *bv)
{
    const struct three *a = av, *b = bv;
    int c;
    int i;

    for (i = 0; i < 3; i++) {
        c = (int) a->b[i] - (int) b->b[i];
        if (c)
            return c;
    }
    return 0;
}

#define FAIL(m) do {                                            \
        printf(__FILE__ ":%d: %s failed\n", __LINE__, m);       \
        err++;                                                  \
    } while(0)

/* 26 items -- even */
static char *s[] = {
    "Bob", "Alice", "John", "Ceres",
    "Helga", "Drepper", "Emeralda", "Zoran",
    "Momo", "Frank", "Pema", "Xavier",
    "Yeva", "Gedun", "Irina", "Nono",
    "Wiener", "Vincent", "Tsering", "Karnica",
    "Lulu", "Quincy", "Osama", "Riley",
    "Ursula", "Sam"
};
/* 23 items -- odd, prime */
static long n[] = {
    879045, 394, 99405644, 33434, 232323, 4334, 5454,
    343, 45545, 454, 324, 22, 34344, 233, 45345, 343,
    848405, 3434, 3434344, 3535, 93994, 2230404, 4334
};

static struct three t[] = {
    i3(879045), i3(394), i3(99405644), i3(33434), i3(232323), i3(4334), i3(5454),
    i3(343), i3(45545), i3(454), i3(324), i3(22), i3(34344), i3(233), i3(45345), i3(343),
    i3(848405), i3(3434), i3(3434344), i3(3535), i3(93994), i3(2230404), i3(4334)
};

int test_qsort(void)
{
	int i;
	int err=0;

	qsort(s, sizeof(s)/sizeof(s[0]), sizeof(s[0]), scmp);
	for (i=0; i<(int) (sizeof(s)/sizeof(char *)-1); i++) {
		if (strcmp(s[i], s[i+1]) > 0) {
			FAIL("string sort");
			for (i=0; i<(int)(sizeof(s)/sizeof(char *)); i++)
				printf("\t%s\n", s[i]);
			break;
		}
	}

	qsort(n, sizeof(n)/sizeof(n[0]), sizeof(n[0]), icmp);
	for (i=0; i<(int)(sizeof(n)/sizeof(n[0])-1); i++) {
		if (n[i] > n[i+1]) {
			FAIL("integer sort");
			for (i=0; i<(int)(sizeof(n)/sizeof(n[0])); i++)
				printf("\t%ld\n", n[i]);
			break;
		}
	}

        qsort(t, sizeof(t)/sizeof(t[0]), sizeof(t[0]), tcmp);
	for (i=0; i<(int)(sizeof(t)/sizeof(t[0])-1); i++) {
                if (tcmp(&t[i], &t[i+1]) > 0) {
			FAIL("three byte sort");
			for (i=0; i<(int)(sizeof(t)/sizeof(t[0])); i++)
                                printf("\t0x%02x%02x%02x\n", t[i].b[0], t[i].b[1], t[i].b[2]);
			break;
		}
	}


	return err;
}

#undef qsort
#define TEST_NAME qsort
#include "testcase.h"
