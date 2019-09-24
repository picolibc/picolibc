/* Copyright © 2013 Bart Massey */
/* This program is licensed under the GPL version 2 or later.
   Please see the file COPYING in this distribution for
   license terms. */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static char buf[1024];

static void failmsg(int serial, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("test %d failed: ", serial);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

static int test(int serial, char *expect, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, 1024, fmt, ap);
    va_end(ap);
    if (n >= 1024) {
        failmsg(serial, "buffer overflow");
        return 1;
    }
    if (n != strlen(expect)) {
        failmsg(serial, "expected \"%s\" (%d), got \"%s\" (%d)",
		expect, strlen(expect), buf, n);
        return 1;
    }
    if (strcmp(buf, expect)) {
        failmsg(serial, "expected \"%s\", got \"%s\"", expect, buf);
        return 1;
    }
    return 0;
}

int main() {
    int result = 0;
#include "testcases.c"
    return result;
}
