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

#ifndef __EA_H
#define __EA_H

#include <stdint.h>
#include <stddef.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/unistd.h>

/*
 * take this out when compiler support is common
 */
#if (!defined( __EA32__ ) && !defined( __EA64__ ))
#warning  "need __ea support in compiler to compile to take advantage of libea features"
#define __ea
#define __EA32__
#endif

#ifdef __EA64__

typedef uint64_t size_ea_t;
typedef int64_t ssize_ea_t;
typedef uint64_t key_ea_t;
#define MAP_FAILED_EA ((__ea void *) -1LL)

#else

typedef uint32_t size_ea_t;
typedef int32_t ssize_ea_t;
typedef uint32_t key_ea_t;
#define MAP_FAILED_EA ((__ea void *) -1)

#endif

typedef __ea void * eaptr;
struct iovec_ea
{
#ifdef __EA32__
  unsigned int __pad1;   /* 32 bit padding */
#endif
  /*__ea void *iov_base;*/
  eaptr iov_base; /* Starting address */
#ifdef __EA32__
  unsigned int __pad2;   /* 32 bit padding */
#endif
  size_ea_t iov_len;   /* Number of bytes */
};


/* Memory Mapping functions */
__ea void *mmap_ea (__ea void *start, size_ea_t length, int prot, int
                     flags, int fd, off_t offset);
int munmap_ea (__ea void *start, size_ea_t length);
__ea void *mremap_ea (__ea void *old_address, size_ea_t old_size,
                       size_ea_t new_size, unsigned long flags);
int msync_ea (__ea void *start, size_ea_t length, int flags);

/* EA memory mangement functions */
__ea void *calloc_ea (size_ea_t nmemb, size_ea_t length);
void free_ea (__ea void *ptr);
__ea void *malloc_ea (size_ea_t size);
__ea void *realloc_ea (__ea void *ptr, size_ea_t size);
int posix_memalign_ea (__ea void **memptr, size_ea_t alignment, size_ea_t size);

/* String copying functions */
__ea void *memcpy_ea (__ea void *dest, __ea const void *src, size_ea_t n);
__ea void *memmove_ea (__ea void *dest, __ea const void *src, size_ea_t n);
__ea char *strcpy_ea (__ea char *dest, __ea const char *src);
__ea char *strncpy_ea (__ea char *dest, __ea const char *src, size_ea_t n);

/* Concatenation functions */
__ea char *strcat_ea (__ea char *dest, __ea const char *src);
__ea char *strncat_ea (__ea char *dest, __ea const char *src, size_ea_t n);

/* Comparison functions */
int memcmp_ea (__ea void *s1, __ea const void *s2, size_ea_t n);
int strcmp_ea (__ea char *s1, __ea const char *s2);
int strncmp_ea (__ea void *s1, __ea const void *s2, size_ea_t n3);

/* Search functions*/
__ea void *memchr_ea (__ea const void *s, int c, size_ea_t n);
__ea char *strchr_ea (__ea const char *s, int c);
size_ea_t strcspn_ea (__ea const char *s, const char *reject);
__ea char *strpbrk_ea (__ea const char *s, const char *accept);
__ea char *strrchr_ea (__ea const char *s, int c);
size_ea_t strspn_ea (__ea const char *s, const char *accept);
__ea char * strstr_ea (__ea const char *s1, __ea const char *s2);

/* Misc functions */
__ea void *memset_ea (__ea void *dest, int c, size_ea_t n);
size_ea_t strlen_ea (__ea const char *s);

/* Linux system call functions */
ssize_ea_t read_ea(int fd, __ea void *buf, size_ea_t count);
ssize_ea_t pread_ea(int fd, __ea void *buf, size_ea_t count, off_t offset);
ssize_ea_t readv_ea(int fd, struct iovec_ea *vector, int count);
ssize_ea_t write_ea(int fd, __ea const void *buf, size_ea_t count);
ssize_ea_t pwrite_ea(int fd, __ea const void *buf, size_ea_t count, off_t offset);
ssize_ea_t writev_ea(int fd, struct iovec_ea *vector, int count);


#if defined( __EA64__ ) && defined( __EA32__ )
#error __EA64__ and __EA32__ are both defined
#endif

#endif
