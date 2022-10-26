/* create_posix_thread.cc: funcs to create posix threads or thread stacks

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/param.h>
#include "create_posix_thread.h"
#include "cygheap_malloc.h"
#include "ntdll.h"
#include "mmap_alloc.h"

/* create_posix_thread

   Replacement function for CreateThread to create pthreads.  Mainly this
   creates its own stack, either from Cygwin's thread pool, or allowing
   the caller to specify own stack addresses, stack sizes and guard pages.

   create_new_main_thread_stack

   Just set up a system-like main thread stack from the pthread stack area
   maintained by the thr_alloc class.  See the description in _dll_crt0 to
   understand why we have to do this. */

struct pthread_wrapper_arg
{
  LPTHREAD_START_ROUTINE func;
  PVOID arg;
  PBYTE stackaddr;
  PBYTE stackbase;
  PBYTE stacklimit;
  ULONG guardsize;
};

DWORD
pthread_wrapper (PVOID arg)
{
  /* Just plain paranoia. */
  if (!arg)
    return ERROR_INVALID_PARAMETER;

  /* The process is now threaded.  Note for later usage by arc4random. */
  __isthreaded = 1;

  /* Fetch thread wrapper info and free from cygheap. */
  pthread_wrapper_arg wrapper_arg = *(pthread_wrapper_arg *) arg;
  cfree (arg);

  /* Set stack values in TEB */
  PTEB teb = NtCurrentTeb ();
  teb->Tib.StackBase = wrapper_arg.stackbase;
  teb->Tib.StackLimit = wrapper_arg.stacklimit ?: wrapper_arg.stackaddr;
  /* Set DeallocationStack value.  If we have an application-provided stack,
     we set DeallocationStack to NULL, so NtTerminateThread does not deallocate
     any stack.  If we created the stack in CygwinCreateThread, we set
     DeallocationStack to the stackaddr of our own stack, so it's automatically
     deallocated when the thread is terminated. */
  PBYTE dealloc_addr = (PBYTE) teb->DeallocationStack;
  teb->DeallocationStack = wrapper_arg.stacklimit ? wrapper_arg.stackaddr
						  : NULL;
  /* Store the OS-provided DeallocationStack address in wrapper_arg.stackaddr.
     The below assembler code will release the OS stack after switching to our
     new stack. */
  wrapper_arg.stackaddr = dealloc_addr;
  /* Set thread stack guarantee matching the guardsize.
     Note that the guardsize is one page bigger than the guarantee. */
  if (wrapper_arg.guardsize > wincap.def_guard_page_size ())
    {
      wrapper_arg.guardsize -= wincap.page_size ();
      SetThreadStackGuarantee (&wrapper_arg.guardsize);
    }
  /* Initialize new _cygtls. */
  _my_tls.init_thread (wrapper_arg.stackbase - __CYGTLS_PADSIZE__,
		       (DWORD (*)(void*, void*)) wrapper_arg.func);
#ifdef __x86_64__
  __asm__ ("\n\
	   leaq  %[WRAPPER_ARG], %%rbx	# Load &wrapper_arg into rbx	\n\
	   movq  (%%rbx), %%r12		# Load thread func into r12	\n\
	   movq  8(%%rbx), %%r13	# Load thread arg into r13	\n\
	   movq  16(%%rbx), %%rcx	# Load stackaddr into rcx	\n\
	   movq  24(%%rbx), %%rsp	# Load stackbase into rsp	\n\
	   subq  %[CYGTLS], %%rsp	# Subtract __CYGTLS_PADSIZE__	\n\
					# (here we are 16 bytes aligned)\n\
	   subq  $32, %%rsp		# Subtract another 32 bytes	\n\
					# (shadow space for arg regs)	\n\
	   xorq  %%rbp, %%rbp		# Set rbp to 0			\n\
	   # We moved to the new stack.					\n\
	   # Now it's safe to release the OS stack.			\n\
	   movl  $0x8000, %%r8d		# dwFreeType: MEM_RELEASE	\n\
	   xorl  %%edx, %%edx		# dwSize:     0			\n\
	   # dwAddress is already in the correct arg register rcx	\n\
	   call  VirtualFree						\n\
	   # All set.  We can copy the thread arg from the safe		\n\
	   # register r13 and then just call the function.		\n\
	   movq  %%r13, %%rcx		# Move thread arg to 1st arg reg\n\
	   call  *%%r12			# Call thread func		\n"
	   : : [WRAPPER_ARG] "o" (wrapper_arg),
	       [CYGTLS] "i" (__CYGTLS_PADSIZE__));
#else
#error unimplemented for this target
#endif
  /* pthread::thread_init_wrapper calls pthread::exit, which
     in turn calls ExitThread, so we should never arrive here. */
  api_fatal ("Dumb thinko in pthread handling.  Whip the developer.");
}

