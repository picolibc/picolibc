/*
 * Support file for amdgcn in newlib.
 * Copyright (c) 2024 BayLibre.
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

/* Lock routines for AMD GPU devices.

   The lock is a 32-bit int:
     - bits 0-3: wavefront id
     - bits 4-23: workgroup id (+1, so never zero)
     - bits 24-31: recursive lock count.

   The purpose of the "relaxed" loads and stores being "atomic" here is
   mostly just to ensure we punch through the caches consistently.

   Non-recursive locks may be unlocked by any thread.  It's an error to
   attempt to unlock a recursive lock from the wrong thread.

   The DEBUG statements here use sprintf and write to avoid taking locks
   themselves.  */

#include <sys/lock.h>
#include <assert.h>

#define DEBUG 0

#if DEBUG
extern void write(int, char *, int);
#endif

static unsigned
__gcn_thread_id ()
{
  /* Dim(0) is the workgroup ID; range 0 to maybe thousands.
     Dim(1) is the wavefront ID; range 0 to 15.  */
  return (((__builtin_gcn_dim_pos (0) + 1) << 4)
	  + __builtin_gcn_dim_pos (1));
}

static int
__gcn_lock_acquire_int (_LOCK_T *lock_ptr, int _try)
{
  int id = __gcn_thread_id ();

#if DEBUG
  char buf[1000];
  __builtin_sprintf (buf,"acquire:%p(%d) lock_value:0x%x  id:0x%x", lock_ptr,
		     _try, *lock_ptr, id);
  write (1, buf, __builtin_strlen(buf));
#endif

  int expected = 0;
  while (!__atomic_compare_exchange_n (lock_ptr, &expected, id, 0,
				       __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
    {
      /* Lock *not* acquired.  */
      if (_try)
	return 0;
      else
	{
	  asm ("s_sleep 64");
	  expected = 0;
	}
    }

#if DEBUG
  __builtin_sprintf (buf,"acquired:%p(%d) lock_value:0x%x  id:0x%x", lock_ptr,
		     _try, *lock_ptr, id);
  write (1, buf, __builtin_strlen(buf));
#endif

  return 1;
}

int
__gcn_try_lock_acquire (_LOCK_T *lock_ptr)
{
  return __gcn_lock_acquire_int (lock_ptr, 1);
}

void
__gcn_lock_acquire (_LOCK_T *lock_ptr)
{
  __gcn_lock_acquire_int (lock_ptr, 0);
}

static int
__gcn_lock_acquire_recursive_int (_LOCK_T *lock_ptr, int _try)
{
  int id = __gcn_thread_id ();

#if DEBUG
  char buf[1000];
  __builtin_sprintf (buf,"acquire recursive:%p(%d) lock_value:0x%x  id:0x%x",
		     lock_ptr, _try, *lock_ptr, id);
  write (1, buf, __builtin_strlen(buf));
#endif

  unsigned int lock_value = __atomic_load_n (lock_ptr, __ATOMIC_RELAXED);
  if ((lock_value & 0xffffff) == id)
    {
      /* This thread already holds the lock.
	 Increment the recursion counter and update the lock.  */
      int count = lock_value >> 24;
      lock_value = ((count + 1) << 24) | id;
      __atomic_store_n (lock_ptr, lock_value, __ATOMIC_RELAXED);

#if DEBUG
      __builtin_sprintf (buf,
			 "increment recursive:%p(%d) lock_value:0x%x  id:0x%x",
			 lock_ptr, _try, *lock_ptr, id);
      write (1, buf, __builtin_strlen(buf));
#endif

      return 1;
    }
  else
    return __gcn_lock_acquire_int (lock_ptr, _try);
}

int
__gcn_lock_try_acquire_recursive (_LOCK_T *lock_ptr)
{
  return __gcn_lock_acquire_recursive_int (lock_ptr, 1);
}

void
__gcn_lock_acquire_recursive (_LOCK_T *lock_ptr)
{
  __gcn_lock_acquire_recursive_int (lock_ptr, 0);
}

void
__gcn_lock_release (_LOCK_T *lock_ptr)
{
#if DEBUG
  char buf[1000];
  __builtin_sprintf (buf,"release:%p lock_value:0x%x  id:0x%x", lock_ptr,
		     *lock_ptr, __gcn_thread_id());
  write (1, buf, __builtin_strlen(buf));
#endif

  __atomic_store_n (lock_ptr, 0, __ATOMIC_RELEASE);
}

void
__gcn_lock_release_recursive (_LOCK_T *lock_ptr)
{
  int id = __gcn_thread_id ();
  unsigned int lock_value = __atomic_load_n (lock_ptr, __ATOMIC_RELAXED);

#if DEBUG
  char buf[1000];
  __builtin_sprintf (buf, "release recursive:%p lock_value:0x%x  id:0x%x",
		     lock_ptr, lock_value, id);
  write (1, buf, __builtin_strlen(buf));
#endif

  /* It is an error to call this function from the wrong thread.  */
  assert ((lock_value & 0xffffff) == id);

  /* Decrement or release the lock.  */
  int count = lock_value >> 24;
  if (count > 0)
    {
      lock_value = ((count - 1) << 24) | id;
      __atomic_store_n (lock_ptr, lock_value, __ATOMIC_RELAXED);

#if DEBUG
      __builtin_sprintf (buf, "decrement recursive:%p lock_value:0x%x  id:0x%x",
			 lock_ptr, *lock_ptr, id);
      write (1, buf, __builtin_strlen(buf));
#endif
    }
  else
    __gcn_lock_release (lock_ptr);
}
