/* autoload.cc: all dynamic load stuff.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
   2011, 2012, 2013, 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include "fenv.h"
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>

bool NO_COPY wsock_started;

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
 * "discovery of the entry point" step.
 *
 * So, immediately following the the call to one of the above routines
 * we have:
 *  DLL info (4/8 bytes) Pointer to a block of information concerning
 *			 the DLL (see below).
 *  DLL args (4 bytes)	 The number of arguments pushed on the stack by
 *			 the call.  If this is an odd value then this
 *			 is a flag that non-existence of this function
 *			 is not a fatal error
 *  func addr (8 bytes)  (64 bit ONLY!)
 *			 Address of the actual Win32 function.  For the
 *			 reason why this is necessary, see the below
 *			 description of the load_state.
 *  func name (n bytes)	 asciz string containing the name of the function
 *			 to be loaded.
 *
 * The DLL info block consists of the following
 *  load_state (4/8 bytes) Pointer to a word containing the routine used
 *			 to eventually invoke the function.  Initially
 *			 points to an init function which loads the DLL,
 *			 gets the process's load address, changes the contents
 *			 here to point to the function address, and changes
 *			 the address argument of the initial jmp call.
 *			 On 64 bit, the jmp is not tweaked directly.  Rather,
 *			 the address of the Win32 function is stored in the
 *			 aforementioned Win32 function address slot and fetched
 *			 there for a jmp *%rax call.  This indirection is
 *			 necessary to workaround the lack of a jmp opcode with
 *			 offset values > 32 bit.  If the initialization has
 *			 been done, only the load part is done.
 *  DLL handle (4/8 bytes) The handle to use when loading the DLL.
 *  DLL locker (4 bytes) Word to use to avoid multi-thread access during
 *			 initialization.
 *  extra init (4/8 bytes) Extra initialization function.
 *  DLL name (n bytes)	 asciz string containing the name of the DLL.
 */

/* LoadDLLprime is used to prime the DLL info information, providing an
   additional initialization routine to call prior to calling the first
   function.  */
#ifdef __x86_64__
#define LoadDLLprime(dllname, init_also, no_resolve_on_fork) __asm__ ("	\n\
.ifndef " #dllname "_primed				\n\
  .section	.data_cygwin_nocopy,\"w\"		\n\
  .align	8					\n\
."#dllname "_info:					\n\
  .quad		_std_dll_init				\n\
  .quad		" #no_resolve_on_fork "			\n\
  .long		-1					\n\
  .align	8					\n\
  .quad		" #init_also "				\n\
  .string16	\"" #dllname ".dll\"			\n\
  .text							\n\
  .set		" #dllname "_primed, 1			\n\
.endif							\n\
");
#else
#define LoadDLLprime(dllname, init_also, no_resolve_on_fork) __asm__ ("	\n\
.ifndef " #dllname "_primed				\n\
  .section	.data_cygwin_nocopy,\"w\"		\n\
  .align	4					\n\
."#dllname "_info:					\n\
  .long		_std_dll_init				\n\
  .long		" #no_resolve_on_fork "			\n\
  .long		-1					\n\
  .long		" #init_also "				\n\
  .string16	\"" #dllname ".dll\"			\n\
  .text							\n\
  .set		" #dllname "_primed, 1			\n\
.endif							\n\
");
#endif

/* Create a "decorated" name */
#define mangle(name, n) #name "@" #n

/* Standard DLL load macro.  May invoke a fatal error if the function isn't
   found. */
#define LoadDLLfunc(name, n, dllname) \
  LoadDLLfuncEx (name, n, dllname, 0)
#define LoadDLLfuncEx(name, n, dllname, notimp) \
  LoadDLLfuncEx2(name, n, dllname, notimp, 0)
#define LoadDLLfuncEx2(name, n, dllname, notimp, err) \
  LoadDLLfuncEx3(name, n, dllname, notimp, err, 0)

