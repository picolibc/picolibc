/* cygxdr.h:

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */
#ifndef _CYGXDR_H
#define _CYGXDR_H

extern "C"
{

typedef void (*xdr_vprintf_t)(const char *, va_list);

xdr_vprintf_t xdr_set_vprintf (xdr_vprintf_t);

void cygxdr_vwarnx (const char *, va_list);

}

#endif

