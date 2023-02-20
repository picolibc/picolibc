/* collate.h: Internal BSD libc header, used in glob and regcomp, for instance.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */


#ifdef __cplusplus
extern "C" {
#endif

extern const int __collate_load_error;

extern int __wcollate_range_cmp (wint_t, wint_t);
extern int __wscollate_range_cmp (wint_t *, wint_t *, size_t, size_t);

int is_unicode_equiv (wint_t, wint_t);

int is_unicode_coll_elem (const wint_t *);

size_t next_unicode_char (wint_t *);

#ifdef __cplusplus
};
#endif
