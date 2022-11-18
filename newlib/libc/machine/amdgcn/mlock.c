/*
 * Support file for AMDGCN in newlib.
 * Copyright (c) 2017 Mentor Graphics.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <stdlib.h>
#include <stdint.h>
#include <reent.h>

/* _sbrk_r expects us to use the real errno, not the reentrant one.  */
#include <errno.h>
#undef errno
extern int errno;

/* The runtime passes in heap space like this.  */
struct heap {
  int64_t size;
  char data[0];
};

static char *__heap_ptr = (char*)-1;
static char *__heap_end = (char*)-1;
static int __heap_lock = 0;
static void *__heap_lock_id = NULL;
static int __heap_lock_cnt = 0;

void *
sbrk (ptrdiff_t nbytes)
{
  if (__heap_ptr == (char *)-1)
    {
      /* Find the heap from kernargs.  */
      char *kernargs;
#if defined(__has_builtin) && __has_builtin(__builtin_gcn_kernarg_ptr)
      kernargs = __builtin_gcn_kernarg_ptr ();
#else
      /* The kernargs pointer is in s[8:9].
	 This will break if the enable_sgpr_* flags are ever changed.  */
      asm ("s_mov_b64 %0, s[8:9]" : "=Sg"(kernargs));
#endif

      /* The heap data is at kernargs[3].  */
      struct heap *heap = *(struct heap **)(kernargs + 24);

      __heap_ptr = heap->data;
      __heap_end = __heap_ptr + heap->size;
    }

  if ((__heap_ptr + nbytes) >= __heap_end)
    {
      errno = ENOMEM;
      return (void*)-1;
    }

  char *base = __heap_ptr;
  __heap_ptr += nbytes;

  return base;
}

void
__malloc_lock (struct _reent *reent)
{
  void *id = reent;

  if (id == __heap_lock_id)
    {
      if (__heap_lock_cnt < 1)
	abort ();
      ++__heap_lock_cnt;
      return;
    }

  while (__sync_lock_test_and_set (&__heap_lock, 1))
    /* A sleep seems like it should allow the wavefront to yeild (maybe?)
       Use the shortest possible sleep time of 1*64 cycles.  */
    asm volatile ("s_sleep\t1" ::: "memory");

  if (__heap_lock_id != NULL)
    abort ();
  if (__heap_lock_cnt != 0)
    abort ();

  __heap_lock_cnt = 1;
  __heap_lock_id = id;
}

void
__malloc_unlock (struct _reent *reent)
{
  void *id = reent;

  if (id != __heap_lock_id)
    abort ();
  if (__heap_lock_cnt < 1)
    abort ();

  --__heap_lock_cnt;

  if (__heap_lock_cnt > 0)
    return;

  __heap_lock_id = NULL;
  __sync_lock_release (&__heap_lock);
}
