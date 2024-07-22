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

/* Implementation of <<malloc>> <<free>> <<calloc>> <<realloc>>
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
