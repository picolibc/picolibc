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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>

#if DEBUG
#include <assert.h>
#else
#undef assert
#define assert(x) ((void)0)
#endif

#ifndef MAX
#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#endif

#define _SBRK_R(X) _sbrk_r(X)

#ifdef _LIBC

#include <sys/config.h>
#include <reent.h>

#define RARG struct _reent *reent_ptr,
#define RONEARG struct _reent *reent_ptr
#define RCALL reent_ptr,
#define RONECALL reent_ptr

#define MALLOC_LOCK __malloc_lock(reent_ptr)
#define MALLOC_UNLOCK __malloc_unlock(reent_ptr)

#define RERRNO reent_ptr->_errno

#define nano_malloc		_malloc_r
#define nano_free		_free_r
#define nano_realloc		_realloc_r
#define nano_memalign		_memalign_r
#define nano_valloc		_valloc_r
#define nano_pvalloc		_pvalloc_r
#define nano_calloc		_calloc_r
#define nano_cfree		_cfree_r
#define nano_malloc_usable_size _malloc_usable_size_r
#define nano_malloc_stats	_malloc_stats_r
#define nano_mallinfo		_mallinfo_r
#define nano_mallopt		_mallopt_r

#else /* ! _LIBC */

#define RARG
#define RONEARG
#define RCALL
#define RONECALL
#define MALLOC_LOCK
#define MALLOC_UNLOCK
#define RERRNO errno

#define nano_malloc		malloc
#define nano_free		free
#define nano_realloc		realloc
#define nano_memalign		memalign
#define nano_valloc		valloc
#define nano_pvalloc		pvalloc
#define nano_calloc		calloc
#define nano_cfree		cfree
#define nano_malloc_usable_size malloc_usable_size
#define nano_malloc_stats	malloc_stats
#define nano_mallinfo		mallinfo
#define nano_mallopt		mallopt
#endif /* ! _LIBC */

/* Redefine names to avoid conflict with user names */
#define free_list __malloc_free_list
#define sbrk_start __malloc_sbrk_start
#define current_mallinfo __malloc_current_mallinfo

#define ALIGN_PTR(ptr, align) \
    (((ptr) + (align) - (intptr_t)1) & ~((align) - (intptr_t)1))
#define ALIGN_SIZE(size, align) \
    (((size) + (align) - (size_t)1) & ~((align) - (size_t)1))

/* Alignment of allocated block */
#define MALLOC_ALIGN (8U)
#define CHUNK_ALIGN (sizeof(void*))
#define MALLOC_PADDING ((MAX(MALLOC_ALIGN, CHUNK_ALIGN)) - CHUNK_ALIGN)

/* as well as the minimal allocation size
 * to hold a free pointer */
#define MALLOC_MINSIZE (sizeof(void *))
#define MALLOC_PAGE_ALIGN (0x1000)
#define MAX_ALLOC_SIZE (0x80000000U)

typedef size_t malloc_size_t;

typedef struct malloc_chunk
{
    /*          --------------------------------------
     *   chunk->| size                               |
     *          --------------------------------------
     *          | Padding for alignment              |
     *          | This includes padding inserted by  |
     *          | the compiler (to align fields) and |
     *          | explicit padding inserted by this  |
     *          | implementation. If any explicit    |
     *          | padding is being used then the     |
     *          | sizeof (size) bytes at             |
     *          | mem_ptr - CHUNK_OFFSET must be     |
     *          | initialized with the negative      |
     *          | offset to size.                    |
     *          --------------------------------------
     * mem_ptr->| When allocated: data               |
     *          | When freed: pointer to next free   |
     *          | chunk                              |
     *          --------------------------------------
     */
    /* size of the allocated payload area, including size before
       CHUNK_OFFSET */
    long size;

    /* since here, the memory is either the next free block, or data load */
    struct malloc_chunk * next;
}chunk;


#define CHUNK_OFFSET ((malloc_size_t)(&(((struct malloc_chunk *)0)->next)))

/* size of smallest possible chunk. A memory piece smaller than this size
 * won't be able to create a chunk */
#define MALLOC_MINCHUNK (CHUNK_OFFSET + MALLOC_PADDING + MALLOC_MINSIZE)

