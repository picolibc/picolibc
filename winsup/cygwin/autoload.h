/* autoload.h: Define functions for auto-loading symbols from a DLL.

   Copyright 1999 Cygnus Solutions.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define LoadDLLinitfunc(dllname) \
HANDLE NO_COPY dllname ## _handle = NULL; \
static int dllname ## _init () __asm__ (#dllname "_init") __attribute__ ((unused)); \
static int dllname ## _init ()

#define LoadDLLinitnow(dllname) \
  ({__asm__ ("movl $cygwin_dll_func_load, " #dllname "_init_holder"); dllname##_init ();})

#define _LoadDLLinitnow(dllname) \
  __asm__ ("movl $cygwin_dll_func_load, " #dllname "_init_holder"); \
  __asm__ ("call " #dllname "_init"); \

#define LoadDLLinit(dllname) \
  __asm__ (".section .data_cygwin_nocopy,\"w\""); \
  __asm__ (#dllname "_init_holder: .long " #dllname "_init_and_load"); \
  __asm__ (".text"); \
  __asm__ (#dllname "_init_and_load:"); \
  _LoadDLLinitnow (dllname); \
  __asm__ ("jmp cygwin_dll_func_load");


/* Macro for defining "auto-load" functions.
 * Note that this is self-modifying code *gasp*.
 * The first invocation of a routine will trigger the loading of
 * the DLL.  This will then be followed by the discovery of
 * the procedure's entry point, which is placed into the location
 * pointed to by the stack pointer.  This code then changes
 * the "call" operand which invoked it to a "jmp" which will
 * transfer directly to the DLL function on the next invocation.
 *
 * Subsequent calls to routines whose transfer address has not been
 * determined will skip the "load the dll" step, starting at the
 * "discovery of the DLL" step.
 *
 * So, immediately following the the call to one of the above routines
 * we have:
 *  foojmp (4 bytes) 	 Pointer to a word containing the routine used
 *			 to eventually invokethe function.  Initially
 *			 points to an init function which loads the
 *			 DLL, gets the processes load address,
 *			 changes the contents here to point to the
 *			 function address, and changes the call *(%eax)
 *			 to a jmp %eax.  If the initialization has been
 *			 done, only the load part is done.
 *  DLL handle (4 bytes) The handle to use when loading the DLL.
 *  func name (n bytes)	 asciz string containing the name of the function
 *			 to be loaded.
 */

#define LoadDLLfunc(name, mangled, dllname) \
__asm__ (".section .data_cygwin_nocopy,\"w\""); \
__asm__ (".global _" #mangled); \
__asm__ (".global _win32_" #mangled); \
__asm__ (".align 8"); \
__asm__ ("_" #mangled ":"); \
__asm__ ("_win32_" #mangled ":"); \
__asm__ ("movl (" #name "jump),%eax"); \
__asm__ ("call *(%eax)"); \
__asm__ (#name "jump: .long " #dllname "_init_holder"); \
__asm__ (" .long _" #dllname "_handle"); \
__asm__ (".asciz \"" #name "\""); \
__asm__ (".text");

extern "C" void cygwin_dll_func_load () __asm__ ("cygwin_dll_func_load");
