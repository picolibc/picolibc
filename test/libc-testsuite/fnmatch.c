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
    { "[[?*\\\\]", "\\", 0, 0 },
    { "[]?*\\\\]", "]", 0, 0 },
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
    { "*[[:alpha:]]/*[[:alnum:]]", "a/b", FNM_PATHNAME, 0 },
    { "*[![:digit:]]*/[![:d-d]", "a/b", FNM_PATHNAME, 0 },
    { "*[![:digit:]]*/[[:d-d]", "a/[", FNM_PATHNAME, 0 },
    { "*[![:digit:]]*/[![:d-d]", "a/[", FNM_PATHNAME, -FNM_NOMATCH },
    { "a?b", "a.b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { "a*b", "a.b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { "a[.]b", "a.b", FNM_PATHNAME|FNM_PERIOD, 0 },
    { "[[=a=]]", "a", 0, 0 },
    { "[[=a=]][[=b=]]", "ab", 0, 0 },
    { "[[=a=]][[=c=]]", "ab", 0, FNM_NOMATCH },
    { "[[.foo.]]", "f", 0, -FNM_NOMATCH },
    { "[[.a.]]", "a", 0, 0 },
    { "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a"
      "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a"
      "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a"
      "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a",
      "aaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaa",
      0,
#ifdef __PICOLIBC__
      -FNM_NOMATCH
#else
      0
#endif
    },
};

#define zassert_equal(x, v) do {                                        \
        int __val = (x);                                                \
        if (__val != (v)) {                                             \
            printf("Test %d failed: got %d want %d. %s\n",              \
                   __LINE__, __val, (v), #x);                           \
            ret = 0;                                                    \
        }                                                               \
    } while(0)

#define zassert_not_equal(x, v) do {                                    \
        int __val = (x);                                                \
        if (__val == (v)) {                                             \
            printf("Test %d failed: unexpectedly got %d. %s\n",         \
                   __LINE__, __val, #x);                                \
            ret = 0;                                                    \
        }                                                               \
    } while(0)

#define zassert_ok(x) zassert_equal(x, 0)