/* Forward data declarations */
extern chunk * free_list;
extern char * sbrk_start;
extern struct mallinfo current_mallinfo;

/* Forward function declarations */
extern void * nano_malloc(RARG malloc_size_t);
extern void nano_free (RARG void * free_p);
extern void nano_cfree(RARG void * ptr);
extern void * nano_calloc(RARG malloc_size_t n, malloc_size_t elem);
extern void nano_malloc_stats(RONEARG);
extern malloc_size_t nano_malloc_usable_size(RARG void * ptr);
extern void * nano_realloc(RARG void * ptr, malloc_size_t size);
extern void * nano_memalign(RARG size_t align, size_t s);
extern int nano_mallopt(RARG int parameter_number, int parameter_value);
extern void * nano_valloc(RARG size_t s);
extern void * nano_pvalloc(RARG size_t s);

static inline chunk * get_chunk_from_ptr(void * ptr)
{
    /* Assume that there is no explicit padding in the
       chunk, and that the chunk starts at ptr - CHUNK_OFFSET.  */
    chunk * c = (chunk *)((char *)ptr - CHUNK_OFFSET);

    /* c->size being negative indicates that there is explicit padding in
       the chunk. In which case, c->size is currently the negative offset to
       the true size.  */
    if (c->size < 0) c = (chunk *)((char *)c + c->size);
    return c;
}

#ifdef DEFINE_MALLOC
/* List list header of free blocks */
chunk * free_list = NULL;

/* Starting point of memory allocated from system */
char * sbrk_start = NULL;

/** Function sbrk_aligned
  * Algorithm:
  *   Use sbrk() to obtain more memory and ensure it is CHUNK_ALIGN aligned
  *   Optimise for the case that it is already aligned - only ask for extra
  *   padding after we know we need it
  */
static void* sbrk_aligned(RARG malloc_size_t s)
{
    char *p, *align_p;

    if (sbrk_start == NULL) sbrk_start = _SBRK_R(RCALL 0);

    p = _SBRK_R(RCALL s);

    /* sbrk returns -1 if fail to allocate */
    if (p == (void *)-1)
        return p;

    align_p = (char*)ALIGN_PTR((uintptr_t)p, CHUNK_ALIGN);
    if (align_p != p)
    {
        /* p is not aligned, ask for a few more bytes so that we have s
         * bytes reserved from align_p. */
        p = _SBRK_R(RCALL align_p - p);
        if (p == (void *)-1)
            return p;
    }
    return align_p;
}

/** Function nano_malloc
  * Algorithm:
  *   Walk through the free list to find the first match. If fails to find
  *   one, call sbrk to allocate a new chunk.
  */
