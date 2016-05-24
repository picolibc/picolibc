/* cygtls_padsize.h: Extra file to be included from utils.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* FIXME: Find some way to autogenerate this value */
#ifdef __x86_64__
const int CYGTLS_PADSIZE = 12800;	/* Must be 16-byte aligned */
#else
const int CYGTLS_PADSIZE = 12700;
#endif
