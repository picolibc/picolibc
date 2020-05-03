/* Copyright © 2013 Bart Massey */
/* This program is licensed under the GPL version 2 or later.
   Please see the file COPYING.GPL2 in this distribution for
   license terms. */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifndef TINY_STDIO
#define printf_float(x) x
#endif

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
    double dv;
    int n;
    char *star;
    switch (fmt[strlen(fmt)-1]) {
    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G':
	    star = strchr(fmt, '*');
	    if (star) {
		    if (strchr(star+1, '*')) {
			    int iv1 = va_arg(ap, int);
			    int iv2 = va_arg(ap, int);
			    dv = va_arg(ap, double);
			    n = snprintf(buf, 1024, fmt, iv1, iv2, printf_float(dv));
		    } else  {
			    int iv = va_arg(ap, int);
			    dv = va_arg(ap, double);
			    n = snprintf(buf, 1024, fmt, iv, printf_float(dv));
		    }
	    } else {
		    dv = va_arg(ap, double);
		    n = snprintf(buf, 1024, fmt, printf_float(dv));
	    }
	    break;
    default:
	    n = vsnprintf(buf, 1024, fmt, ap);
	    break;
    }
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