void * nano_malloc(RARG malloc_size_t s)
{
    chunk *p, *r;
    char * ptr, * align_ptr;
    int offset;

    malloc_size_t alloc_size;

    alloc_size = ALIGN_SIZE(s, CHUNK_ALIGN); /* size of aligned data load */
    alloc_size += MALLOC_PADDING; /* padding */
    alloc_size += CHUNK_OFFSET; /* size of chunk head */
    alloc_size = MAX(alloc_size, MALLOC_MINCHUNK);

    if (alloc_size >= MAX_ALLOC_SIZE || alloc_size < s)
    {
        RERRNO = ENOMEM;
        return NULL;
    }

    MALLOC_LOCK;

    p = free_list;
    r = p;

    while (r)
    {
        int rem = r->size - alloc_size;
        if (rem >= 0)
        {
            if (rem >= MALLOC_MINCHUNK)
            {
                if (p == r)
                {
                    /* First item in the list, break it into two chunks
                    *  and return the first one */
                    r->size = alloc_size;
                    free_list = (chunk *)((char *)r + alloc_size);
                    free_list->size = rem;
                    free_list->next = r->next;
                } else {
                    /* Any other item in the list. Split and return
                    * the first one */
                    r->size = alloc_size;
                    p->next = (chunk *)((char *)r + alloc_size);
                    p->next->size = rem;
                    p->next->next = r->next;
                }
            }
            /* Find a chunk that is exactly the size or slightly bigger
             * than requested size, just return this chunk */
            else if (p == r)
            {
                /* Now it implies p==r==free_list. Move the free_list
                 * to next chunk */
                free_list = r->next;
            }
            else
            {
                /* Normal case. Remove it from free_list */
                p->next = r->next;
            }
            break;
        }
        p=r;
        r=r->next;
    }

    /* Failed to find a appropriate chunk. Ask for more memory */
    if (r == NULL)
    {
        r = sbrk_aligned(RCALL alloc_size);

        /* sbrk returns -1 if fail to allocate */
        if (r == (void *)-1)
        {
            /* sbrk didn't have the requested amount. Let's check
             * if the last item in the free list is adjacent to the
             * current heap end (sbrk(0)). In that case, only ask
             * for the difference in size and merge them */
            p = free_list;
            r = p;

            while (r)
            {
                p=r;
                r=r->next;
            }

            if (p != NULL && (char *)p + p->size == (char *)_SBRK_R(RCALL 0))
            {
               /* The last free item has the heap end as neighbour.
                * Let's ask for a smaller amount and merge */
               alloc_size -= p->size;
               alloc_size = ALIGN_SIZE(alloc_size, CHUNK_ALIGN); /* size of aligned data load */
               alloc_size += MALLOC_PADDING; /* padding */
               alloc_size += CHUNK_OFFSET; /* size of chunk head */
               alloc_size = MAX(alloc_size, MALLOC_MINCHUNK);

               if (sbrk_aligned(RCALL alloc_size) != (void *)-1)
               {
                   p->size += alloc_size;
                   r = p;
               }
               else
               {
                   RERRNO = ENOMEM;
                   MALLOC_UNLOCK;
                   return NULL;
               }
            }
            else
            {
                RERRNO = ENOMEM;
                MALLOC_UNLOCK;
                return NULL;
            }
        }
        else
        {
            r->size = alloc_size;
        }
    }
    MALLOC_UNLOCK;

    ptr = (char *)r + CHUNK_OFFSET;

    align_ptr = (char *)ALIGN_PTR((uintptr_t)ptr, MALLOC_ALIGN);
    offset = align_ptr - ptr;

    if (offset)
    {
        /* Initialize sizeof (malloc_chunk.size) bytes at
           align_ptr - CHUNK_OFFSET with negative offset to the
           size field (at the start of the chunk).

           The negative offset to size from align_ptr - CHUNK_OFFSET is
           the size of any remaining padding minus CHUNK_OFFSET.  This is
           equivalent to the total size of the padding, because the size of
           any remaining padding is the total size of the padding minus
           CHUNK_OFFSET.

           Note that the size of the padding must be at least CHUNK_OFFSET.

           The rest of the padding is not initialized.  */
        *(long *)((char *)r + offset) = -offset;
    }

    assert(align_ptr + size <= (char *)r + alloc_size);
    return align_ptr;
}
#endif /* DEFINE_MALLOC */

#ifdef DEFINE_FREE
#define MALLOC_CHECK_DOUBLE_FREE

/** Function nano_free
  * Implementation of libc free.
  * Algorithm:
  *  Maintain a global free chunk single link list, headed by global
  *  variable free_list.
  *  When free, insert the to-be-freed chunk into free list. The place to
  *  insert should make sure all chunks are sorted by address from low to
  *  high.  Then merge with neighbor chunks if adjacent.
  */
