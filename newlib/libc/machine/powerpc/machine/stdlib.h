#ifndef	_MACHSTDLIB_H_
#define	_MACHSTDLIB_H_

#ifndef __STRICT_ANSI__

# if defined(__ALTIVEC__)

void *vec_calloc (size_t __nmemb, size_t __size);
void *_vec_calloc_r (struct _reent *, size_t __nmemb, size_t __size);
void   vec_free (void *);
#define _vec_freer _freer
void *vec_malloc (size_t __size);
#define _vec_mallocr _memalign_r
void *vec_realloc (void *__r, size_t __size);
void *_vec_realloc_r (struct _reent *, void *__r, size_t __size);

# endif /* __ALTIVEC__ */

# if defined(__SPE__)

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


