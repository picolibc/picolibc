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

struct mallinfo
mallinfo(void)
{
    char * sbrk_now;
    chunk_t *pf;
    size_t free_size = 0;
    size_t total_size;
    size_t ordblks = 0;
    struct mallinfo current_mallinfo = {};

    MALLOC_LOCK;

    if (__malloc_sbrk_start == NULL) total_size = 0;
    else {
        sbrk_now = __malloc_sbrk_top;

        if (sbrk_now == NULL)
            total_size = (size_t)-1;
        else
            total_size = (size_t) (sbrk_now - __malloc_sbrk_start);
    }

    for (pf = __malloc_free_list; pf; pf = pf->next) {
	ordblks++;
        free_size += _size(pf);
    }

    current_mallinfo.ordblks = ordblks;
    current_mallinfo.arena = total_size;
    current_mallinfo.fordblks = free_size;
    current_mallinfo.uordblks = total_size - free_size;

    MALLOC_UNLOCK;

    return current_mallinfo;
}
