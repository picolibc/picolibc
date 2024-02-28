/*
 * Copyright (c) 2012, 2013 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Implementation of <<malloc>> <<free>> <<calloc>> <<realloc>>, optional
 * as to be reenterable.
 *
 * Interface documentation refer to malloc.c.
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/config.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <stdint.h>

#if MALLOC_DEBUG
#include <assert.h>
#define MALLOC_LOCK do { __LIBC_LOCK(); __malloc_validate(); } while(0)
#define MALLOC_UNLOCK do { __malloc_validate(); __LIBC_UNLOCK(); } while(0)
#else
#define MALLOC_LOCK __LIBC_LOCK()
#define MALLOC_UNLOCK __LIBC_UNLOCK()
#undef assert
#define assert(x) ((void)0)
#endif

typedef union {
    void *p;
    double d;
    long long ll;
    size_t s;
} align_chunk_t;

/*          --------------------------------------
 *          | size                               |
 *   chunk->| When allocated: data               |
 *          | When freed: pointer to next free   |
 *          | chunk                              |
 *          --------------------------------------
 *
 * mem_ptr is aligned to MALLOC_CHUNK_ALIGN. That means that the
 * address of 'size' may not be aligned to MALLOC_CHUNK_ALIGN. But it
 * will be aligned to MALLOC_HEAD_ALIGN.
 *
 * size is set so that a chunk starting at chunk+size will be
 * aligned correctly
 *
 * We can't use a single struct containing both size and next as that
 * may insert padding between the size and pointer fields when
 * pointers are larger than size_t.
 */

typedef struct malloc_head {
    size_t      size;
} head_t;

typedef struct malloc_chunk {
    struct malloc_chunk *next;
} chunk_t;

/* Alignment of allocated chunk. Compute the alignment required from a
 * range of types */
#define MALLOC_CHUNK_ALIGN	_Alignof(align_chunk_t)

/* Alignment of the header. Never larger than MALLOC_CHUNK_ALIGN, but
 * may be smaller on some targets when size_t is smaller than
 * align_chunk_t.
 */
#define MALLOC_HEAD_ALIGN	_Alignof(head_t)

#define MALLOC_HEAD 		sizeof(head_t)

/* nominal "page size" */
#define MALLOC_PAGE_ALIGN 	(0x1000)

/* Minimum allocation size */
#define MALLOC_MINSIZE		__align_up(MALLOC_HEAD + sizeof(chunk_t), MALLOC_HEAD_ALIGN)

/* Maximum allocation size */
#define MALLOC_MAXSIZE 		(SIZE_MAX - (MALLOC_HEAD + 2*MALLOC_CHUNK_ALIGN))

static inline size_t *_size_ref(chunk_t *chunk)
{
    return (size_t *) ((char *) chunk - MALLOC_HEAD);
}

static inline size_t _size(chunk_t *chunk)
{
    return *_size_ref(chunk);
}

static inline void _set_size(chunk_t *chunk, size_t size)
{
    *_size_ref(chunk) = size;
}

/* Forward data declarations */
extern chunk_t *__malloc_free_list;
extern char * __malloc_sbrk_start;
extern char * __malloc_sbrk_top;

/* Forward function declarations */
void __malloc_validate(void);
void __malloc_validate_block(chunk_t *r);
void * __malloc_sbrk_aligned(size_t s);
bool __malloc_grow_chunk(chunk_t *c, size_t new_size);

/* Work around compiler optimizing away stores to 'size' field before
 * call to free.
 */
#ifdef _HAVE_ALIAS_ATTRIBUTE
extern void __malloc_free(void *);
extern void *__malloc_malloc(size_t);
#else
#define __malloc_free(x) free(x)
#define __malloc_malloc(x) malloc(x)
#endif

/* convert storage pointer to chunk */
static inline chunk_t *
ptr_to_chunk(void * ptr)
{
    return (chunk_t *) ptr;
}

/* convert chunk to storage pointer */
static inline void *
chunk_to_ptr(chunk_t *c)
{
    return c;
}

/* Convert address of chunk region to chunk pointer */
static inline chunk_t *
blob_to_chunk(void *blob)
{
    return (chunk_t *) ((char *) blob + MALLOC_HEAD);
}

/* Convert chunk pointer to address of chunk region */
static inline void *
chunk_to_blob(chunk_t *c)
{
    return (void *) ((char *) c - MALLOC_HEAD);
}

/* end of chunk -- address of first byte past chunk storage */
static inline void *
chunk_end(chunk_t *c)
{
    size_t *s = _size_ref(c);
    return (char *) s + *s;
}

