/*
 * mthr.c
 *
 * Implement Mingw thread-support DLL .
 *
 * This file is used iff the following conditions are met:
 *  - gcc uses -mthreads option 
 *  - user code uses C++ exceptions
 *
 * The sole job of the Mingw thread support DLL (MingwThr) is to catch 
 * all the dying threads and clean up the data allocated in the TLSs 
 * for exception contexts during C++ EH. Posix threads have key dtors, 
 * but win32 TLS keys do not, hence the magic. Without this, there's at 
 * least `6 * sizeof (void*)' bytes leaks for each catch/throw in each
 * thread. The only public interface is __mingwthr_key_dtor(). 
 *
 * Created by Mumit Khan  <khan@nanotech.wisc.edu>
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <stdlib.h>

/* To protect the thread/key association data structure modifications. */
CRITICAL_SECTION __mingwthr_cs;

typedef struct __mingwthr_thread __mingwthr_thread_t;
typedef struct __mingwthr_key __mingwthr_key_t;

/* The list of threads active with key/dtor pairs. */
struct __mingwthr_key {
  DWORD key;
  void (*dtor) (void *);
  __mingwthr_key_t *next;
};

/* The list of key/dtor pairs for a particular thread. */
struct __mingwthr_thread {
  DWORD thread_id;
  __mingwthr_key_t *keys;
  __mingwthr_thread_t *next;
};

static __mingwthr_thread_t *__mingwthr_thread_list;

/*
 * __mingwthr_key_add:
 *
 * Add key/dtor association for this thread. If the thread entry does not
 * exist, create a new one and add to the head of the threads list; add
 * the new assoc at the head of the keys list. 
 *
 */

static int
__mingwthr_add_key_dtor (DWORD thread_id, DWORD key, void (*dtor) (void *))
{
  __mingwthr_thread_t *threadp;
  __mingwthr_key_t *new_key;

  new_key = (__mingwthr_key_t *) calloc (1, sizeof (__mingwthr_key_t));
  if (new_key == NULL)
    return -1;
  
  new_key->key = key;
  new_key->dtor = dtor;

  /* This may be called by multiple threads, and so we need to protect
     the whole process of adding the key/dtor pair.  */ 
  EnterCriticalSection (&__mingwthr_cs);

  for (threadp = __mingwthr_thread_list; 
       threadp && (threadp->thread_id != thread_id); 
       threadp = threadp->next)
    ;
  
  if (threadp == NULL)
    {
      threadp = (__mingwthr_thread_t *) 
        calloc (1, sizeof (__mingwthr_thread_t));
      if (threadp == NULL)
        {
	  free (new_key);
	  LeaveCriticalSection (&__mingwthr_cs);
	  return -1;
	}
      threadp->thread_id = thread_id;
      threadp->next = __mingwthr_thread_list;
      __mingwthr_thread_list = threadp;
    }

  new_key->next = threadp->keys;
  threadp->keys = new_key;

  LeaveCriticalSection (&__mingwthr_cs);

#ifdef DEBUG
  printf ("%s: allocating: (%ld, %ld, %x)\n", 
          __FUNCTION__, thread_id, key, dtor);
#endif

  return 0;
}

/*
 * __mingwthr_run_key_dtors (DWORD thread_id):
 *
 * Callback from DllMain when thread detaches to clean up the key
 * storage. 
 *
 * Note that this does not delete the key itself, but just runs
 * the dtor if the current value are both non-NULL. Note that the
 * keys with NULL dtors are not added by __mingwthr_key_dtor, the
 * only public interface, so we don't need to check. 
 *
 */

void
__mingwthr_run_key_dtors (DWORD thread_id)
{
  __mingwthr_thread_t *prev_threadp, *threadp;
  __mingwthr_key_t *keyp;

#ifdef DEBUG
  printf ("%s: Entering Thread id %ld\n", __FUNCTION__, thread_id);
#endif

  /* Since this is called just once per thread, we only need to protect 
     the part where we take out this thread's entry and reconfigure the 
     list instead of wrapping the whole process in a critical section. */
  EnterCriticalSection (&__mingwthr_cs);

  prev_threadp = NULL;
  for (threadp = __mingwthr_thread_list; 
       threadp && (threadp->thread_id != thread_id); 
       prev_threadp = threadp, threadp = threadp->next)
    ;
  
  if (threadp == NULL)
    {
      LeaveCriticalSection (&__mingwthr_cs);
      return;
    }

  /* take the damned thread out of the chain. */
  if (prev_threadp == NULL)		/* first entry hit. */
    __mingwthr_thread_list = threadp->next;
  else
    prev_threadp->next = threadp->next;

  LeaveCriticalSection (&__mingwthr_cs);

  for (keyp = threadp->keys; keyp; )
    {
      __mingwthr_key_t *prev_keyp;
      LPVOID value = TlsGetValue (keyp->key);
      if (GetLastError () == ERROR_SUCCESS)
	{
#ifdef DEBUG
	  printf ("   (%ld, %x)\n", keyp->key, keyp->dtor);
#endif
	  if (value)
	    (*keyp->dtor) (value);
	}
#ifdef DEBUG
      else
	{
	  printf ("   TlsGetValue FAILED  (%ld, %x)\n", 
		  keyp->key, keyp->dtor);
	}
#endif
      prev_keyp = keyp;
      keyp = keyp->next;
      free (prev_keyp);
    }
  
  free (threadp);

#ifdef DEBUG
  printf ("%s: Exiting Thread id %ld\n", __FUNCTION__, thread_id);
#endif
}
  
/*
 * __mingwthr_register_key_dtor (DWORD key, void (*dtor) (void *))
 *
 * Public interface called by C++ exception handling mechanism in
 * libgcc (cf: __gthread_key_create).
 *
 */

__declspec(dllexport)
int
__mingwthr_key_dtor (DWORD key, void (*dtor) (void *))
{
  if (dtor)
    {
      DWORD thread_id = GetCurrentThreadId ();
      return __mingwthr_add_key_dtor (thread_id, key, dtor);
    }

  return 0;
}

