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
#include <stdio.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <unistd.h>

/* adapted from dietlibc's test-newfnmatch.c */

/*
 * xlat / printflags adapted from strace
 * http://www.liacs.nl/~wichert/strace/
 */

#define FLAG(f)	{ f, #f }

const struct xlat {
  int val;
  char *str;
} fnmatch_flags[] = {
  FLAG(FNM_NOESCAPE),
  FLAG(FNM_PATHNAME),
  FLAG(FNM_PERIOD),
  {0,            NULL},
};

static void printflags(const struct xlat *map, int flags) {
  char * sep;

  if (! flags) {
    printf("0");
    return;
  }

  sep = "";
  for (; map->str; map++) {
    if (map->val && (flags & map->val) == map->val) {
      printf("%s%s", sep, map->str);
      sep = "|";
      flags &= ~(map->val);
    }
  }
  if (flags) printf("%sunknown=%#x", sep, flags);
}

/*
 * tests harness adapted from glibc testfnm.c
 */
const struct {
    const char *pattern;
    const char *string;
    int flags;
    int expected;
} tests[] = {
    /* begin dietlibc tests */
    { "*.c", "foo.c", 0, 0 },
    { "*.c", ".c", 0, 0 },
    { "*.a", "foo.c", 0, FNM_NOMATCH },
    { "*.c", ".foo.c", 0, 0 },
    { "*.c", ".foo.c", FNM_PERIOD, FNM_NOMATCH },
    { "*.c", "foo.c", FNM_PERIOD, 0 },
    { "a\\*.c", "a*.c", FNM_NOESCAPE, FNM_NOMATCH },
    { "a\\*.c", "ax.c", 0, FNM_NOMATCH },
    { "a[xy].c", "ax.c", 0, 0 },
    { "a[!y].c", "ax.c", 0, 0 },
    { "a[a/z]*.c", "a/x.c", FNM_PATHNAME, FNM_NOMATCH },
    { "a/*.c", "a/x.c", FNM_PATHNAME, 0 },
    { "a*.c", "a/x.c", FNM_PATHNAME, FNM_NOMATCH },
    { "*/foo", "/foo", FNM_PATHNAME, 0 },
    { "-O[01]", "-O1", 0, 0 },
#ifndef __PICOLIBC__
    { "[[?*\\]", "\\", 0, 0 },
    { "[]?*\\]", "]", 0, 0 },
#endif
    /* initial right-bracket tests */
    { "[!]a-]", "b", 0, 0 },
    { "[]-_]", "^", 0, 0 },   /* range: ']', '^', '_' */
    { "[!]-_]", "X", 0, 0 },
    { "??", "-", 0, FNM_NOMATCH },
    /* begin glibc tests */
    { "*LIB*", "lib", FNM_PERIOD, FNM_NOMATCH },
    { "a[/]b", "a/b", 0, 0 },
    { "a[/]b", "a/b", FNM_PATHNAME, FNM_NOMATCH },
    { "[a-z]/[a-z]", "a/b", 0, 0 },
    { "*", "a/b", FNM_PATHNAME, FNM_NOMATCH },
    { "*[/]b", "a/b", FNM_PATHNAME, FNM_NOMATCH },
    { "*[b]", "a/b", FNM_PATHNAME, FNM_NOMATCH },
    { "[*]/b", "a/b", 0, FNM_NOMATCH },
    { "[*]/b", "*/b", 0, 0 },
    { "[?]/b", "a/b", 0, FNM_NOMATCH },
    { "[?]/b", "?/b", 0, 0 },
    { "[[a]/b", "a/b", 0, 0 },
    { "[[a]/b", "[/b", 0, 0 },
    { "\\*/b", "a/b", 0, FNM_NOMATCH },
    { "\\*/b", "*/b", 0, 0 },
    { "\\?/b", "a/b", 0, FNM_NOMATCH },
    { "\\?/b", "?/b", 0, 0 },
    { "[/b", "[/b", 0, 0 },
    { "\\[/b", "[/b", 0, 0 },
    { "??""/b", "aa/b", 0, 0 },
    { "???b", "aa/b", 0, 0 },
    { "???b", "aa/b", FNM_PATHNAME, FNM_NOMATCH },
    { "?a/b", ".a/b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
    { "a/?b", "a/.b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
    { "*a/b", ".a/b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
    { "a/*b", "a/.b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
    { "[.]a/b", ".a/b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
    { "a/[.]b", "a/.b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
    { "*/?", "a/b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { "?/*", "a/b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { ".*/?", ".a/b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { "*/.?", "a/.b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { "*/*", "a/.b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
    { "*?*/*", "a/.b", FNM_PERIOD, 0 },
    { "*[.]/b", "a./b", FNM_PATHNAME|FNM_PERIOD, 0 },
#ifndef __PICOLIBC__
    { "*[[:alpha:]]/*[[:alnum:]]", "a/b", FNM_PATHNAME, 0 },
#endif
    /* These three tests should result in error according to SUSv3.
     * See XCU 2.13.1, XBD 9.3.5, & fnmatch() */
    { "*[![:digit:]]*/[![:d-d]", "a/b", FNM_PATHNAME, -FNM_NOMATCH },
    { "*[![:digit:]]*/[[:d-d]", "a/[", FNM_PATHNAME, -FNM_NOMATCH },
    { "*[![:digit:]]*/[![:d-d]", "a/[", FNM_PATHNAME, -FNM_NOMATCH },
    { "a?b", "a.b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { "a*b", "a.b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { "a[.]b", "a.b", FNM_PATHNAME|FNM_PERIOD, 0 },
};

int test_fnmatch(void) {
    int i;
    unsigned int failed = 0;

    for (i = 0; i < (int)(sizeof(tests) / sizeof(*tests)); i++) {
        int r, x;

        r = fnmatch(tests[i].pattern, tests[i].string, tests[i].flags);
	x = tests[i].expected;
	if (r != x && (r != FNM_NOMATCH || x != -FNM_NOMATCH)) {
            failed++;
            printf("fail - fnmatch(\"%s\", \"%s\", ",
                    tests[i].pattern, tests[i].string);
            printflags(fnmatch_flags, tests[i].flags);
            printf(") => %d (expected %d)\n", r, tests[i].expected);
	}
    }

    return failed;
}

#define TEST_NAME fnmatch
#include "testcase.h"
