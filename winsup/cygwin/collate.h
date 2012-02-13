/* collate.h: Internal BSD libc header, used in glob and regcomp, for instance.

   Copyright 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */


#ifdef __cplusplus
extern "C" {
#endif

/* We never have a collate load error. */
const int __collate_load_error = 0;

int __collate_range_cmp (int c1, int c2);

#ifdef __cplusplus
};
#endif
