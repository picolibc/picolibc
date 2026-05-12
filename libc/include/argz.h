/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#ifndef _ARGZ_H_
#define _ARGZ_H_

#include <sys/cdefs.h>
#define __need_size_t
#include <stddef.h>

_BEGIN_STD_C

#ifndef _ERROR_T_DECLARED
typedef int error_t;
#define _ERROR_T_DECLARED
#endif

/* The newlib implementation of these functions assumes that sizeof(char) == 1. */
error_t argz_create(char * const argv[], char **argz, size_t *argz_len) __picolibc_export;
error_t argz_create_sep(const char *string, int sep, char **argz,
                        size_t *argz_len) __picolibc_export;
size_t  argz_count(const char *argz, size_t argz_len) __picolibc_export;
void    argz_extract(char *argz, size_t argz_len, char **argv) __picolibc_export;
void    argz_stringify(char *argz, size_t argz_len, int sep) __picolibc_export;
error_t argz_add(char **argz, size_t *argz_len, const char *str) __picolibc_export;
error_t argz_add_sep(char **argz, size_t *argz_len, const char *str, int sep) __picolibc_export;
error_t argz_append(char **argz, size_t *argz_len, const char *buf,
                    size_t buf_len) __picolibc_export;
error_t argz_delete(char **argz, size_t *argz_len, char *entry) __picolibc_export;
error_t argz_insert(char **argz, size_t *argz_len, char *before,
                    const char *entry) __picolibc_export;
char   *argz_next(char *argz, size_t argz_len, const char *entry) __picolibc_export;
error_t argz_replace(char **argz, size_t *argz_len, const char *str, const char *with,
                     unsigned *replace_count) __picolibc_export;

_END_STD_C

#endif /* _ARGZ_H_ */
