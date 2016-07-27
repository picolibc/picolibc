/* miscfuncs.h: main Cygwin header file.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _MISCFUNCS_H
#define _MISCFUNCS_H

#define likely(X) __builtin_expect (!!(X), 1)
#define unlikely(X) __builtin_expect (!!(X), 0)

extern "C" int getentropy (void *ptr, size_t len);

int __reg1 winprio_to_nice (DWORD);
DWORD __reg1 nice_to_winprio (int &);

bool __reg3 create_pipe (PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD);

BOOL WINAPI CreatePipeOverlapped (PHANDLE read_handle, PHANDLE write_handle,
				  LPSECURITY_ATTRIBUTES sa);
BOOL WINAPI ReadPipeOverlapped (HANDLE h, PVOID buf, DWORD len,
				LPDWORD ret_len, DWORD timeout);
BOOL WINAPI WritePipeOverlapped (HANDLE h, LPCVOID buf, DWORD len,
				 LPDWORD ret_len, DWORD timeout);

/* class for per-line reading using native functions.  The caller provides
   the file as an POBJECT_ATTRIBUTES, and the buffer space. */
class NT_readline
{
  HANDLE fh;
  PCHAR buf;
  PCHAR got;
  PCHAR end;
  ULONG buflen;
  ULONG len;
  ULONG line;
public:
  NT_readline () : fh (NULL) {}
  bool init (POBJECT_ATTRIBUTES attr, char *buf, ULONG buflen);
  PCHAR gets ();
  void close () { if (fh) NtClose (fh); fh = NULL; }
  ~NT_readline () { close (); }
};

extern "C" void yield ();

#define import_address(x) __import_address ((void *)(x))
void * __reg1 __import_address (void *);
 
#define caller_return_address() \
		__caller_return_address (__builtin_return_address (0))
void * __reg1 __caller_return_address (void *);

void backslashify (const char *, char *, bool);
void slashify (const char *, char *, bool);
#define isslash(c) ((c) == '/')

extern void transform_chars (PWCHAR, PWCHAR);
extern inline void
transform_chars (PUNICODE_STRING upath, USHORT start_idx)
{
  transform_chars (upath->Buffer + start_idx,
		   upath->Buffer + upath->Length / sizeof (WCHAR) - 1);
}

/* Memory checking */
int __reg2 check_invalid_virtual_addr (const void *s, unsigned sz);

ssize_t __reg3 check_iovec (const struct iovec *, int, bool);
#define check_iovec_for_read(a, b) check_iovec ((a), (b), false)
#define check_iovec_for_write(a, b) check_iovec ((a), (b), true)

#ifdef __x86_64__
extern PVOID create_new_main_thread_stack (PVOID &allocationbase,
					   SIZE_T parent_commitsize);
#endif

extern "C" DWORD WINAPI pthread_wrapper (PVOID arg);
extern "C" HANDLE WINAPI CygwinCreateThread (LPTHREAD_START_ROUTINE thread_func,
					     PVOID thread_arg, PVOID stackaddr,
					     ULONG stacksize, ULONG guardsize,
					     DWORD creation_flags,
					     LPDWORD thread_id);

void SetThreadName (DWORD dwThreadID, const char* threadName);

#endif /*_MISCFUNCS_H*/
