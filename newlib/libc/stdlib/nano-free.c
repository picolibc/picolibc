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

#include "nano-malloc.h"

/*
 * Algorithm:
 *  Maintain a global free chunk_t single link list, headed by global
 *  variable __malloc_free_list.
 *  When free, insert the to-be-freed chunk_t into free list. The place to
 *  insert should make sure all chunks are sorted by address from low to
 *  high.  Then merge with neighbor chunks if adjacent.
 */

void
free (void * free_p)
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
	if (p_to_free <= r) {

            /* Check for double free */
            if (p_to_free == r)
            {
                errno = ENOMEM;
                goto unlock;
            }

	    break;
        }

	/* Merge blocks together */
	if (chunk_after(r) == p_to_free)
	{
	    *_size_ref(r) += _size(p_to_free);
	    p_to_free = r;
	    r = r->next;
	    goto no_insert;
	}

    }

    p_to_free->next = r;
    *p = p_to_free;

no_insert:

    /* Merge blocks together */
    if (chunk_after(p_to_free) == r)
    {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wanalyzer-null-dereference"
#endif
	*_size_ref(p_to_free) += _size(r);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
	p_to_free->next = r->next;
    }

unlock:
    MALLOC_UNLOCK;
}

#ifdef _HAVE_ALIAS_ATTRIBUTE
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(free, __malloc_free);
__strong_reference(free, cfree);
#else
void cfree(void * ptr)
{
    free(ptr);
}
#endif
