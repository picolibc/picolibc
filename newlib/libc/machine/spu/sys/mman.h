/*
(C) Copyright IBM Corp. 2007

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

#ifndef _MMAN_H_
#define _MMAN_H_

#include <sys/types.h>

/*
 * Prots to 'mmap'.
 */
#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define PROT_EXEC       0x4
#define PROT_NONE       0x0
/*
 * Flags to 'mmap'.
 */
#define MAP_SHARED      0x001
#define MAP_PRIVATE     0x002
#define MAP_FIXED       0x010
#define MAP_FILE        0x000
#define MAP_ANONYMOUS   0x020
#define MAP_ANON        MAP_ANONYMOUS
#define MAP_GROWSDOWN   0x0100
#define MAP_DENYWRITE   0x0800
#define MAP_EXECUTABLE  0x1000
#define MAP_LOCKED      0x0080
#define MAP_NORESERVE   0x0040
/*
 * Failed flag from 'mmap'.
 */
#define MAP_FAILED_EADDR  ((unsigned long long) (-1LL))
/*
 * Flags to 'mremap'.
 */
#define MREMAP_MAYMOVE  1
/*
 * Flags to 'msync'.
 */
#define MS_ASYNC        1
#define MS_SYNC         4
#define MS_INVALIDATE   2


extern int shm_open(const char *name, int oflag, mode_t mode);
extern int shm_unlink(const char * name);

unsigned long long mmap_eaddr(unsigned long long start, size_t length, int
                              prot, int flags, int fd, off_t offset);
unsigned long long mremap_eaddr(unsigned long long old_addr, size_t
                                old_size, size_t new_size, int flags);
unsigned long long msync_eaddr(unsigned long long start, size_t length,
                               int flags);
unsigned long long munmap_eaddr(unsigned long long start, size_t length);

#endif /* _MMAN_H_ */