void nano_free (RARG void * free_p)
{
    chunk * p_to_free;
    chunk * p, * q;

    if (free_p == NULL) return;

    p_to_free = get_chunk_from_ptr(free_p);

    MALLOC_LOCK;
    if (free_list == NULL)
    {
        /* Set first free list element */
        p_to_free->next = free_list;
        free_list = p_to_free;
        MALLOC_UNLOCK;
        return;
    }

    if (p_to_free < free_list)
    {
        if ((char *)p_to_free + p_to_free->size == (char *)free_list)
        {
            /* Chunk to free is just before the first element of
             * free list  */
            p_to_free->size += free_list->size;
            p_to_free->next = free_list->next;
        }
        else
        {
            /* Insert before current free_list */
            p_to_free->next = free_list;
        }
        free_list = p_to_free;
        MALLOC_UNLOCK;
        return;
    }

    q = free_list;
    /* Walk through the free list to find the place for insert. */
    do
    {
        p = q;
        q = q->next;
    } while (q && q <= p_to_free);

    /* Now p <= p_to_free and either q == NULL or q > p_to_free
     * Try to merge with chunks immediately before/after it. */

    if ((char *)p + p->size == (char *)p_to_free)
    {
        /* Chunk to be freed is adjacent
         * to a free chunk before it */
        p->size += p_to_free->size;
        /* If the merged chunk is also adjacent
         * to the chunk after it, merge again */
        if ((char *)p + p->size == (char *) q)
        {
            p->size += q->size;
            p->next = q->next;
        }
    }
#ifdef MALLOC_CHECK_DOUBLE_FREE
    else if ((char *)p + p->size > (char *)p_to_free)
    {
        /* Report double free fault */
        RERRNO = ENOMEM;
        MALLOC_UNLOCK;
        return;
    }
#endif
    else if ((char *)p_to_free + p_to_free->size == (char *) q)
    {
        /* Chunk to be freed is adjacent
         * to a free chunk after it */
        p_to_free->size += q->size;
        p_to_free->next = q->next;
        p->next = p_to_free;
    }
    else
    {
        /* Not adjacent to any chunk. Just insert it. Resulting
         * a fragment. */
        p_to_free->next = q;
        p->next = p_to_free;
    }
    MALLOC_UNLOCK;
}
#endif /* DEFINE_FREE */

#ifdef DEFINE_CFREE
void nano_cfree(RARG void * ptr)
{
    nano_free(RCALL ptr);
}
#endif /* DEFINE_CFREE */

#ifdef DEFINE_CALLOC
/* Function nano_calloc
 * Implement calloc simply by calling malloc and set zero */
void * nano_calloc(RARG malloc_size_t n, malloc_size_t elem)
{
    malloc_size_t bytes;
    void * mem;

    if (__builtin_mul_overflow (n, elem, &bytes))
    {
        RERRNO = ENOMEM;
        return NULL;
    }
    mem = nano_malloc(RCALL bytes);
    if (mem != NULL) memset(mem, 0, bytes);
    return mem;
}
#endif /* DEFINE_CALLOC */

#ifdef DEFINE_REALLOC
/* Function nano_realloc
 * Implement realloc by malloc + memcpy */
void * nano_realloc(RARG void * ptr, malloc_size_t size)
{
    void * mem;
    chunk * p_to_realloc;
    malloc_size_t old_size;

    if (ptr == NULL) return nano_malloc(RCALL size);

    if (size == 0)
    {
        nano_free(RCALL ptr);
        return NULL;
    }

    old_size = nano_malloc_usable_size(RCALL ptr);
    if (size <= old_size && (old_size >> 1) < size)
      return ptr;

    mem = nano_malloc(RCALL size);
    if (mem != NULL)
    {
	if (old_size > size)
	    old_size = size;
        memcpy(mem, ptr, old_size);
        nano_free(RCALL ptr);
    }
    return mem;
}
#endif /* DEFINE_REALLOC */

#ifdef DEFINE_MALLINFO
struct mallinfo current_mallinfo={0,0,0,0,0,0,0,0,0,0};

struct mallinfo nano_mallinfo(RONEARG)
{
    char * sbrk_now;
    chunk * pf;
    size_t free_size = 0;
    size_t total_size;

    MALLOC_LOCK;

    if (sbrk_start == NULL) total_size = 0;
    else {
        sbrk_now = _SBRK_R(RCALL 0);

        if (sbrk_now == (void *)-1)
            total_size = (size_t)-1;
        else
            total_size = (size_t) (sbrk_now - sbrk_start);
    }

    for (pf = free_list; pf; pf = pf->next)
        free_size += pf->size;

    current_mallinfo.arena = total_size;
    current_mallinfo.fordblks = free_size;
    current_mallinfo.uordblks = total_size - free_size;

    MALLOC_UNLOCK;
    return current_mallinfo;
}
#endif /* DEFINE_MALLINFO */

#ifdef DEFINE_MALLOC_STATS
void nano_malloc_stats(RONEARG)
{
    nano_mallinfo(RONECALL);
    fiprintf(stderr, "max system bytes = %10u\n",
             current_mallinfo.arena);
    fiprintf(stderr, "system bytes     = %10u\n",
             current_mallinfo.arena);
    fiprintf(stderr, "in use bytes     = %10u\n",
             current_mallinfo.uordblks);
}
#endif /* DEFINE_MALLOC_STATS */

