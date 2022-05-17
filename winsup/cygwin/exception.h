/* exception.h

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#define exception_list void
typedef struct _DISPATCHER_CONTEXT *PDISPATCHER_CONTEXT;

class exception
{
  static EXCEPTION_DISPOSITION myfault (EXCEPTION_RECORD *, exception_list *,
					CONTEXT *, PDISPATCHER_CONTEXT);
  static EXCEPTION_DISPOSITION handle (EXCEPTION_RECORD *, exception_list *,
				       CONTEXT *, PDISPATCHER_CONTEXT);
public:
  exception () __attribute__ ((always_inline))
  {
    /* Install SEH handler. */
    asm volatile ("\n\
    1:									\n\
      .seh_handler							  \
	_ZN9exception6handleEP17_EXCEPTION_RECORDPvP8_CONTEXTP19_DISPATCHER_CONTEXT,	  \
	@except								\n\
      .seh_handlerdata							\n\
      .long 1								\n\
      .rva 1b, 2f, 2f, 2f						\n\
      .seh_code								\n");
  };
  ~exception () __attribute__ ((always_inline))
  {
    asm volatile ("\n\
      nop								\n\
    2:									\n\
      nop								\n");
  }
};

LONG CALLBACK myfault_altstack_handler (EXCEPTION_POINTERS *);

class cygwin_exception
{
  PUINT_PTR framep;
  PCONTEXT ctx;
  EXCEPTION_RECORD *e;
  HANDLE h;
  void dump_exception ();
  void open_stackdumpfile ();
public:
  cygwin_exception (PUINT_PTR in_framep, PCONTEXT in_ctx = NULL, EXCEPTION_RECORD *in_e = NULL):
    framep (in_framep), ctx (in_ctx), e (in_e), h (NULL) {}
  void dumpstack ();
  PCONTEXT context () const {return ctx;}
  EXCEPTION_RECORD *exception_record () const {return e;}
};
