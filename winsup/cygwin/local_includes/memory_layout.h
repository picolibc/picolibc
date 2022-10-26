/* memory_layout.h: document all addresses crucial to the fixed memory
		    layout of Cygwin processes.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* We use fixed addresses outside the low 32 bit arena, which is exclusively
   used by the OS now:
     - The executable starts at 0x1:00400000L
     - The Cygwin DLL starts at 0x1:80040000L
     - Rebased DLLs are located from 0x2:00000000L up to 0x4:00000000L
     - auto-image-based DLLs are located from 0x4:00000000L up to 0x6:00000000L
     - Thread stacks are located from 0x6:00000000L up to 0x8:00000000L.
     - So the heap starts at 0x8:00000000L. */

  /* TODO: Make Cygwin work with ASLR.
     - The executable starts at 0x1:00400000L
     - Rebased non-ASLRed DLLs from 0x2:00000000L up to 0x4:00000000L
     - auto-image-based non-ASLRed DLLs from 0x4:00000000L up to 0x6:00000000L
     - Thread stacks are located from 0x6:00000000L up to 0x8:00000000L.
     - cygheap from 0x8:00000000L up to 0xa:00000000L.
     - So the heap starts at 0xa:00000000L. */

/* This is where the Cygwin executables are loaded to. */
#define EXECUTABLE_ADDRESS		0x100400000UL

/* Fixed address set by the linker. The Cygwin DLL will have this address set
   in the DOS header. Keep this area free with ASLR, for the case where
   dynamicbase is accidentally not set in the PE/COFF header of the DLL. */
#define CYGWIN_DLL_ADDRESS		0x180040000UL

/* Rebased DLLs are located in this 16 Gigs arena.  Will be kept for
   backward compatibility. */
#define REBASED_DLL_STORAGE_LOW		0x200000000UL
#define REBASED_DLL_STORAGE_HIGH	0x400000000UL

/* Auto-image-based DLLs are located in this 16 Gigs arena.  This is used
   by the linker to set a default address for DLLs. */
#define AUTOBASED_DLL_STORAGE_LOW	0x400000000UL
#define AUTOBASED_DLL_STORAGE_HIGH	0x600000000UL

/* Storage area for thread stacks. */
#define THREAD_STORAGE_LOW		0x600000000UL
#define THREAD_STORAGE_HIGH		0x800000000UL

/* That's where the cygheap is located. CYGHEAP_STORAGE_INITIAL defines the
   end of the initially committed heap area. */
#define CYGHEAP_STORAGE_LOW		0x800000000UL
#define CYGHEAP_STORAGE_INITIAL		0x800300000UL
#define CYGHEAP_STORAGE_HIGH		0xa00000000UL

/* This is where the user heap starts.  There's no defined end address.
   The user heap pontentially grows into the mmap arena.  However,
   the user heap grows upwar4ds and the mmap arena grows downwards,
   so there's not much chance to meet unluckily. */
#define USERHEAP_START			0xa00000000UL

/* The memory region used for memory maps.
   Up to Win 8 only 44 bit address space, 48 bit starting witrh 8.1, so
   the max value is variable. */
#define MMAP_STORAGE_LOW	0x001000000000L /* Leave ~32 Gigs for heap. */
#define MMAP_STORAGE_HIGH       wincap.mmap_storage_high ()
