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
 * Implement either by merging adjacent free memory
 * or by calling malloc/memcpy
 */
void *
realloc(void * ptr, size_t size)
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
