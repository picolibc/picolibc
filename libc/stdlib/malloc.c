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

#include "local-malloc.h"

/* List list header of free blocks */
chunk_t *__malloc_free_list;

#ifdef MALLOC_MAX_BUCKET
chunk_t *__malloc_bucket_list[NUM_BUCKET_POT];
#endif

/* Starting point of memory allocated from system */
char *__malloc_sbrk_start;
char *__malloc_sbrk_top;

/*
 * Algorithm:
 *   Use sbrk() to obtain more memory and ensure the storage is
 *   MALLOC_CHUNK_ALIGN aligned. Optimise for the case that it is
 *   already aligned - only ask for extra padding after we know we
 *   need it
 */
static void *
__malloc_sbrk_aligned(size_t s)
{
    char *p, *align_p;

#ifdef __APPLE__
    /* Mac OS X 'emulates' sbrk, but the
     * parameter is int, not intptr_t or ptrdiff_t,
     */
    int d = (int)s;
    if (d < 0 || (size_t)d != s)
        return (void *)-1;
#else
    intptr_t d = (intptr_t)s;

    if (d < 0)
        return (void *)-1;
#endif
    p = sbrk(d);

    /* sbrk returns -1 if fail to allocate */
    if (p == (void *)-1)
        return p;

    __malloc_sbrk_top = (char *)((uintptr_t)p + s);

    /* Adjust returned space so that the storage area
     * is MALLOC_CHUNK_ALIGN aligned and the head is
     * MALLOC_HEAD_ALIGN aligned.
     */
    align_p = (char *)(__align_up((uintptr_t)p + MALLOC_HEAD, MALLOC_CHUNK_ALIGN) - MALLOC_HEAD);

    if (align_p != p) {
        /* p is not aligned, ask for a few more bytes so that we have
         * s bytes reserved from align_p. This should only occur for
         * the first sbrk in a chunk of memory as all others should be
         * aligned to the right value as chunk sizes are selected to
         * make them abut in memory
         */
        intptr_t adjust = align_p - p;
        char    *extra = sbrk(adjust);
        if (extra != (char *)((uintptr_t)p + s))
            return (void *)-1;
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
    char  *heap = __malloc_sbrk_aligned(add_size);

    /* Check if we got what we wanted */
    if (heap == chunk_e) {
        /* Set size and return */
        *_size_ref(c) += add_size;
        return true;
    }

    if (heap != (char *)-1) {
        /* sbrk returned unexpected memory, free it */
        make_free_chunk((chunk_t *)(heap + MALLOC_HEAD), add_size);
    }
    return false;
}

/** Function malloc
 * Algorithm:
 *   Walk through the free list to find the first match. If fails to find
 *   one, call sbrk to allocate a new chunk_t.
 */
void *
malloc(size_t s)
{
    chunk_t **p, *r;
    char     *ptr;
    size_t    alloc_size;

    if (s > MALLOC_MAXSIZE) {
        errno = ENOMEM;
        return NULL;
    }

    MALLOC_LOCK;

#ifdef MALLOC_MAX_BUCKET
    /* Small allocations use the bucket allocator */
    if (s <= MALLOC_MAX_BUCKET) {
        int bucket = BUCKET_NUM(s);

        alloc_size = chunk_size(BUCKET_SIZE(bucket));
        p = &__malloc_bucket_list[bucket];
        if ((r = *p) != NULL)
            *p = r->next;
    } else
#endif
    {
        alloc_size = chunk_size(s);

        for (p = &__malloc_free_list; (r = *p) != NULL; p = &r->next) {
            if (_size(r) >= alloc_size) {
                size_t rem = _size(r) - alloc_size;

                if (rem >= MALLOC_MINSIZE) {
                    /* Find a chunk_t that much larger than required size, break
                     * it into two chunks and return the first one
                     */

                    chunk_t *s = (chunk_t *)((char *)r + alloc_size);
                    _set_size(s, rem);
                    s->next = r->next;
                    *p = s;

                    _set_size(r, alloc_size);
                } else {
                    /* Find a chunk_t that is exactly the size or slightly bigger
                     * than requested size, just return this chunk_t
                     */
                    *p = r->next;
                }
                break;
            }
            if (!r->next && __malloc_grow_chunk(r, alloc_size)) {
                /* Grow the last chunk in memory to the requested size,
                 * just return it
                 */
                *p = r->next;
                break;
            }
        }
    }

    /* Failed to find a appropriate chunk_t. Ask for more memory */
    if (r == NULL) {
        void *blob = __malloc_sbrk_aligned(alloc_size);

        /* sbrk returns -1 if fail to allocate */
        if (blob == (void *)-1) {
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

#ifdef __strong_reference
#if defined(__GNUCLIKE_PRAGMA_DIAGNOSTIC) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(malloc, __malloc_malloc);
#endif

#if MALLOC_DEBUG

#include <assert.h>

void
__malloc_validate_block(chunk_t *r)
{
    assert(__align_up(chunk_to_ptr(r), MALLOC_CHUNK_ALIGN) == chunk_to_ptr(r));
    assert(__align_up(r, MALLOC_HEAD_ALIGN) == r);
    assert(_size(r) >= MALLOC_MINSIZE);
    assert(_size(r) < 0x80000000UL);
    assert(__align_up(_size(r), MALLOC_HEAD_ALIGN) == _size(r));
}

void
__malloc_validate(void)
{
    chunk_t *r;

    for (r = __malloc_free_list; r; r = r->next) {
        __malloc_validate_block(r);
        assert(r->next == NULL || (char *)r + _size(r) <= (char *)r->next);
    }
}

#endif