#ifdef DEFINE_MALLOC_USABLE_SIZE
malloc_size_t nano_malloc_usable_size(RARG void * ptr)
{
    chunk * c = (chunk *)((char *)ptr - CHUNK_OFFSET);
    int size_or_offset = c->size;

    if (size_or_offset < 0)
    {
        /* Padding is used. Excluding the padding size */
        c = (chunk *)((char *)c + c->size);
        return c->size - CHUNK_OFFSET + size_or_offset;
    }
    return c->size - CHUNK_OFFSET;
}
#endif /* DEFINE_MALLOC_USABLE_SIZE */

#ifdef DEFINE_MEMALIGN
/* Function nano_memalign
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
void * nano_memalign(RARG size_t align, size_t s)
{
    chunk * chunk_p;
    malloc_size_t size_allocated, offset, ma_size, size_with_padding;
    char * allocated, * aligned_p;

    /* Return NULL if align isn't power of 2 */
    if ((align & (align-1)) != 0) return NULL;

    align = MAX(align, MALLOC_ALIGN);

    /* Make sure ma_size does not overflow */
    if (s > __SIZE_MAX__ - CHUNK_ALIGN)
    {
	RERRNO = ENOMEM;
	return NULL;
    }
    ma_size = ALIGN_SIZE(MAX(s, MALLOC_MINSIZE), CHUNK_ALIGN);

    /* Make sure size_with_padding does not overflow */
    if (ma_size > __SIZE_MAX__ - (align - MALLOC_ALIGN))
    {
	RERRNO = ENOMEM;
	return NULL;
    }
    size_with_padding = ma_size + (align - MALLOC_ALIGN);

    allocated = nano_malloc(RCALL size_with_padding);
    if (allocated == NULL) return NULL;

    chunk_p = get_chunk_from_ptr(allocated);
    aligned_p = (char *)ALIGN_PTR(
                  (uintptr_t)((char *)chunk_p + CHUNK_OFFSET),
                  (uintptr_t)align);
    offset = aligned_p - ((char *)chunk_p + CHUNK_OFFSET);

    if (offset)
    {
        if (offset >= MALLOC_MINCHUNK)
        {
            /* Padding is too large, free it */
            chunk * front_chunk = chunk_p;
            chunk_p = (chunk *)((char *)chunk_p + offset);
            chunk_p->size = front_chunk->size - offset;
            front_chunk->size = offset;
            nano_free(RCALL (char *)front_chunk + CHUNK_OFFSET);
        }
        else
        {
            /* Padding is used. Need to set a jump offset for aligned pointer
            * to get back to chunk head */
            assert(offset >= sizeof(int));
            *(long *)((char *)chunk_p + offset) = -offset;
        }
    }

    size_allocated = chunk_p->size;
    if ((char *)chunk_p + size_allocated >
         (aligned_p + ma_size + MALLOC_MINCHUNK))
    {
        /* allocated much more than what's required for padding, free
         * tail part */
        chunk * tail_chunk = (chunk *)(aligned_p + ma_size);
        chunk_p->size = aligned_p + ma_size - (char *)chunk_p;
        tail_chunk->size = size_allocated - chunk_p->size;
        nano_free(RCALL (char *)tail_chunk + CHUNK_OFFSET);
    }
    return aligned_p;
}
#endif /* DEFINE_MEMALIGN */

#ifdef DEFINE_MALLOPT
int nano_mallopt(RARG int parameter_number, int parameter_value)
{
    return 0;
}
#endif /* DEFINE_MALLOPT */

#ifdef DEFINE_VALLOC
void * nano_valloc(RARG size_t s)
{
    return nano_memalign(RCALL MALLOC_PAGE_ALIGN, s);
}
#endif /* DEFINE_VALLOC */

#ifdef DEFINE_PVALLOC
void * nano_pvalloc(RARG size_t s)
{
    /* Make sure size given to nano_valloc does not overflow */
    if (s > __SIZE_MAX__ - MALLOC_PAGE_ALIGN)
    {
	RERRNO = ENOMEM;
	return NULL;
    }
    return nano_valloc(RCALL ALIGN_SIZE(s, MALLOC_PAGE_ALIGN));
}
#endif /* DEFINE_PVALLOC */
