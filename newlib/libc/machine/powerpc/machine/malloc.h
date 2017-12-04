#ifndef	_MACHMALLOC_H_
#define	_MACHMALLOC_H_

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


#endif	/* _MACHMALLOC_H_ */


