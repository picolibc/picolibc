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

static bool
__realloc_grow_adjacent(chunk_t *p_to_realloc, size_t old_size, size_t new_size)
{
    chunk_t **p, *r, *after = chunk_after(p_to_realloc);

    /*
     * Check to see if there's a large enough chunk_t of free space
     * just past the current chunk. If we find one, merge it in
     */
    for (p = &__malloc_free_list; (r = *p) != NULL; p = &r->next) {
        if (r == after) {
            size_t r_size = _size(r);

            if (r_size >= new_size || old_size + r_size >= new_size) {
                /* remove R from the free list */
                *p = r->next;

                _set_size(p_to_realloc, old_size + r_size);
                return true;
            }
        }
        if (p_to_realloc < r)
            break;
    }
    return false;
}

/*
 * Implement either by merging adjacent free memory
 * or by calling malloc/memcpy
 */
void * __disable_sanitizer
realloc(void *ptr, size_t size)
{
    void *mem;

    if (ptr == NULL)
        return malloc(size);

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    if (size > MALLOC_ALLOC_MAX) {
        errno = ENOMEM;
        return NULL;
    }

    if (!_check_busy(ptr, "realloc: already freed\n"))
        return NULL;

    size_t   new_size = chunk_size(size);

    chunk_t *p_to_realloc = ptr_to_chunk(ptr);

#if MALLOC_DEBUG
    assert(!_is_free(p_to_realloc));
    __malloc_validate_chunk(p_to_realloc);
#endif

    size_t old_size = _size(p_to_realloc);

#if __MALLOC_SMALL_BUCKET
    bool is_bucket;
    is_bucket = (old_size <= MALLOC_MAX_BUCKET && old_size == BUCKET_SIZE(BUCKET_NUM(old_size)));
#else
#define is_bucket 0
#endif

    /* See if we can avoid allocating new memory
     * when increasing the size
     */
    if (!is_bucket && new_size > old_size) {
        MALLOC_LOCK;

        if (__malloc_grow_chunk(p_to_realloc, new_size)
            || __realloc_grow_adjacent(p_to_realloc, old_size, new_size)) {
            /* clear new memory */
            memset((char *)chunk_to_blob(p_to_realloc) + old_size, '\0', new_size - old_size);

            /* Reset chunk_t size */
            old_size = _size(p_to_realloc);
        }

        MALLOC_UNLOCK;
    }

    if (new_size <= old_size) {
        size_t rem = old_size - new_size;

#ifdef __MALLOC_CLEAR_FREED
        memset((char *)ptr + size, 0, rem);
#endif
        /* If there's enough space left over, split it out
         * and free it
         */
        if (!is_bucket && rem >= MALLOC_CHUNK_MIN) {

#if __MALLOC_SMALL_BUCKET
            /*
             * If the remainder fits a bucket, adjust to the largest
             * possible bucket and free that
             */
            if (rem <= MALLOC_MAX_BUCKET) {
                /*
                 * Adjust remainder to bucket size
                 */

                int bucket = BUCKET_FLOOR(rem);
                rem = BUCKET_SIZE(bucket);

                new_size = old_size - rem;
            }
#endif
            _set_size(p_to_realloc, new_size);
            make_free_chunk(chunk_after(p_to_realloc), rem);
        }
        return ptr;
    }
    /* No short cuts, allocate new memory and copy */

    mem = malloc(size);
    if (!mem)
        return NULL;

    memcpy(mem, ptr, malloc_size(old_size));
    free(ptr);

    return mem;
}
