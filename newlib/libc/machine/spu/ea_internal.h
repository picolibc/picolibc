/*
(C) Copyright IBM Corp. 2007, 2008

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __EA_INTERNAL_H
#define __EA_INTERNAL_H

#include <ea.h>
#define JSRE_POSIX1_SIGNALCODE	0x2101
#define SPE_POSIX1_FTOK		0x05
#define SPE_POSIX1_MMAP		0x0b
#define SPE_POSIX1_MUNMAP	0x0e
/* implemented to here */
#define SPE_POSIX1_MREMAP	0x0c
#define SPE_POSIX1_MSYNC	0x0d
#define SPE_POSIX1_SHMGET	0x14
#define SPE_POSIX1_SHMCTL	0x12
#define SPE_POSIX1_SHMAT	0x11
#define SPE_POSIX1_SHMDT	0x13
#define SPE_POSIX1_SHM_OPEN	0x15
#define SPE_POSIX1_SHM_UNLINK	0x16

#define JSRE_LIBEA_SIGNALCODE	0x2105
#define SPE_LIBEA_CALLOC	0x01
#define SPE_LIBEA_FREE		0x02
#define SPE_LIBEA_MALLOC	0x03
#define SPE_LIBEA_REALLOC	0x04
#define SPE_LIBEA_POSIX_MEMALIGN	0x05

#define PAD_INT 3
#ifdef __EA64__
#define PAD_LONG 2
#else /* 32  bit */
#define PAD_LONG 3
#endif

#define ROUND_UP_NEXT_128(x) (((x) + 128) & (~127))
#define ROUND_DOWN_128(x) ((x) & (~127))

/* Macro that generates an __ea alias.  */
#ifdef __EA64__
#define COMPAT_EA_ALIAS(name) asm (".global\t__" #name "64\n\t.set\t__" #name "64," #name)
#else
#define COMPAT_EA_ALIAS(name) asm (".global\t__" #name "32\n\t.set\t__" #name "32," #name)
#endif

static inline __ea void* round_down_128_ea(__ea void* x)
{
  size_ea_t tmp = (size_ea_t) x;
  tmp &= (~127);
  return (__ea void*)tmp;
}

static
inline __ea void* round_up_next_128_ea(__ea void* x)
{
  size_ea_t tmp = (size_ea_t) x;
  tmp += 128;
  tmp &= (~127);
  return (__ea void*)tmp;
}

#define __cache_fetch_dirty_all(x) \
		__cache_fetch_dirty(round_down_128_ea(x), 128)

/* please optimize, this hurts my eyes */
static inline size_t
three_way_min(size_t x, size_t y, size_t z)
{
  if (x < y)
    if (x < z)
      return x;
    else
      return z;
  else
    if (y < z)
      return y;
    else
      return z;
}

#undef eavoid_to_ul
#define eavoid_to_ul(X) ({ \
  unsigned long _y;                             \
  __asm__ ("# %0 %1" : "=r" (_y) : "0" (X));    \
  _y;                                           \
})

#undef eavoid_to_ull
#define eavoid_to_ull(X) ({ \
  unsigned long long _y;                        \
  __asm__ ("# %0 %1" : "=r" (_y) : "0" (X));    \
  _y;                                           \
})

#ifdef __EA32__
#undef ull_to_eavoid
#define ull_to_eavoid(X) ({ \
  __ea void* _y;  \
  unsigned long long X2;   \
  (X2) = (X) << 32;\
  __asm__ ("# %0 %1" : "=r" (_y) : "0" (X2));    \
  _y;                                           \
})
#else /*__EA64__*/
#define ull_to_eavoid(X) ({ \
  __ea void* _y;  \
  __asm__ ("# %0 %1" : "=r" (_y) : "0" (X));    \
  _y;                                           \
})
#endif

#undef ul_to_eavoid
#define ul_to_eavoid(X) ({ \
  __ea void* _y;                             \
  __asm__ ("# %0 %1" : "=r" (_y) : "0" (X));    \
  _y;                                           \
})

#endif /*__EA_INTERNAL_H*/
