/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MALLOC_SKIP_H_
#define _MALLOC_SKIP_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MS_LEVEL_BITS 2
#define MS_LEVEL_MASK ((1 << MS_LEVEL_BITS) - 1)
#define MS_MAX_LEVEL  (30 / MS_LEVEL_BITS)

typedef struct {
    chunk_t *next[MS_MAX_LEVEL + 1];
} malloc_head_t;

typedef struct {
    chunk_t **prev[MS_MAX_LEVEL + 1];
} malloc_prev_t;

extern malloc_head_t __malloc_skip_list;

static inline size_t
_ms_size(size_t level)
{
    return (level + 1) * sizeof(chunk_t *);
}

/* Mask off the low bit from a pointer */
static inline chunk_t *
_ms_ref(chunk_t *ptr)
{
    return (chunk_t *)((uintptr_t)ptr & ~1);
}

static inline bool
_ms_greater(const chunk_t *a, const chunk_t *b)
{
    return (uintptr_t)a > (uintptr_t)b;
}

/* Store a pointer while preserving the low bit in the destination */
static inline void
_ms_set_ref(chunk_t **ref, chunk_t *ptr)
{
    *ref = (chunk_t *)(((uintptr_t)*ref & 1) | (uintptr_t)ptr);
}

static inline chunk_t *
_ms_next(chunk_t *ms)
{
    return _ms_ref(ms->next[0]);
}

static inline bool
_ms_is_last(chunk_t *ms)
{
    return ((uintptr_t)ms & 1) == 0;
}

static inline void
_ms_step_init(malloc_prev_t *prev)
{
    size_t o;

    for (o = 0; o <= MS_MAX_LEVEL; o++)
        prev->prev[o] = &__malloc_skip_list.next[o];
}

static inline chunk_t *
_ms_this(malloc_prev_t *prev)
{
    return _ms_ref(*prev->prev[0]);
}

void _ms_clip_in(chunk_t *ms, malloc_prev_t *prev);
void _ms_clip_out(chunk_t *ms, malloc_prev_t *prev);

void _ms_find(const chunk_t *template, malloc_prev_t *prev);
void _ms_step(chunk_t *ms, malloc_prev_t *prev);

#ifdef MALLOC_SKIP_API
/* This higher level API is unused by malloc */
void     _ms_insert(chunk_t *new);
bool     _ms_delete(chunk_t *old);
chunk_t *_ms_search(chunk_t *pattern);
#endif

#ifdef MALLOC_DEBUG
void _ms_validate(void);
void _ms_dump(void);
#else
static inline void
_ms_validate(void)
{
}
static inline void
_ms_dump(void)
{
}
#endif

#endif /* _MALLOC_SKIP_H_ */