/* next chunk in memory -- address of chunk header past this chunk */
static inline chunk_t *
chunk_after(chunk_t *c)
{
    return (chunk_t *) ((char *) c + _size(c));
}

/* chunk size needed to hold 'malloc_size' bytes */
static inline size_t
chunk_size(size_t malloc_size)
{
    /* Keep all blocks aligned */
    malloc_size = __align_up(malloc_size, MALLOC_CHUNK_ALIGN);

    /* Add space for header */
    malloc_size += MALLOC_HEAD;

    /* fill the gap between chunks */
    malloc_size += (MALLOC_CHUNK_ALIGN - MALLOC_HEAD_ALIGN);

    /* Make sure the requested size is big enough to hold a free chunk */
    malloc_size = MAX(MALLOC_MINSIZE, malloc_size);
    return malloc_size;
}

/* available storage in chunk */
static inline size_t
chunk_usable(chunk_t *c)
{
    return _size(c) - MALLOC_HEAD;
}

/* assign 'size' to the specified chunk and return it to the free
 * pool */
static inline void
make_free_chunk(chunk_t *c, size_t size)
{
    _set_size(c, size);
    __malloc_free(chunk_to_ptr(c));
}

#ifdef DEFINE_MALLOC
/* List list header of free blocks */
chunk_t *__malloc_free_list;

/* Starting point of memory allocated from system */
char * __malloc_sbrk_start;
char * __malloc_sbrk_top;

/** Function __malloc_sbrk_aligned
  * Algorithm:
  *   Use sbrk() to obtain more memory and ensure the storage is
  *   MALLOC_CHUNK_ALIGN aligned. Optimise for the case that it is
  *   already aligned - only ask for extra padding after we know we
  *   need it
  */
void* __malloc_sbrk_aligned(size_t s)
{
    char *p, *align_p;

#ifdef __APPLE__
    /* Mac OS X 'emulates' sbrk, but the
     * parameter is int, not intptr_t or ptrdiff_t,
     */
    int d = (int) s;
    if (d < 0 || (size_t) d != s)
	return (void *)-1;
#else
    ptrdiff_t d = (ptrdiff_t)s;

    if (d < 0)
	return (void *)-1;
#endif
    p = sbrk(d);

    /* sbrk returns -1 if fail to allocate */
    if (p == (void *)-1)
        return p;

    __malloc_sbrk_top = p + s;

    /* Adjust returned space so that the storage area
     * is MALLOC_CHUNK_ALIGN aligned and the head is
     * MALLOC_HEAD_ALIGN aligned.
     */
    align_p = __align_up(p + MALLOC_HEAD, MALLOC_CHUNK_ALIGN) - MALLOC_HEAD;

    if (align_p != p)
    {
        /* p is not aligned, ask for a few more bytes so that we have
         * s bytes reserved from align_p. This should only occur for
         * the first sbrk in a chunk of memory as all others should be
         * aligned to the right value as chunk sizes are selected to
         * make them abut in memory
	 */
	intptr_t adjust = align_p - p;
        char *extra = sbrk(adjust);
        if (extra != p + s)
            return (void *) -1;
	__malloc_sbrk_top = extra + adjust;
    }
    if (__malloc_sbrk_start == NULL)
	__malloc_sbrk_start = align_p;

    return align_p;
}

bool
__malloc_grow_chunk(chunk_t *c, size_t new_size)
{
    char *chunk_e = chunk_end(c);

    if (chunk_e != __malloc_sbrk_top)
	return false;
    size_t add_size = MAX(MALLOC_MINSIZE, new_size - _size(c));

    /* Ask for the extra memory needed */
    char *heap = __malloc_sbrk_aligned(add_size);

    /* Check if we got what we wanted */
    if (heap == chunk_e)
    {
	/* Set size and return */
	*_size_ref(c) += add_size;
	return true;
    }

    if (heap != (char *) -1)
    {
	/* sbrk returned unexpected memory, free it */
	make_free_chunk((chunk_t *) (heap + MALLOC_HEAD), add_size);
    }
    return false;
}

/** Function malloc
  * Algorithm:
  *   Walk through the free list to find the first match. If fails to find
  *   one, call sbrk to allocate a new chunk_t.
  */
