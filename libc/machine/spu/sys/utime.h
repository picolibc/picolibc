/* Copyright (c) 2007 Patrick Mansfield <patmans@us.ibm.com> */
#ifndef _SYS_UTIME_H
#define _SYS_UTIME_H

#include <sys/cdefs.h>

_BEGIN_STD_C

/*
 * Per POSIX
 */
struct utimbuf {
    time_t actime;
    time_t modtime;
};

int utime(const char *, const struct utimbuf *);

_END_STD_C

#endif