/* Adopt Zephyr fnmatch tests for picolibc test suite */
static int zephyr_test_fnmatch(void) {
    int ret = 1;
    zassert_ok(fnmatch("*.c", "foo.c", 0));
    zassert_ok(fnmatch("*.c", ".c", 0));
    zassert_equal(fnmatch("*.a", "foo.c", 0), FNM_NOMATCH);
    zassert_ok(fnmatch("*.c", ".foo.c", 0));
    zassert_equal(fnmatch("*.c", ".foo.c", FNM_PERIOD), FNM_NOMATCH);
    zassert_ok(fnmatch("*.c", "foo.c", FNM_PERIOD));
    zassert_equal(fnmatch("a\\*.c", "a*.c", FNM_NOESCAPE), FNM_NOMATCH);
    zassert_equal(fnmatch("a\\*.c", "ax.c", 0), FNM_NOMATCH);
    zassert_ok(fnmatch("a[xy].c", "ax.c", 0));
    zassert_ok(fnmatch("a[!y].c", "ax.c", 0));
    zassert_equal(fnmatch("a[a/z]*.c", "a/x.c", FNM_PATHNAME), FNM_NOMATCH);
    zassert_ok(fnmatch("a/*.c", "a/x.c", FNM_PATHNAME));
    zassert_equal(fnmatch("a*.c", "a/x.c", FNM_PATHNAME), FNM_NOMATCH);
    zassert_ok(fnmatch("*/foo", "/foo", FNM_PATHNAME));
    zassert_ok(fnmatch("-O[01]", "-O1", 0));
    zassert_equal(fnmatch("[[?*\\]", "\\", 0), FNM_NOMATCH);    /* \\ escapes ], no bracket expr, no match */
    zassert_ok(fnmatch("[[?*\\]", "\\", FNM_NOESCAPE));         /* \\ unescaped, match */
    zassert_ok(fnmatch("[[?*\\\\]", "\\", 0));                  /* \\ escapes \\, match */
    zassert_equal(fnmatch("[]?*\\]", "]", 0), FNM_NOMATCH);     /* \\ escapes ], no bracket expr, no match */
    zassert_ok(fnmatch("[]?*\\]", "]", FNM_NOESCAPE));          /* \\ unescaped, match */
    zassert_ok(fnmatch("[]?*\\\\]", "]", 0));                   /* \\ escapes \\, match */
    zassert_ok(fnmatch("[!]a-]", "b", 0));
    zassert_ok(fnmatch("[]-_]", "^", 0));
    zassert_ok(fnmatch("[!]-_]", "X", 0));
    zassert_equal(fnmatch("??", "-", 0), FNM_NOMATCH);
    zassert_equal(fnmatch("*LIB*", "lib", FNM_PERIOD), FNM_NOMATCH);
    zassert_ok(fnmatch("a[/]b", "a/b", 0));
    zassert_equal(fnmatch("a[/]b", "a/b", FNM_PATHNAME), FNM_NOMATCH);
    zassert_ok(fnmatch("[a-z]/[a-z]", "a/b", 0));
    zassert_equal(fnmatch("*", "a/b", FNM_PATHNAME), FNM_NOMATCH);
    zassert_equal(fnmatch("*[/]b", "a/b", FNM_PATHNAME), FNM_NOMATCH);
    zassert_equal(fnmatch("*[b]", "a/b", FNM_PATHNAME), FNM_NOMATCH);
    zassert_equal(fnmatch("[*]/b", "a/b", 0), FNM_NOMATCH);
    zassert_ok(fnmatch("[*]/b", "*/b", 0));
    zassert_equal(fnmatch("[?]/b", "a/b", 0), FNM_NOMATCH);
    zassert_ok(fnmatch("[?]/b", "?/b", 0));
    zassert_ok(fnmatch("[[a]/b", "a/b", 0));
    zassert_ok(fnmatch("[[a]/b", "[/b", 0));
    zassert_equal(fnmatch("\\*/b", "a/b", 0), FNM_NOMATCH);
    zassert_ok(fnmatch("\\*/b", "*/b", 0));
    zassert_equal(fnmatch("\\?/b", "a/b", 0), FNM_NOMATCH);
    zassert_ok(fnmatch("\\?/b", "?/b", 0));
    zassert_ok(fnmatch("[/b", "[/b", 0));
    zassert_ok(fnmatch("\\[/b", "[/b", 0));
    zassert_ok(fnmatch("??"
                       "/b",
                       "aa/b", 0));
    zassert_ok(fnmatch("???b", "aa/b", 0));
    zassert_equal(fnmatch("???b", "aa/b", FNM_PATHNAME), FNM_NOMATCH);
    zassert_equal(fnmatch("?a/b", ".a/b", FNM_PATHNAME | FNM_PERIOD), FNM_NOMATCH);
    zassert_equal(fnmatch("a/?b", "a/.b", FNM_PATHNAME | FNM_PERIOD), FNM_NOMATCH);
    zassert_equal(fnmatch("*a/b", ".a/b", FNM_PATHNAME | FNM_PERIOD), FNM_NOMATCH);
    zassert_equal(fnmatch("a/*b", "a/.b", FNM_PATHNAME | FNM_PERIOD), FNM_NOMATCH);
    zassert_equal(fnmatch("[.]a/b", ".a/b", FNM_PATHNAME | FNM_PERIOD), FNM_NOMATCH);
    zassert_equal(fnmatch("a/[.]b", "a/.b", FNM_PATHNAME | FNM_PERIOD), FNM_NOMATCH);
    zassert_ok(fnmatch("*/?", "a/b", FNM_PATHNAME | FNM_PERIOD));
    zassert_ok(fnmatch("?/*", "a/b", FNM_PATHNAME | FNM_PERIOD));
    zassert_ok(fnmatch(".*/?", ".a/b", FNM_PATHNAME | FNM_PERIOD));
    zassert_ok(fnmatch("*/.?", "a/.b", FNM_PATHNAME | FNM_PERIOD));
    zassert_equal(fnmatch("*/*", "a/.b", FNM_PATHNAME | FNM_PERIOD), FNM_NOMATCH);
    zassert_ok(fnmatch("*?*/*", "a/.b", FNM_PERIOD));
    zassert_ok(fnmatch("*[.]/b", "a./b", FNM_PATHNAME | FNM_PERIOD));
    zassert_ok(fnmatch("*[[:alpha:]]/""*[[:alnum:]]", "a/b", FNM_PATHNAME));
    zassert_ok(fnmatch("*[![:digit:]]*/[![:d-d]", "a/b", FNM_PATHNAME));
    zassert_ok(fnmatch("*[![:digit:]]*/[[:d-d]", "a/[", FNM_PATHNAME));
    zassert_not_equal(fnmatch("*[![:digit:]]*/[![:d-d]", "a/[", FNM_PATHNAME), 0);
    zassert_ok(fnmatch("a?b", "a.b", FNM_PATHNAME | FNM_PERIOD));
    zassert_ok(fnmatch("a*b", "a.b", FNM_PATHNAME | FNM_PERIOD));
    zassert_ok(fnmatch("a[.]b", "a.b", FNM_PATHNAME | FNM_PERIOD));
    return ret;
}

static int test_fnmatch(void) {
    int i;
    unsigned int failed = 0;

    if (!zephyr_test_fnmatch())
        return 1;

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