void *malloc(size_t s)
{
    chunk_t **p, *r;
    char * ptr;
    size_t alloc_size;

    if (s > MALLOC_MAXSIZE)
    {
        errno = ENOMEM;
        return NULL;
    }

    alloc_size = chunk_size(s);

    MALLOC_LOCK;

    for (p = &__malloc_free_list; (r = *p) != NULL; p = &r->next)
    {
        if (_size(r) >= alloc_size)
        {
	    size_t rem = _size(r) - alloc_size;

            if (rem >= MALLOC_MINSIZE)
            {
                /* Find a chunk_t that much larger than required size, break
		 * it into two chunks and return the first one
		 */

		chunk_t *s = (chunk_t *)((char *)r + alloc_size);
                _set_size(s, rem);
		s->next = r->next;
		*p = s;

                _set_size(r, alloc_size);
            }
	    else
	    {
		/* Find a chunk_t that is exactly the size or slightly bigger
		 * than requested size, just return this chunk_t
		 */
		*p = r->next;
	    }
            break;
        }
	if (!r->next && __malloc_grow_chunk(r, alloc_size))
	{
	    /* Grow the last chunk in memory to the requested size,
	     * just return it
	     */
	    *p = r->next;
	    break;
	}
    }

    /* Failed to find a appropriate chunk_t. Ask for more memory */
    if (r == NULL)
    {
        void *blob = __malloc_sbrk_aligned(alloc_size);

        /* sbrk returns -1 if fail to allocate */
        if (blob == (void *)-1)
        {
            errno = ENOMEM;
            MALLOC_UNLOCK;
            return NULL;
        }
        r = blob_to_chunk(blob);
        _set_size(r, alloc_size);
    }

    MALLOC_UNLOCK;

    ptr = chunk_to_ptr(r);

    memset(ptr, '\0', alloc_size - MALLOC_HEAD);

    return ptr;
}
#ifdef _HAVE_ALIAS_ATTRIBUTE
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(malloc, __malloc_malloc);
#pragma GCC diagnostic pop
#endif
#endif /* DEFINE_MALLOC */

#ifdef DEFINE_FREE

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wanalyzer-null-dereference"
#endif

/** Function free
  * Implementation of libc free.
  * Algorithm:
  *  Maintain a global free chunk_t single link list, headed by global
  *  variable __malloc_free_list.
  *  When free, insert the to-be-freed chunk_t into free list. The place to
  *  insert should make sure all chunks are sorted by address from low to
  *  high.  Then merge with neighbor chunks if adjacent.
  */
void free (void * free_p)
{
    chunk_t     *p_to_free;
    chunk_t     **p, *r;

    if (free_p == NULL) return;

    p_to_free = ptr_to_chunk(free_p);
    p_to_free->next = NULL;
#if MALLOC_DEBUG
    __malloc_validate_block(p_to_free);
#endif

    MALLOC_LOCK;

    for (p = &__malloc_free_list; (r = *p) != NULL; p = &r->next)
    {
	/* Insert in address order */
	if (p_to_free < r)
	    break;

	/* Merge blocks together */
	if (chunk_after(r) == p_to_free)
	{
	    *_size_ref(r) += _size(p_to_free);
	    p_to_free = r;
	    r = r->next;
	    goto no_insert;
	}

	/* Check for double free */
	if (p_to_free == r)
	{
	    errno = ENOMEM;
	    MALLOC_UNLOCK;
	    return;
	}

    }
    p_to_free->next = r;
    *p = p_to_free;

no_insert:

    /* Merge blocks together */
    if (chunk_after(p_to_free) == r)
    {
	*_size_ref(p_to_free) += _size(r);
	p_to_free->next = r->next;
    }

    MALLOC_UNLOCK;
}
#ifdef _HAVE_ALIAS_ATTRIBUTE
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(free, __malloc_free);
__strong_reference(free, cfree);
#pragma GCC diagnostic pop
#endif
#endif /* DEFINE_FREE */

#ifdef DEFINE_CFREE
#ifndef _HAVE_ALIAS_ATTRIBUTE
void cfree(void * ptr)
{
    free(ptr);
}
#endif
#endif /* DEFINE_CFREE */

#ifdef DEFINE_CALLOC
#include "mul_overflow.h"
/* Function calloc
 *
 * Implement calloc by multiplying sizes (with overflow check) and
 * calling malloc (which sets to zero)
 */

void * calloc(size_t n, size_t elem)
{
    size_t bytes;

    if (mul_overflow (n, elem, &bytes))
    {
        errno = ENOMEM;
        return NULL;
    }
    return malloc(bytes);
}
#endif /* DEFINE_CALLOC */

#ifdef DEFINE_REALLOC

