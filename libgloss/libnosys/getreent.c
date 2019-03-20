/*
 * Stub version of write.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"

struct _reent *
__attribute__((weak))
__getreent (void)
{
    return NULL;
}

stub_warning(__getreent)
