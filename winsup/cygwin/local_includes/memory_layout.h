/* memory_layout.h: document all addresses crucial to the fixed memory
		    layout of Cygwin processes.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* This is where Cygwin executables are loaded to, unless dynamicbase is
   enabled in the PE/COFF header of the executable file. */
#define EXECUTABLE_ADDRESS		0x100400000UL

/* Fixed address set by the linker. The Cygwin DLL will have this address set
   in the DOS header. Keep this area free with ASLR, for the case where
   dynamicbase is accidentally not set in the PE/COFF header of the DLL. */
#define CYGWIN_DLL_ADDRESS		0x180040000UL

/* Area for Cygwin-specific shared memory regions. */
#define SHARED_REGIONS_ADDRESS_LOW	0x1a0000000UL
#define SHARED_REGIONS_ADDRESS_HIGH	0x200000000UL

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
   the user heap grows upwards and the mmap arena grows downwards,
   so there's not much chance to meet unluckily. */
#define USERHEAP_START			0xa00000000UL

/* The memory region used for memory maps.  Mmaps grow downwards.
   Set the lowest address to leave ~32 Gigs for heap. */
#define MMAP_STORAGE_LOW		0x001000000000UL
#define MMAP_STORAGE_HIGH		0x700000000000UL

/* Default number of pages used as thread stack guard pages. */
#define DEFAULT_GUARD_PAGE_COUNT	3
