/* collate.h: Internal BSD libc header, used in glob and regcomp, for instance.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */


#ifdef __cplusplus
extern "C" {
#endif

extern const int __collate_load_error;

extern int __collate_range_cmp (int c1, int c2);

#ifdef __cplusplus
};
#endif