/* We provide the stacks always in 1 Megabyte slots */
#define THREAD_STACK_SLOT	0x000100000L	/* 1 Meg */
/* Maximum stack size returned from the pool. */
#define THREAD_STACK_MAX	0x040000000L	/* 1 Gig */

class thread_allocator
{
  UINT_PTR current;
  PVOID (thread_allocator::*alloc_func) (SIZE_T);
  PVOID _alloc (SIZE_T size)
  {
    static const MEM_ADDRESS_REQUIREMENTS thread_req = {
      (PVOID) THREAD_STORAGE_LOW,
      (PVOID) (THREAD_STORAGE_HIGH - 1),
      THREAD_STACK_SLOT
    };
    /* g++ 11.2 workaround: don't use initializer */
    MEM_EXTENDED_PARAMETER thread_ext = { 0 };
    thread_ext.Type = MemExtendedParameterAddressRequirements;
    thread_ext.Pointer = (PVOID) &thread_req;

    SIZE_T real_size = roundup2 (size, THREAD_STACK_SLOT);
    PVOID real_stackaddr = NULL;

    if (real_size <= THREAD_STACK_MAX)
      real_stackaddr = VirtualAlloc2 (GetCurrentProcess(), NULL, real_size,
				      MEM_RESERVE | MEM_TOP_DOWN,
				      PAGE_READWRITE, &thread_ext, 1);
    /* If the thread area allocation failed, or if the application requests a
       monster stack, fulfill request from mmap area. */
    if (!real_stackaddr)
      {
	static const MEM_ADDRESS_REQUIREMENTS mmap_req = {
	  (PVOID) MMAP_STORAGE_LOW,
	  (PVOID) (MMAP_STORAGE_HIGH - 1),
	  THREAD_STACK_SLOT
	};
	/* g++ 11.2 workaround: don't use initializer */
	MEM_EXTENDED_PARAMETER mmap_ext = { 0 };
	mmap_ext.Type = MemExtendedParameterAddressRequirements;
	mmap_ext.Pointer = (PVOID) &mmap_req;

	real_stackaddr = VirtualAlloc2 (GetCurrentProcess(), NULL, real_size,
					MEM_RESERVE | MEM_TOP_DOWN,
					PAGE_READWRITE, &mmap_ext, 1);
      }
    return real_stackaddr;
  }
  PVOID _alloc_old (SIZE_T size)
  {
    SIZE_T real_size = roundup2 (size, THREAD_STACK_SLOT);
    BOOL overflow = FALSE;
    PVOID real_stackaddr = NULL;

    /* If an application requests a monster stack, fulfill request
       from mmap area. */
    if (real_size > THREAD_STACK_MAX)
      {
	PVOID addr = mmap_alloc.alloc (NULL, real_size, false);
	return VirtualAlloc (addr, real_size, MEM_RESERVE, PAGE_READWRITE);
      }
    /* Simple round-robin.  Keep looping until VirtualAlloc succeeded, or
       until we overflowed and hit the current address. */
    for (UINT_PTR addr = current - real_size;
	 !real_stackaddr && (!overflow || addr >= current);
	 addr -= THREAD_STACK_SLOT)
      {
	if (addr < THREAD_STORAGE_LOW)
	  {
	    addr = THREAD_STORAGE_HIGH - real_size;
	    overflow = TRUE;
	  }
	real_stackaddr = VirtualAlloc ((PVOID) addr, real_size,
				       MEM_RESERVE, PAGE_READWRITE);
	if (!real_stackaddr)
	  {
	    /* So we couldn't grab this space.  Let's check the state.
	       If this area is free, simply try the next lower 1 Meg slot.
	       Otherwise, shift the next try down to the AllocationBase
	       of the current address, minus the requested slot size.
	       Add THREAD_STACK_SLOT since that's subtracted in the next
	       run of the loop anyway. */
	    MEMORY_BASIC_INFORMATION mbi;
	    VirtualQuery ((PVOID) addr, &mbi, sizeof mbi);
	    if (mbi.State != MEM_FREE)
	      addr = (UINT_PTR) mbi.AllocationBase - real_size
						    + THREAD_STACK_SLOT;
	  }
      }
    /* If we got an address, remember it for the next allocation attempt. */
    if (real_stackaddr)
      current = (UINT_PTR) real_stackaddr;
    else
      set_errno (EAGAIN);
    return real_stackaddr;
  }
public:
  thread_allocator () : current (THREAD_STORAGE_HIGH)
  {
    alloc_func = wincap.has_extended_mem_api () ? &_alloc : &_alloc_old;
  }
  PVOID alloc (SIZE_T size)
  {
    return (this->*alloc_func) (size);
  }
};