/* Function realloc
 *
 * Implement either by merging adjacent free memory
 * or by calling malloc/memcpy
 */
void * realloc(void * ptr, size_t size)
{
    void * mem;

    if (ptr == NULL)
	return malloc(size);

    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    if (size > MALLOC_MAXSIZE)
    {
        errno = ENOMEM;
        return NULL;
    }

    size_t new_size = chunk_size(size);
    chunk_t *p_to_realloc = ptr_to_chunk(ptr);

#if MALLOC_DEBUG
    __malloc_validate_block(p_to_realloc);
#endif

    size_t old_size = _size(p_to_realloc);

    /* See if we can avoid allocating new memory
     * when increasing the size
     */
    if (new_size > old_size)
    {
	void *chunk_e = chunk_end(p_to_realloc);

	MALLOC_LOCK;

	if (__malloc_grow_chunk(p_to_realloc, new_size))
	{
	    /* clear new memory */
	    memset(chunk_e, '\0', new_size - old_size);
	    /* adjust chunk_t size */
	    old_size = new_size;
	}
	else
	{
	    chunk_t **p, *r;

	    /* Check to see if there's a chunk_t of free space just past
	     * the current block, merge it in in case that's useful
	     */
	    for (p = &__malloc_free_list; (r = *p) != NULL; p = &r->next)
	    {
		if (r == chunk_e)
		{
		    size_t r_size = _size(r);

		    /* remove R from the free list */
		    *p = r->next;

		    /* clear the memory from r */
		    memset(r, '\0', r_size);

		    /* add it's size to our block */
		    old_size += r_size;
		    _set_size(p_to_realloc, old_size);
		    break;
		}
		if (p_to_realloc < r)
		    break;
	    }
	}

	MALLOC_UNLOCK;
    }

    if (new_size <= old_size)
    {
	size_t extra = old_size - new_size;

	/* If there's enough space left over, split it out
	 * and free it
	 */
	if (extra >= MALLOC_MINSIZE) {
	    _set_size(p_to_realloc, new_size);
	    make_free_chunk(chunk_after(p_to_realloc), extra);
	}
	return ptr;
    }

    /* No short cuts, allocate new memory and copy */

    mem = malloc(size);
    if (!mem)
        return NULL;

    memcpy(mem, ptr, old_size - MALLOC_HEAD);
    free(ptr);

    return mem;
}
#endif /* DEFINE_REALLOC */

#ifdef DEFINE_MALLINFO

volatile chunk_t *__malloc_block;

void
__malloc_validate_block(chunk_t *r)
{
    __malloc_block = r;
    assert (__align_up(chunk_to_ptr(r), MALLOC_CHUNK_ALIGN) == chunk_to_ptr(r));
    assert (__align_up(r, MALLOC_HEAD_ALIGN) == r);
    assert (_size(r) >= MALLOC_MINSIZE);
    assert (_size(r) < 0x80000000UL);
    assert (__align_up(_size(r), MALLOC_HEAD_ALIGN) == _size(r));
}

void
__malloc_validate(void)
{
    chunk_t *r;

    for (r = __malloc_free_list; r; r = r->next) {
	__malloc_validate_block(r);
	assert (r->next == NULL || (char *) r + _size(r) <= (char *) r->next);
    }
}

struct mallinfo mallinfo(void)
{
    char * sbrk_now;
    chunk_t *pf;
    size_t free_size = 0;
    size_t total_size;
    size_t ordblks = 0;
    struct mallinfo current_mallinfo = {};

    MALLOC_LOCK;

    __malloc_validate();

    if (__malloc_sbrk_start == NULL) total_size = 0;
    else {
        sbrk_now = __malloc_sbrk_top;

        if (sbrk_now == NULL)
            total_size = (size_t)-1;
        else
            total_size = (size_t) (sbrk_now - __malloc_sbrk_start);
    }

    for (pf = __malloc_free_list; pf; pf = pf->next) {
	ordblks++;
        free_size += _size(pf);
    }

    current_mallinfo.ordblks = ordblks;
    current_mallinfo.arena = total_size;
    current_mallinfo.fordblks = free_size;
    current_mallinfo.uordblks = total_size - free_size;

    MALLOC_UNLOCK;
    return current_mallinfo;
}
#endif /* DEFINE_MALLINFO */

#ifdef DEFINE_MALLOC_STATS

