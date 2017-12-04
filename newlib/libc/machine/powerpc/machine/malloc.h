#ifndef	_MACHMALLOC_H_
#define	_MACHMALLOC_H_

# if defined(__ALTIVEC__)

void *_EXFUN(vec_calloc,(size_t __nmemb, size_t __size));
void *_EXFUN(_vec_calloc_r,(struct _reent *, size_t __nmemb, size_t __size));
_VOID   _EXFUN(vec_free,(void *));
#define _vec_freer _freer
void *_EXFUN(vec_malloc,(size_t __size));
#define _vec_mallocr _memalign_r
void *_EXFUN(vec_realloc,(void *__r, size_t __size));
void *_EXFUN(_vec_realloc_r,(struct _reent *, void *__r, size_t __size));

# endif /* __ALTIVEC__ */


#endif	/* _MACHMALLOC_H_ */