thread_allocator thr_alloc NO_COPY;

PVOID
create_new_main_thread_stack (PVOID &allocationbase)
{
  PIMAGE_DOS_HEADER dosheader;
  PIMAGE_NT_HEADERS ntheader;
  SIZE_T stacksize;
  ULONG guardsize;
  SIZE_T commitsize;
  PBYTE stacklimit;

  dosheader = (PIMAGE_DOS_HEADER) GetModuleHandle (NULL);
  ntheader = (PIMAGE_NT_HEADERS)
	     ((PBYTE) dosheader + dosheader->e_lfanew);
  stacksize = ntheader->OptionalHeader.SizeOfStackReserve;
  stacksize = roundup2 (stacksize, wincap.allocation_granularity ());

  allocationbase
	= thr_alloc.alloc (ntheader->OptionalHeader.SizeOfStackReserve);
  guardsize = wincap.def_guard_page_size ();
  commitsize = ntheader->OptionalHeader.SizeOfStackCommit;
  commitsize = roundup2 (commitsize, wincap.page_size ());
  if (commitsize > stacksize - guardsize - wincap.page_size ())
    commitsize = stacksize - guardsize - wincap.page_size ();
  stacklimit = (PBYTE) allocationbase + stacksize - commitsize - guardsize;
  /* Setup guardpage. */
  if (!VirtualAlloc (stacklimit, guardsize,
		     MEM_COMMIT, PAGE_READWRITE | PAGE_GUARD))
    return NULL;
  /* Setup committed region. */
  stacklimit += guardsize;
  if (!VirtualAlloc (stacklimit, commitsize, MEM_COMMIT, PAGE_READWRITE))
    return NULL;
  NtCurrentTeb()->Tib.StackBase = ((PBYTE) allocationbase + stacksize);
  NtCurrentTeb()->Tib.StackLimit = stacklimit;
  return ((PBYTE) allocationbase + stacksize - 16);
}

