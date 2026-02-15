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

#if __MALLOC_SMALL_BUCKET
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
    chunk_t *c = (chunk_t *)__align_up((uintptr_t)p + MALLOC_HEAD_SIZE, MALLOC_CHUNK_ALIGN);

    /* And convert back to where the head will be */
    align_p = chunk_to_blob(c);

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
    size_t add_size = MAX(MALLOC_CHUNK_MIN, new_size - _size(c));

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
        make_free_chunk(blob_to_chunk(heap), add_size);
    }
    return false;
}

/** Function malloc
 * Algorithm:
 *   Walk through the free list to find the first match. If fails to find
 *   one, call sbrk to allocate a new chunk_t.
 */
void * __disable_sanitizer
malloc(size_t s)
{
    chunk_t **p, *c;
    char     *ptr;
    size_t    alloc_size;

    if (s > MALLOC_ALLOC_MAX) {
        errno = ENOMEM;
        return NULL;
    }

    alloc_size = chunk_size(s);

    MALLOC_LOCK;

#if __MALLOC_SMALL_BUCKET
    /* Small allocations use the bucket allocator */
    if (alloc_size <= MALLOC_MAX_BUCKET) {
        int bucket = BUCKET_NUM(alloc_size);

        alloc_size = BUCKET_SIZE(bucket);
        p = &__malloc_bucket_list[bucket];
        if ((c = *p) != NULL)
            *p = c->next;
    } else
#endif
    {
        for (p = &__malloc_free_list; (c = *p) != NULL; p = &c->next) {
            if (_size(c) >= alloc_size) {
                size_t rem = _size(c) - alloc_size;

                if (rem >= MALLOC_CHUNK_MIN) {
                    /* Find a chunk_t that much larger than required size, break
                     * it into two chunks and return the first one
                     */

                    chunk_t *s = (chunk_t *)((char *)c + alloc_size);
                    _set_size(c, alloc_size);
                    _set_size(s, rem);

#if __MALLOC_SMALL_BUCKET
                    /*
                     * If the remainder fits a bucket, link it there
                     * rather than into the general list
                     */
                    if (rem <= MALLOC_MAX_BUCKET) {
                        int    bucket = BUCKET_NUM(rem);
                        size_t bucket_size = BUCKET_SIZE(bucket);
                        if (rem == bucket_size) {
                            s->next = __malloc_bucket_list[bucket];
                            __malloc_bucket_list[bucket] = s;

                            /* unlink from the general list */
                            *p = c->next;
                            break;
                        }
                    }
#endif
                    s->next = c->next;
                    *p = s;
                } else {
                    /* Find a chunk_t that is exactly the size or slightly bigger
                     * than requested size, just return this chunk_t
                     */
                    *p = c->next;
                }
                break;
            }
            if (!c->next && __malloc_grow_chunk(c, alloc_size)) {
                /* Grow the last chunk in memory to the requested size,
                 * just return it
                 */
                *p = c->next;
                break;
            }
        }
    }

    /* Failed to find a appropriate chunk_t. Ask for more memory */
    if (c == NULL) {
        void *blob = __malloc_sbrk_aligned(alloc_size);

        /* sbrk returns -1 if fail to allocate */
        if (blob == (void *)-1) {
            errno = ENOMEM;
            MALLOC_UNLOCK;
            return NULL;
        }
        c = blob_to_chunk(blob);
        _set_size(c, alloc_size);
    }

    MALLOC_UNLOCK;

    ptr = chunk_to_ptr(c);

    memset(ptr, '\0', alloc_size - MALLOC_HEAD_SIZE);

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
__malloc_validate_chunk(chunk_t *c)
{
    assert(__align_up(chunk_to_ptr(c), MALLOC_CHUNK_ALIGN) == chunk_to_ptr(c));
    assert(__align_up(c, MALLOC_HEAD_ALIGN) == c);
    assert(_size(c) >= MALLOC_CHUNK_MIN);
    assert(_size(c) < 0x80000000UL);
    assert(__align_up(_size(c), MALLOC_HEAD_ALIGN) == _size(c));
}

void
__malloc_validate(void)
{
    chunk_t *c;

    for (c = __malloc_free_list; c; c = c->next) {
        __malloc_validate_chunk(c);
#if __MALLOC_SMALL_BUCKET
        size_t s = _size(c);
        size_t max_bucket = MALLOC_MAX_BUCKET;
        int    bucket = BUCKET_NUM(s);
        size_t bucket_size = BUCKET_SIZE(bucket);
        assert(s > max_bucket || s != bucket_size);
#endif
        assert(c->next == NULL || chunk_after(c) <= c->next);
    }
#if __MALLOC_SMALL_BUCKET
    size_t b;

    for (b = 0; b < NUM_BUCKET_POT; b++) {
        for (c = __malloc_bucket_list[b]; c; c = c->next) {
            __malloc_validate_chunk(c);
            assert(_size(c) == BUCKET_SIZE(b));
        }
    }
#endif
}

#endif
