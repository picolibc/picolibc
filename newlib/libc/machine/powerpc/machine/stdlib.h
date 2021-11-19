/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#ifndef	_MACHSTDLIB_H_
#define	_MACHSTDLIB_H_

#ifndef __STRICT_ANSI__

# if defined(__ALTIVEC__) && 0

void *vec_calloc (size_t __nmemb, size_t __size);
void *_vec_calloc_r (struct _reent *, size_t __nmemb, size_t __size);
void   vec_free (void *);
#define _vec_freer _freer
void *vec_malloc (size_t __size);
#define _vec_mallocr _memalign_r
void *vec_realloc (void *__r, size_t __size);
void *_vec_realloc_r (struct _reent *, void *__r, size_t __size);

# endif /* __ALTIVEC__ */

# if defined(__SPE__) && 0

#define __need_inttypes
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
__int16_t   atosfix16 (const char *__str);
__int16_t   _atosfix16_r (struct _reent *, const char *__str);
__int32_t   atosfix32 (const char *__str);
__int32_t   _atosfix32_r (struct _reent *, const char *__str);
__int64_t   atosfix64 (const char *__str);
__int64_t   _atosfix64_r (struct _reent *, const char *__str);

__uint16_t atoufix16 (const char *__str);
__uint16_t _atoufix16_r (struct _reent *, const char *__str);
__uint32_t atoufix32 (const char *__str);
__uint32_t _atoufix32_r (struct _reent *, const char *__str);
__uint64_t atoufix64 (const char *__str);
__uint64_t _atoufix64_r (struct _reent *, const char *__str);

__int16_t   strtosfix16 (const char *__str, char **__endptr);
__int16_t   _strtosfix16_r (struct _reent *, const char *__str, 
                 char **__endptr);
__int32_t   strtosfix32 (const char *__str, char **__endptr);
__int32_t   _strtosfix32_r (struct _reent *, const char *__str, 
                 char **__endptr);
__int64_t   strtosfix64 (const char *__str, char **__endptr);
__int64_t   _strtosfix64_r (struct _reent *, const char *__str, 
                 char **__endptr);

__uint16_t strtoufix16 (const char *__str, char **__endptr);
__uint16_t _strtoufix16_r (struct _reent *, const char *__str, 
                 char **__endptr);
__uint32_t strtoufix32 (const char *__str, char **__endptr);
__uint32_t _strtoufix32_r (struct _reent *, const char *__str, 
                 char **__endptr);
__uint64_t strtoufix64 (const char *__str, char **__endptr);
__uint64_t _strtoufix64_r (struct _reent *, const char *__str, 
                 char **__endptr);
#ifdef __cplusplus
}
#endif

# endif /* __SPE__ */

#endif /* !__STRICT_ANSI__ */


#endif	/* _MACHSTDLIB_H_ */