void malloc_stats(void)
{
    struct mallinfo current_mallinfo;

    current_mallinfo = mallinfo();
    fprintf(stderr, "max system bytes = %10lu\n",
            (long) current_mallinfo.arena);
    fprintf(stderr, "system bytes     = %10lu\n",
            (long) current_mallinfo.arena);
    fprintf(stderr, "in use bytes     = %10lu\n",
            (long) current_mallinfo.uordblks);
    fprintf(stderr, "free blocks      = %10lu\n",
	    (long) current_mallinfo.ordblks);
}
#endif /* DEFINE_MALLOC_STATS */

#ifdef DEFINE_MALLOC_USABLE_SIZE
size_t malloc_usable_size(void * ptr)
{
    return chunk_usable(ptr_to_chunk(ptr));
}
#endif /* DEFINE_MALLOC_USABLE_SIZE */

#ifdef DEFINE_MEMALIGN
/* Function memalign
 * Allocate memory block aligned at specific boundary.
 *   align: required alignment. Must be power of 2. Return NULL
 *          if not power of 2. Undefined behavior is bigger than
 *          pointer value range.
 *   s: required size.
 * Return: allocated memory pointer aligned to align
 * Algorithm: Malloc a big enough block, padding pointer to aligned
 *            address, then truncate and free the tail if too big.
 *            Record the offset of align pointer and original pointer
 *            in the padding area.
 */
void * memalign(size_t align, size_t s)
{
    chunk_t *chunk_p;
    size_t offset, size_with_padding;
    char * allocated, * aligned_p;

    /* Return NULL if align isn't power of 2 */
    if ((align & (align-1)) != 0)
    {
        errno = EINVAL;
        return NULL;
    }

    align = MAX(align, MALLOC_MINSIZE);

    if (s > MALLOC_MAXSIZE - align)
    {
        errno = ENOMEM;
        return NULL;
    }

    s = __align_up(MAX(s,1), MALLOC_CHUNK_ALIGN);

    /* Make sure there's space to align the allocation and split
     * off chunk_t from the front
     */
    size_with_padding = s + align + MALLOC_MINSIZE;

    allocated = __malloc_malloc(size_with_padding);
    if (allocated == NULL) return NULL;

    chunk_p = ptr_to_chunk(allocated);

    aligned_p = __align_up(allocated, align);

    offset = (size_t) (aligned_p - allocated);

    /* Split off the front piece if necessary */
    if (offset)
    {
	if (offset < MALLOC_MINSIZE) {
	    aligned_p += align;
	    offset += align;
	}

	chunk_t *new_chunk_p = ptr_to_chunk(aligned_p);
	_set_size(new_chunk_p, _size(chunk_p) - offset);

	make_free_chunk(chunk_p, offset);

	chunk_p = new_chunk_p;
    }

    offset = _size(chunk_p) - chunk_size(s);

    /* Split off the back piece if large enough */
    if (offset >= MALLOC_MINSIZE)
    {
	*_size_ref(chunk_p) -= offset;

	make_free_chunk(chunk_after(chunk_p), offset);
    }
    return aligned_p;
}
#ifdef _HAVE_ALIAS_ATTRIBUTE
__strong_reference(memalign, aligned_alloc);
#endif
#endif /* DEFINE_MEMALIGN */

#ifdef DEFINE_MALLOPT
int mallopt(int parameter_number, int parameter_value)
{
    (void) parameter_number;
    (void) parameter_value;
    return 0;
}
#endif /* DEFINE_MALLOPT */

#ifdef DEFINE_VALLOC
void * valloc(size_t s)
{
    return memalign(MALLOC_PAGE_ALIGN, s);
}
#endif /* DEFINE_VALLOC */

#ifdef DEFINE_PVALLOC
void * pvalloc(size_t s)
{
    if (s > MALLOC_MAXSIZE - MALLOC_PAGE_ALIGN)
    {
        errno = ENOMEM;
        return NULL;
    }

    return valloc(__align_up(s, MALLOC_PAGE_ALIGN));
}
#endif /* DEFINE_PVALLOC */

#ifdef DEFINE_GETPAGESIZE
int getpagesize(void)
{
    return MALLOC_PAGE_ALIGN;
}
#endif /* DEFINE_GETPAGESIZE */

#ifdef DEFINE_POSIX_MEMALIGN
int posix_memalign(void **memptr, size_t align, size_t size)
{
    /* Return EINVAL if align isn't power of 2 or not a multiple of a pointer size */
    if ((align & (align-1)) != 0 || align % sizeof(void*) != 0 || align == 0)
        return EINVAL;

    void *mem = memalign(align, size);

    if (!mem)
        return ENOMEM;

    *memptr = mem;
    return 0;
}
#endif