/* Main DLL setup stuff. */
#ifdef __x86_64__
#define LoadDLLfuncEx3(name, n, dllname, notimp, err, no_resolve_on_fork) \
  LoadDLLprime (dllname, dll_func_load, no_resolve_on_fork) \
  __asm__ ("						\n\
  .section	." #dllname "_autoload_text,\"wx\"	\n\
  .global	" #name "				\n\
  .global	_win32_" #name "			\n\
  .align	16					\n\
" #name ":						\n\
_win32_" #name ":					\n\
  movq		3f(%rip),%rax				\n\
  jmp		*%rax					\n\
1:movq		2f(%rip),%rax				\n\
  push		%rbp		# Keep 16 byte aligned	\n\
  push		%r9					\n\
  push		%r8					\n\
  push		%rdx					\n\
  push		%rcx					\n\
  call		*(%rax)					\n\
2:.quad		." #dllname "_info			\n\
  .long		(" #n "+" #notimp ") | (((" #err ") & 0xff) <<16) \n\
3:.quad		1b					\n\
  .asciz	\"" #name "\"				\n\
  .text							\n\
");
#else
#define LoadDLLfuncEx3(name, n, dllname, notimp, err, no_resolve_on_fork) \
  LoadDLLprime (dllname, dll_func_load, no_resolve_on_fork) \
  __asm__ ("						\n\
  .section	." #dllname "_autoload_text,\"wx\"	\n\
  .global	_" mangle (name, n) "			\n\
  .global	_win32_" mangle (name, n) "		\n\
  .align	8					\n\
_" mangle (name, n) ":					\n\
_win32_" mangle (name, n) ":				\n\
  .byte		0xe9					\n\
  .long		-4 + 1f - .				\n\
1:movl		(2f),%eax				\n\
   call		*(%eax)					\n\
2:.long		." #dllname "_info			\n\
  .long		(" #n "+" #notimp ") | (((" #err ") & 0xff) <<16) \n\
  .asciz	\"" #name "\"				\n\
  .text							\n\
");
#endif

/* DLL loader helper functions used during initialization. */

/* The function which finds the address, given the name and overwrites
   the call so that future invocations go straight to the function in
   the DLL. */
extern "C" void dll_func_load () __asm__ ("dll_func_load");

/* Called by the primary initialization function "init_std_dll" to
   setup the stack and eliminate future calls to init_std_dll for other
   functions from this DLL.  */
extern "C" void dll_chain () __asm__ ("dll_chain");

extern "C" {

#ifdef __x86_64__
__asm__ ("								\n\
	 .section .rdata,\"r\"							\n\
msg1:									\n\
	.ascii	\"couldn't dynamically determine load address for '%s' (handle %p), %E\\0\"\n\
									\n\
	 .text								\n\
	.p2align 4,,15							\n\
noload:									\n\
	movq	40(%rsp),%rdx	# Get the address of the information block\n\
	movl	8(%rdx),%eax	# Should we 'ignore' the lack		\n\
	test	$1,%eax		#  of this function?			\n\
	jz	1f		# Nope.					\n\
	andl	$0xffff0000,%eax# upper word (== desired return value)	\n\
	movl	%eax,32(%rsp)	# Save for later (in shadow space)	\n\
	movl	$127,%ecx	# ERROR_PROC_NOT_FOUND			\n\
	call	SetLastError	# Set it				\n\
	movl	32(%rsp),%eax	# Get back return value			\n\
	sarl	$16,%eax	# swap to low order word		\n\
	addq	$40,%rsp	# Revert stack				\n\
	pop	%r10		# Drop pointer to 'return address'	\n\
	pop	%rcx		# Restore arg registers			\n\
	pop	%rdx							\n\
	pop	%r8							\n\
	pop	%r9							\n\
	pop	%rbp		# ...and restore frame pointer		\n\
	ret			# Return				\n\
1:									\n\
	movq	(%rdx),%rax	# Handle value				\n\
	movq	8(%rax),%r8						\n\
	lea	20(%rdx),%rdx	# Location of name of function		\n\
	lea	msg1(%rip),%rcx	# The message				\n\
	call	api_fatal	# Print message. Never returns		\n\
									\n\
	.globl	dll_func_load						\n\
dll_func_load:								\n\
	movq	(%rsp),%rdx	# 'Return address' contains load info	\n\
	movq	(%rdx),%rcx	# Where handle lives			\n\
	movq	8(%rcx),%rcx	# Address of Handle to DLL		\n\
	addq	$20,%rdx	# Address of name of function to load	\n\
	subq	$40,%rsp	# Shadow space + 8 byte for alignment	\n\
	call	GetProcAddress	# Load it				\n\
	test	%rax,%rax	# Success?				\n\
	jne	gotit		# Yes					\n\
	jmp	noload		# Issue an error or return		\n\
gotit:									\n\
	addq	$40,%rsp	# Revert stack				\n\
	pop 	%r10		# Pointer to 'return address'		\n\
	movq	%rax,12(%r10)	# Move absolute address to address slot	\n\
	subq	$25,%r10	# Point to jmp				\n\
	pop	%rcx		# Restore arg registers			\n\
	pop	%rdx							\n\
	pop	%r8							\n\
	pop	%r9							\n\
	pop	%rbp		# ...and restore frame pointer		\n\
	jmp	*%r10		# Jump to actual function		\n\
									\n\
	.global	dll_chain						\n\
dll_chain:								\n\
	push	%rax		# Restore 'return address'		\n\
	jmp	*%rdx		# Jump to next init function		\n\
");
#else
__asm__ ("								\n\
	 .text								\n\
msg1:									\n\
	.ascii	\"couldn't dynamically determine load address for '%s' (handle %p), %E\\0\"\n\
									\n\
	.align	32							\n\
noload:									\n\
	popl	%edx		# Get the address of the information block\n\
	movl	4(%edx),%eax	# Should we 'ignore' the lack		\n\
	test	$1,%eax		#  of this function?			\n\
	jz	1f		# Nope.					\n\
	decl	%eax		# Yes.  This is the # of bytes + 1	\n\
	popl	%edx		# Caller's caller			\n\
	addl	%eax,%esp	# Pop off bytes				\n\
	andl	$0xffff0000,%eax# upper word				\n\
	subl	%eax,%esp	# adjust for possible return value	\n\
	pushl	%eax		# Save for later			\n\
	movl	$127,%eax	# ERROR_PROC_NOT_FOUND			\n\
	pushl	%eax		# First argument			\n\
	call	_SetLastError@4	# Set it				\n\
	popl	%eax		# Get back argument			\n\
	sarl	$16,%eax	# return value in high order word	\n\
	jmp	*%edx		# Return				\n\
1:									\n\
	movl	(%edx),%eax	# Handle value				\n\
	pushl	4(%eax)							\n\
	leal	8(%edx),%eax	# Location of name of function		\n\
	pushl	%eax							\n\
	pushl	$msg1		# The message				\n\
	call	_api_fatal	# Print message. Never returns		\n\
									\n\
	.globl	dll_func_load						\n\
dll_func_load:								\n\
	movl	(%esp),%eax	# 'Return address' contains load info	\n\
	addl	$8,%eax		# Address of name of function to load	\n\
	pushl	%eax		# Second argument			\n\
	movl	-8(%eax),%eax	# Where handle lives			\n\
	movl	4(%eax),%eax	# Address of Handle to DLL		\n\
	pushl	%eax		# Handle to DLL				\n\
	call	_GetProcAddress@8# Load it				\n\
	test	%eax,%eax	# Success?				\n\
	jne	gotit		# Yes					\n\
	jmp	noload		# Issue an error or return		\n\
gotit:									\n\
	popl	%edx		# Pointer to 'return address'		\n\
	subl	%edx,%eax	# Make it relative			\n\
	addl	$7,%eax		# Tweak					\n\
	subl	$12,%edx	# Point to jmp				\n\
	movl	%eax,1(%edx)	# Move relative address after jump	\n\
	jmp	*%edx		# Jump to actual function		\n\
									\n\
	.global	dll_chain						\n\
dll_chain:								\n\
	pushl	%eax		# Restore 'return address'		\n\
	jmp	*%edx		# Jump to next init function		\n\
");
#endif

/* C representations of the two info blocks described above.
   FIXME: These structures confuse gdb for some reason.  GDB can print
   the whole structure but has problems with the name field? */
struct dll_info
{
  UINT_PTR load_state;
  HANDLE handle;
  LONG here;
  void (*init) ();
  WCHAR name[];
};

struct func_info
{
  struct dll_info *dll;
  LONG decoration;
#ifdef __x86_64__
  UINT_PTR func_addr;
#endif
  char name[];
};

/* Mechanism for setting up info for passing to dll_chain routines. */
#ifdef __x86_64__
typedef __uint128_t two_addr_t;
#else
typedef __uint64_t two_addr_t;
#endif
union retchain
{
  struct {uintptr_t high; uintptr_t low;};
  two_addr_t ll;
};


/* This function is a workaround for the problem reported here:
  http://cygwin.com/ml/cygwin/2011-02/msg00552.html
  and discussed here:
  http://cygwin.com/ml/cygwin-developers/2011-02/threads.html#00007

  To wit: winmm.dll calls FreeLibrary in its DllMain and that can result
  in LoadLibraryExW returning an ERROR_INVALID_ADDRESS.  */
static __inline bool
dll_load (HANDLE& handle, WCHAR *name)
{
  HANDLE h = LoadLibraryW (name);
  if (!h && handle && wincap.use_dont_resolve_hack ()
      && GetLastError () == ERROR_INVALID_ADDRESS)
    h = LoadLibraryExW (name, NULL, DONT_RESOLVE_DLL_REFERENCES);
  if (!h)
    return false;
  handle = h;
  return true;
}

#define RETRY_COUNT 10

/* The standard DLL initialization routine. */
#ifdef __x86_64__

/* On x86_64, we need assembler wrappers for std_dll_init and wsock_init.
   In the x86_64 ABI it's no safe bet that frame[1] (aka 8(%rbp)) contains
   the return address.  Consequentially, if we try to overwrite frame[1]
   with the address of dll_chain, we end up with a scrambled stack, the
   result depending on the optimization settings and the current frame of
   mind of the compiler.  So for x86_64, we disable overwriting the return
   address in the real std_dll_init/wsock_init function, but rather do this
   in the wrapper, after return from the function, when we exactly know
   where the original return address is stored on the stack. */

#define INIT_WRAPPER(func) \
__asm__ ("								\n\
	.text								\n\
	.p2align 4,,15							\n\
	.seh_proc _" #func "						\n\
_" #func ":								\n\
	pushq	%rbp							\n\
	.seh_pushreg %rbp						\n\
	movq	%rsp,%rbp						\n\
	.seh_setframe %rbp,0						\n\
	subq	$0x20,%rsp						\n\
	.seh_stackalloc 32						\n\
	.seh_endprologue						\n\
	movq	0x28(%rsp),%rcx		# return address as parameter	\n\
	call	" #func "						\n\
	movdqa	%xmm0,0x10(%rsp)	# 128 bit return value in xmm0	\n\
	movq	0x10(%rsp),%rax		# copy over to %rax and %rdx	\n\
	movq	0x18(%rsp),%rdx						\n\
	leaq	dll_chain(%rip),%rcx	# load address of dll_chain	\n\
	movq	%rcx,0x28(%rsp)		# and overwrite return address	\n\
	addq	$0x20,%rsp						\n\
	popq	%rbp							\n\
	ret								\n\
	.seh_endproc							\n\
");

INIT_WRAPPER (std_dll_init)

__attribute__ ((used, noinline)) static two_addr_t
std_dll_init (struct func_info *func)
#else
__attribute__ ((used, noinline)) static two_addr_t
std_dll_init ()
#endif
{
#ifndef __x86_64__
  struct func_info *func = (struct func_info *) __builtin_return_address (0);
#endif
  struct dll_info *dll = func->dll;
  retchain ret;

  if (InterlockedIncrement (&dll->here))
    do
      {
	InterlockedDecrement (&dll->here);
	yield ();
      }
    while (InterlockedIncrement (&dll->here));
  else if ((uintptr_t) dll->handle <= 1)
    {
      fenv_t fpuenv;
      fegetenv (&fpuenv);
      WCHAR dll_path[MAX_PATH];
      DWORD err = ERROR_SUCCESS;
      int i;
      /* http://www.microsoft.com/technet/security/advisory/2269637.mspx */
      wcpcpy (wcpcpy (dll_path, windows_system_directory), dll->name);
      /* MSDN seems to imply that LoadLibrary can fail mysteriously, so,
	 since there have been reports of this in the mailing list, retry
	 several times before giving up. */
      for (i = 1; i <= RETRY_COUNT; i++)
	{
	  /* If loading the library succeeds, just leave the loop. */
	  if (!dll_load (dll->handle, dll_path))
	    break;
	  /* Otherwise check error code returned by LoadLibrary.  If the
	     error code is neither NOACCESS nor DLL_INIT_FAILED, break out
	     of the loop. */
	  err = GetLastError ();
	  if (err != ERROR_NOACCESS && err != ERROR_DLL_INIT_FAILED)
	    break;
	  if (i < RETRY_COUNT)
	    yield ();
	}
      if ((uintptr_t) dll->handle <= 1)
	{
	  /* If LoadLibrary with full path returns one of the weird errors
	     reported on the Cygwin mailing list, retry with only the DLL
	     name.  Only do this when the above retry loop has been exhausted. */
	  if (i > RETRY_COUNT && dll_load (dll->handle, dll->name))
	    /* got it with the fallback */;
	  else if ((func->decoration & 1))
	    dll->handle = INVALID_HANDLE_VALUE;
	  else
	    api_fatal ("unable to load %W, %E", dll_path);
	}
      fesetenv (&fpuenv);
    }

  /* Set "arguments" for dll_chain. */
  ret.low = (uintptr_t) dll->init;
  ret.high = (uintptr_t) func;

  InterlockedDecrement (&dll->here);

#ifndef __x86_64__
  /* Kludge alert.  Redirects the return address to dll_chain. */
  uintptr_t *volatile frame = (uintptr_t *) __builtin_frame_address (0);
  frame[1] = (uintptr_t) dll_chain;
#endif

  return ret.ll;
}

/* Initialization function for winsock stuff. */
WSADATA NO_COPY wsadata;

#ifdef __x86_64__
/* See above comment preceeding std_dll_init. */
INIT_WRAPPER (wsock_init)

__attribute__ ((used, noinline)) static two_addr_t
wsock_init (struct func_info *func)
#else
__attribute__ ((used, noinline)) static two_addr_t
wsock_init ()
#endif
{
  static LONG NO_COPY here = -1L;
#ifndef __x86_64__
  struct func_info *func = (struct func_info *) __builtin_return_address (0);
#endif
  struct dll_info *dll = func->dll;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
      yield ();
    }

  if (!wsock_started)
    {
      int __stdcall (*wsastartup) (int, WSADATA *);

      /* Don't use autoload to load WSAStartup to eliminate recursion. */
      wsastartup = (int __stdcall (*)(int, WSADATA *))
		   GetProcAddress ((HMODULE) (dll->handle), "WSAStartup");
      if (wsastartup)
	{
	  int res = wsastartup ((2<<8) | 2, &wsadata);

	  debug_printf ("res %d", res);
	  debug_printf ("wVersion %d", wsadata.wVersion);
	  debug_printf ("wHighVersion %d", wsadata.wHighVersion);
	  debug_printf ("szDescription %s", wsadata.szDescription);
	  debug_printf ("szSystemStatus %s", wsadata.szSystemStatus);
	  debug_printf ("iMaxSockets %d", wsadata.iMaxSockets);
	  debug_printf ("iMaxUdpDg %d", wsadata.iMaxUdpDg);

	  wsock_started = 1;
	}
    }

#ifndef __x86_64__
  /* Kludge alert.  Redirects the return address to dll_chain. */
  uintptr_t *volatile frame = (uintptr_t *) __builtin_frame_address (0);
  frame[1] = (uintptr_t) dll_chain;
#endif

  InterlockedDecrement (&here);

  volatile retchain ret;
  /* Set "arguments for dll_chain. */
  ret.low = (uintptr_t) dll_func_load;
  ret.high = (uintptr_t) func;
  return ret.ll;
}

LoadDLLprime (ws2_32, _wsock_init, 0)

LoadDLLfunc (CheckTokenMembership, 12, advapi32)
LoadDLLfunc (CreateProcessAsUserW, 44, advapi32)
LoadDLLfunc (DeregisterEventSource, 4, advapi32)
LoadDLLfunc (LogonUserW, 24, advapi32)
LoadDLLfunc (LookupAccountNameW, 28, advapi32)
LoadDLLfunc (LookupAccountSidW, 28, advapi32)
LoadDLLfunc (LsaClose, 4, advapi32)
LoadDLLfunc (LsaEnumerateAccountRights, 16, advapi32)
LoadDLLfunc (LsaFreeMemory, 4, advapi32)
LoadDLLfunc (LsaOpenPolicy, 16, advapi32)
LoadDLLfunc (LsaQueryInformationPolicy, 12, advapi32)
LoadDLLfunc (LsaRetrievePrivateData, 12, advapi32)
LoadDLLfunc (LsaStorePrivateData, 12, advapi32)
LoadDLLfunc (RegOpenUserClassesRoot, 16, advapi32)
LoadDLLfunc (RegOpenCurrentUser, 8, advapi32)
LoadDLLfunc (RegCloseKey, 4, advapi32)
LoadDLLfunc (RegCreateKeyExW, 36, advapi32)
LoadDLLfunc (RegEnumKeyExW, 32, advapi32)
LoadDLLfunc (RegEnumValueW, 32, advapi32)
LoadDLLfunc (RegGetKeySecurity, 16, advapi32)
LoadDLLfunc (RegOpenKeyExW, 20, advapi32)
LoadDLLfunc (RegQueryInfoKeyW, 48, advapi32)
LoadDLLfunc (RegQueryValueExW, 24, advapi32)
LoadDLLfunc (RegisterEventSourceW, 8, advapi32)
LoadDLLfunc (ReportEventW, 36, advapi32)
LoadDLLfunc (SystemFunction036, 8, advapi32)	/* Aka "RtlGenRandom" */

LoadDLLfunc (DnsQuery_A, 24, dnsapi)
LoadDLLfunc (DnsRecordListFree, 8, dnsapi)

LoadDLLfunc (GetAdaptersAddresses, 20, iphlpapi)
LoadDLLfunc (GetIfEntry, 4, iphlpapi)
LoadDLLfunc (GetIpAddrTable, 12, iphlpapi)
LoadDLLfunc (GetIpForwardTable, 12, iphlpapi)
LoadDLLfunc (GetNetworkParams, 8, iphlpapi)
LoadDLLfunc (GetUdpTable, 12, iphlpapi)

LoadDLLfuncEx (CancelSynchronousIo, 4, kernel32, 1)
LoadDLLfunc (CreateSymbolicLinkW, 12, kernel32)
LoadDLLfuncEx (GetNamedPipeClientProcessId, 8, kernel32, 1)
LoadDLLfunc (GetSystemTimePreciseAsFileTime, 4, kernel32)
LoadDLLfuncEx2 (IdnToAscii, 20, kernel32, 1, 0)
LoadDLLfuncEx2 (IdnToUnicode, 20, kernel32, 1, 0)
LoadDLLfunc (LocaleNameToLCID, 8, kernel32)

/* ldap functions are cdecl! */
#pragma push_macro ("mangle")
#undef mangle
#define mangle(name, n) #name
LoadDLLfunc (ldap_bind_s, 0, wldap32)
LoadDLLfunc (ldap_count_entries, 0, wldap32)
LoadDLLfunc (ldap_count_valuesW, 0, wldap32)
LoadDLLfunc (ldap_first_entry, 0, wldap32)
LoadDLLfunc (ldap_get_next_page_s, 0, wldap32)
LoadDLLfunc (ldap_get_valuesW, 0, wldap32)
LoadDLLfunc (ldap_get_values_lenW, 0, wldap32)
LoadDLLfunc (ldap_initW, 0, wldap32)
LoadDLLfunc (ldap_msgfree, 0, wldap32)
LoadDLLfunc (ldap_next_entry, 0, wldap32)
LoadDLLfunc (ldap_search_abandon_page, 0, wldap32)
LoadDLLfunc (ldap_search_init_pageW, 0, wldap32)
LoadDLLfunc (ldap_search_sW, 0, wldap32)
LoadDLLfunc (ldap_set_option, 0, wldap32)
LoadDLLfunc (ldap_sslinitW, 0, wldap32)
LoadDLLfunc (ldap_unbind, 0, wldap32)
LoadDLLfunc (ldap_value_freeW, 0, wldap32)
LoadDLLfunc (ldap_value_free_len, 0, wldap32)
LoadDLLfunc (LdapGetLastError, 0, wldap32)
LoadDLLfunc (LdapMapErrorToWin32, 0, wldap32)
#pragma pop_macro ("mangle")

LoadDLLfunc (WNetCloseEnum, 4, mpr)
LoadDLLfunc (WNetEnumResourceA, 16, mpr)
LoadDLLfunc (WNetGetProviderNameA, 12, mpr)
LoadDLLfunc (WNetGetResourceInformationA, 16, mpr)
LoadDLLfunc (WNetOpenEnumA, 20, mpr)

LoadDLLfunc (DsEnumerateDomainTrustsW, 16, netapi32)
LoadDLLfunc (DsGetDcNameW, 24, netapi32)
LoadDLLfunc (NetApiBufferFree, 4, netapi32)
LoadDLLfunc (NetGroupEnum, 28, netapi32)
LoadDLLfunc (NetLocalGroupEnum, 28, netapi32)
LoadDLLfunc (NetLocalGroupGetInfo, 16, netapi32)
LoadDLLfunc (NetUseGetInfo, 16, netapi32)
LoadDLLfunc (NetUserEnum, 32, netapi32)
LoadDLLfunc (NetUserGetGroups, 28, netapi32)
LoadDLLfunc (NetUserGetInfo, 16, netapi32)
LoadDLLfunc (NetUserGetLocalGroups, 32, netapi32)

LoadDLLfunc (NtCommitTransaction, 8, ntdll)
LoadDLLfunc (NtCreateTransaction, 40, ntdll)
LoadDLLfunc (NtRollbackTransaction, 8, ntdll)
LoadDLLfunc (RtlGetCurrentTransaction, 0, ntdll)
LoadDLLfunc (RtlSetCurrentTransaction, 4, ntdll)

LoadDLLfunc (CoTaskMemFree, 4, ole32)

LoadDLLfunc (LsaDeregisterLogonProcess, 4, secur32)
LoadDLLfunc (LsaFreeReturnBuffer, 4, secur32)
LoadDLLfunc (LsaLogonUser, 56, secur32)
LoadDLLfunc (LsaLookupAuthenticationPackage, 12, secur32)
LoadDLLfunc (LsaRegisterLogonProcess, 12, secur32)

LoadDLLfunc (SHGetDesktopFolder, 4, shell32)

LoadDLLfunc (CloseClipboard, 0, user32)
LoadDLLfunc (CloseDesktop, 4, user32)
LoadDLLfunc (CloseWindowStation, 4, user32)
LoadDLLfunc (CreateDesktopW, 24, user32)
LoadDLLfunc (CreateWindowExW, 48, user32)
LoadDLLfunc (CreateWindowStationW, 16, user32)
LoadDLLfunc (DefWindowProcW, 16, user32)
LoadDLLfunc (DispatchMessageW, 4, user32)
LoadDLLfunc (EmptyClipboard, 0, user32)
LoadDLLfunc (EnumWindows, 8, user32)
LoadDLLfunc (GetClipboardData, 4, user32)
LoadDLLfunc (GetForegroundWindow, 0, user32)
LoadDLLfunc (GetKeyboardLayout, 4, user32)
LoadDLLfunc (GetMessageW, 16, user32)
LoadDLLfunc (GetPriorityClipboardFormat, 8, user32)
LoadDLLfunc (GetProcessWindowStation, 0, user32)
LoadDLLfunc (GetThreadDesktop, 4, user32)
LoadDLLfunc (GetUserObjectInformationW, 20, user32)
LoadDLLfunc (GetWindowThreadProcessId, 8, user32)
LoadDLLfunc (MessageBeep, 4, user32)
LoadDLLfunc (MessageBoxW, 16, user32)
LoadDLLfunc (MsgWaitForMultipleObjectsEx, 20, user32)
LoadDLLfunc (OpenClipboard, 4, user32)
LoadDLLfunc (PeekMessageW, 20, user32)
LoadDLLfunc (PostMessageW, 16, user32)
LoadDLLfunc (PostQuitMessage, 4, user32)
LoadDLLfunc (RegisterClassW, 4, user32)
LoadDLLfunc (RegisterClipboardFormatW, 4, user32)
LoadDLLfunc (SendNotifyMessageW, 16, user32)
LoadDLLfunc (SetClipboardData, 8, user32)
LoadDLLfunc (SetParent, 8, user32)
LoadDLLfunc (SetProcessWindowStation, 4, user32)
LoadDLLfunc (SetThreadDesktop, 4, user32)

LoadDLLfuncEx3 (waveInAddBuffer, 12, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveInClose, 4, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveInGetNumDevs, 0, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveInOpen, 24, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveInPrepareHeader, 12, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveInReset, 4, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveInStart, 4, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveInUnprepareHeader, 12, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutClose, 4, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutGetNumDevs, 0, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutGetVolume, 8, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutOpen, 24, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutPrepareHeader, 12, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutReset, 4, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutSetVolume, 8, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutUnprepareHeader, 12, winmm, 1, 0, 1)
LoadDLLfuncEx3 (waveOutWrite, 12, winmm, 1, 0, 1)

LoadDLLfunc (accept, 12, ws2_32)
LoadDLLfunc (bind, 12, ws2_32)
LoadDLLfunc (closesocket, 4, ws2_32)
LoadDLLfunc (connect, 12, ws2_32)
LoadDLLfunc (FreeAddrInfoW, 4, ws2_32)
LoadDLLfunc (GetAddrInfoW, 16, ws2_32)
LoadDLLfunc (GetNameInfoW, 28, ws2_32)
LoadDLLfunc (gethostbyaddr, 12, ws2_32)
LoadDLLfunc (gethostbyname, 4, ws2_32)
LoadDLLfunc (gethostname, 8, ws2_32)
LoadDLLfunc (getpeername, 12, ws2_32)
LoadDLLfunc (getprotobyname, 4, ws2_32)
LoadDLLfunc (getprotobynumber, 4, ws2_32)
LoadDLLfunc (getservbyname, 8, ws2_32)
LoadDLLfunc (getservbyport, 8, ws2_32)
LoadDLLfunc (getsockname, 12, ws2_32)
LoadDLLfunc (getsockopt, 20, ws2_32)
LoadDLLfunc (ioctlsocket, 12, ws2_32)
LoadDLLfunc (listen, 8, ws2_32)
LoadDLLfunc (setsockopt, 20, ws2_32)
LoadDLLfunc (shutdown, 8, ws2_32)
LoadDLLfunc (socket, 12, ws2_32)
LoadDLLfunc (WSAAsyncSelect, 16, ws2_32)
LoadDLLfunc (WSADuplicateSocketW, 12, ws2_32)
LoadDLLfunc (WSAEnumNetworkEvents, 12, ws2_32)
LoadDLLfunc (WSAEventSelect, 12, ws2_32)
LoadDLLfunc (WSAGetLastError, 0, ws2_32)
LoadDLLfunc (WSAIoctl, 36, ws2_32)
LoadDLLfunc (WSARecv, 28, ws2_32)
LoadDLLfunc (WSARecvFrom, 36, ws2_32)
LoadDLLfunc (WSASendMsg, 24, ws2_32)
LoadDLLfunc (WSASendTo, 36, ws2_32)
LoadDLLfunc (WSASetLastError, 4, ws2_32)
LoadDLLfunc (WSASocketW, 24, ws2_32)
// LoadDLLfunc (WSAStartup, 8, ws2_32)
LoadDLLfunc (WSAWaitForMultipleEvents, 20, ws2_32)
}