HANDLE
create_posix_thread (LPTHREAD_START_ROUTINE thread_func, PVOID thread_arg,
		     PVOID stackaddr, ULONG stacksize, ULONG guardsize,
		     DWORD creation_flags, LPDWORD thread_id)
{
  PVOID real_stackaddr = NULL;
  ULONG real_stacksize = 0;
  ULONG real_guardsize = 0;
  pthread_wrapper_arg *wrapper_arg;
  HANDLE thread = NULL;

  wrapper_arg = (pthread_wrapper_arg *) ccalloc (HEAP_STR, 1,
						sizeof *wrapper_arg);
  if (!wrapper_arg)
    {
      SetLastError (ERROR_OUTOFMEMORY);
      return NULL;
    }
  wrapper_arg->func = thread_func;
  wrapper_arg->arg = thread_arg;

  if (stackaddr)
    {
      /* If the application provided the stack, just use it.  There won't
	 be any stack overflow handling! */
      wrapper_arg->stackaddr = (PBYTE) stackaddr;
      wrapper_arg->stackbase = (PBYTE) stackaddr + stacksize;
    }
  else
    {
      PBYTE real_stacklimit;

      /* If not, we have to create the stack here. */
      real_stacksize = roundup2 (stacksize, wincap.page_size ());
      real_guardsize = roundup2 (guardsize, wincap.page_size ());
      /* Add the guardsize to the stacksize */
      real_stacksize += real_guardsize;
      /* Take dead zone page into account, which always stays uncommited. */
      real_stacksize += wincap.page_size ();
      /* Now roundup the result to the next allocation boundary. */
      real_stacksize = roundup2 (real_stacksize,
				 wincap.allocation_granularity ());
      /* Reserve stack. */
      real_stackaddr = thr_alloc.alloc (real_stacksize);
      if (!real_stackaddr)
	return NULL;
      /* Set up committed region.  We set up the stack like the OS does,
	 with a reserved region, the guard pages, and a commited region.
	 We commit the stack commit size from the executable header, but
	 at least PTHREAD_STACK_MIN (64K). */
      static ULONG exe_commitsize;

      if (!exe_commitsize)
	{
	  PIMAGE_DOS_HEADER dosheader;
	  PIMAGE_NT_HEADERS ntheader;

	  dosheader = (PIMAGE_DOS_HEADER) GetModuleHandle (NULL);
	  ntheader = (PIMAGE_NT_HEADERS)
		     ((PBYTE) dosheader + dosheader->e_lfanew);
	  exe_commitsize = ntheader->OptionalHeader.SizeOfStackCommit;
	  exe_commitsize = roundup2 (exe_commitsize, wincap.page_size ());
	}
      ULONG commitsize = exe_commitsize;
      if (commitsize > real_stacksize - real_guardsize - wincap.page_size ())
	commitsize = real_stacksize - real_guardsize - wincap.page_size ();
      else if (commitsize < PTHREAD_STACK_MIN)
	commitsize = PTHREAD_STACK_MIN;
      real_stacklimit = (PBYTE) real_stackaddr + real_stacksize
			- commitsize - real_guardsize;
      if (!VirtualAlloc (real_stacklimit, real_guardsize, MEM_COMMIT,
			 PAGE_READWRITE | PAGE_GUARD))
	goto err;
      real_stacklimit += real_guardsize;
      if (!VirtualAlloc (real_stacklimit, commitsize, MEM_COMMIT,
			 PAGE_READWRITE))
	goto err;

      wrapper_arg->stackaddr = (PBYTE) real_stackaddr;
      wrapper_arg->stackbase = (PBYTE) real_stackaddr + real_stacksize;
      wrapper_arg->stacklimit = real_stacklimit;
      wrapper_arg->guardsize = real_guardsize;
    }
  /* Use the STACK_SIZE_PARAM_IS_A_RESERVATION parameter so only the
     minimum size for a thread stack is reserved by the OS.  Note that we
     reserve a 256K stack, not 64K, otherwise the thread creation might
     crash the process due to a stack overflow. */
  thread = CreateThread (&sec_none_nih, 4 * PTHREAD_STACK_MIN,
			 pthread_wrapper, wrapper_arg,
			 creation_flags | STACK_SIZE_PARAM_IS_A_RESERVATION,
			 thread_id);

err:
  if (!thread && real_stackaddr)
    {
      /* Don't report the wrong error even though VirtualFree is very unlikely
	 to fail. */
      DWORD err = GetLastError ();
      VirtualFree (real_stackaddr, 0, MEM_RELEASE);
      SetLastError (err);
    }
  return thread;
}
