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

#include "local-malloc.h"

#ifdef __MALLOC_SKIP_LIST

static inline size_t
_ms_new_level(size_t size)
{
    long   bits = random();
    size_t level = 0;

    while (!(bits & MS_LEVEL_MASK) && level < MS_MAX_LEVEL
           && malloc_size(size) >= _ms_size(level + 1)) {
        level++;
        bits >>= MS_LEVEL_BITS;
    }
    return level;
}

static void
_ms_init(chunk_t *ms)
{
    size_t level = _ms_new_level(_size(ms));
    size_t o;
    for (o = 0; o < level; o++)
        ms->next[o] = (chunk_t *)((uintptr_t)1);
    ms->next[o] = NULL;
}

/*
 * Find the position for 'template'. Return pointers to the pointer
 * at each level; this allows simple list modification for both
 * insert and delete
 */
void
_ms_find(const chunk_t *template, malloc_prev_t *prev)
{
    size_t    o;
    chunk_t  *next;
    chunk_t **p;

    p = &__malloc_skip_list.next[MS_MAX_LEVEL];

    /* For all levels */
    for (o = MS_MAX_LEVEL;; o--) {

        /* Search at this level for the insertion point */
        while ((next = _ms_ref(*p)) != NULL) {

            /* Stop when template doesn't follow next */
            if (!_ms_greater(template, next))
                break;

            p = &next->next[o];
        }

        /* Save this position */
        prev->prev[o] = p;

        /* all done ? */
        if (o == 0)
            break;

        /* Step to the previous reference */
        p--;
    }
}

/* Insert 'ms' into lists at position prev */
void
_ms_clip_in(chunk_t *ms, malloc_prev_t *prev)
{
    size_t o;

    _ms_init(ms);
    /* Insert into all levels for the new object */
    for (o = 0;; o++) {
        _ms_set_ref(&ms->next[o], _ms_ref(*prev->prev[o]));
        _ms_set_ref(prev->prev[o], ms);
        if (_ms_is_last(ms->next[o]))
            break;
    }
}

/* Remove 'ms' from lists at position prev */
void
_ms_clip_out(chunk_t *ms, malloc_prev_t *prev)
{
    size_t o;

    /* Delete from all levels for the old object */
    for (o = 0;; o++) {
        chunk_t *ref = ms->next[o];
        _ms_set_ref(prev->prev[o], _ms_ref(ref));
        if (_ms_is_last(ref))
            break;
    }
}

/* Step forward one object, updating all of the prev pointers */
void
_ms_step(chunk_t *ms, malloc_prev_t *prev)
{
    chunk_t ***p;
    chunk_t  **n;

    /*
     * Update the 'prev' pointer array to reference the current
     * object for all levels it contains.
     */
    n = ms->next;
    p = prev->prev;
    for (;;) {
        *p = n;
        if (_ms_is_last(*n))
            break;
        p++;
        n++;
    }
}

#ifdef MALLOC_SKIP_API

/* This higher level API is unused by malloc */

void
_ms_insert(chunk_t *new)
{
    malloc_prev_t prev;

    _ms_find(new, &prev);
    _ms_clip_in(new, &prev);
}

bool
_ms_delete(chunk_t *old)
{
    malloc_prev_t prev;

    _ms_find(old, &prev);

    if (_ms_this(&prev) != old)
        return false;

    _ms_clip_out(old, &prev);
    return true;
}

chunk_t *
_ms_search(chunk_t *pattern)
{
    malloc_prev_t prev;
    chunk_t      *found;

    _ms_find(pattern, &prev);

    found = _ms_this(&prev);
    if (found && _ms_greater(pattern, found))
        found = NULL;
    return found;
}
#endif

#ifdef MALLOC_DEBUG
void
_ms_dump(void)
{
    chunk_t      *ms;
    malloc_prev_t prev;
    size_t        o;

    _ms_step_init(&prev);

    printf("##########\n");
    for (_ms_step_init(&prev); (ms = _ms_this(&prev)) != NULL; _ms_step(ms, &prev)) {
        printf("%p(%6zd):", ms, _size((chunk_t *)ms));

        bool pass_through = false;

        for (o = 0; o <= MS_MAX_LEVEL; o++) {
            if (pass_through)
                printf("  |  ");
            else {
                if (_ms_ref(*prev.prev[o]) != ms)
                    printf("--?--");
                else
                    printf("--+--");
                if (_ms_is_last(ms->next[o]))
                    pass_through = true;
            }
        }
        printf("\n");
    }
    printf("##########\n");
}

void
_ms_validate(void)
{
    size_t   o;
    chunk_t *ms, *next, *down;
    size_t   prev_count = 0;
    size_t   count;

    for (o = MS_MAX_LEVEL;; o--) {

        count = 0;
        /* Make sure that this chain is in order */
        for (ms = __malloc_skip_list.next[o]; ms; ms = next) {

            count++;

            next = _ms_ref(ms->next[o]);

            assert(!next || !_ms_greater(ms, next));

            if (o != 0) {
                /* Make sure we find 'next' on the next chain down */
                for (down = ms;; down = _ms_ref(down->next[o - 1])) {
                    if (down == next)
                        break;
                    assert(down != NULL);
                }
            }
        }
        /*
         * Make sure the counts are "reasonable", increasing at about
         * 4x per level
         */
        if (count >= 32) {
            if ((count && count < prev_count * 2) || (prev_count && count > prev_count * 32)) {
                printf("level %zd: %zd level %zd %zd\n", o, count, o + 1, prev_count);
            }
        }

        prev_count = count;

        if (o == 0)
            break;
    }
}
#endif /* MALLOC_DEBUG */

#endif /* __MALLOC_SKIP_LIST */
