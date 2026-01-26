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

#define _GNU_SOURCE
#include "local-malloc.h"

/*
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

void *
memalign(size_t align, size_t s)
{
    chunk_t *chunk_p;
    size_t   offset, size_with_padding;
    char    *allocated, *aligned_p;

    /* Return NULL if align isn't power of 2 */
    if ((align & (align - 1)) != 0) {
        errno = EINVAL;
        return NULL;
    }

    align = MAX(align, MALLOC_MINSIZE);

    if (s > MALLOC_MAXSIZE - align) {
        errno = ENOMEM;
        return NULL;
    }

    s = __align_up(MAX(s, 1), MALLOC_CHUNK_ALIGN);

    /* Make sure there's space to align the allocation and split
     * off chunk_t from the front
     */
    size_with_padding = s + align + MALLOC_MINSIZE;

    allocated = __malloc_malloc(size_with_padding);
    if (allocated == NULL)
        return NULL;

    chunk_p = ptr_to_chunk(allocated);

    aligned_p = __align_up(allocated, align);

    offset = (size_t)(aligned_p - allocated);

    /* Split off the front piece if necessary */
    if (offset) {
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
    if (offset >= MALLOC_MINSIZE) {
        *_size_ref(chunk_p) -= offset;

        make_free_chunk(chunk_after(chunk_p), offset);
    }
    return aligned_p;
}

#ifdef __strong_reference
__strong_reference(memalign, aligned_alloc);
#endif
