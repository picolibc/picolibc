/* _cygwin_crt0_common.cc: common crt0 function for cygwin crt0's.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "crt0.h"
#include "cygwin-cxx.h"

/* Weaken these declarations so the references don't pull in C++ dependencies
   unnecessarily.  */
#define WEAK __attribute__ ((weak))

#ifdef __x86_64__
#define REAL_ZNWX		"__real__Znwm"
#define REAL_ZNAX		"__real__Znam"
#define REAL_ZNWX_NOTHROW_T	"__real__ZnwmRKSt9nothrow_t"
#define REAL_ZNAX_NOTHROW_T	"__real__ZnamRKSt9nothrow_t"
#else
#define REAL_ZNWX		"___real__Znwj"
#define REAL_ZNAX		"___real__Znaj"
#define REAL_ZNWX_NOTHROW_T	"___real__ZnwjRKSt9nothrow_t"
#define REAL_ZNAX_NOTHROW_T	"___real__ZnajRKSt9nothrow_t"
#endif
#define REAL_ZDLPV		_SYMSTR (__real__ZdlPv)
#define REAL_ZDAPV		_SYMSTR (__real__ZdaPv)
#define REAL_ZDLPV_NOTHROW_T	_SYMSTR (__real__ZdlPvRKSt9nothrow_t)
#define REAL_ZDAPV_NOTHROW_T	_SYMSTR (__real__ZdaPvRKSt9nothrow_t)

/* Use asm names to bypass the --wrap that is being applied to redirect all other
   references to these operators toward the redirectors in the Cygwin DLL; this
   way we can record what definitions were visible at final link time but still
   send all calls to the redirectors.  */
extern WEAK void *operator new(std::size_t sz) throw (std::bad_alloc)
			__asm__ (REAL_ZNWX);
extern WEAK void *operator new[](std::size_t sz) throw (std::bad_alloc)
			__asm__ (REAL_ZNAX);
extern WEAK void operator delete(void *p) throw()
			__asm__ (REAL_ZDLPV);
extern WEAK void operator delete[](void *p) throw()
			__asm__ (REAL_ZDAPV);
extern WEAK void *operator new(std::size_t sz, const std::nothrow_t &nt) throw()
			__asm__ (REAL_ZNWX_NOTHROW_T);
extern WEAK void *operator new[](std::size_t sz, const std::nothrow_t &nt) throw()
			__asm__ (REAL_ZNAX_NOTHROW_T);
extern WEAK void operator delete(void *p, const std::nothrow_t &nt) throw()
			__asm__ (REAL_ZDLPV_NOTHROW_T);
extern WEAK void operator delete[](void *p, const std::nothrow_t &nt) throw()
			__asm__ (REAL_ZDAPV_NOTHROW_T);

/* Avoid an info message from linker when linking applications.  */
extern __declspec(dllimport) struct _reent *_impure_ptr;

/* Initialised in _cygwin_dll_entry. */
extern int __dynamically_loaded;

#undef environ

extern "C"
{
#ifdef __i386__
char **environ;
#endif
int _fmode;

extern char __RUNTIME_PSEUDO_RELOC_LIST__;
extern char __RUNTIME_PSEUDO_RELOC_LIST_END__;
#ifdef __x86_64__
extern char __image_base__;
#define _image_base__ __image_base__
#else
extern char _image_base__;
#endif

struct per_process_cxx_malloc __cygwin_cxx_malloc =
{
  &(operator new), &(operator new[]),
  &(operator delete), &(operator delete[]),
  &(operator new), &(operator new[]),
  &(operator delete), &(operator delete[])
};

/* Set up pointers to various pieces so the dll can then use them,
   and then jump to the dll.  */

int __stdcall
_cygwin_crt0_common (MainFunc f, per_process *u)
{
  per_process *newu = (per_process *) cygwin_internal (CW_USER_DATA);
  bool uwasnull;

  /* u is non-NULL if we are in a DLL, and NULL in the main exe.
     newu is the Cygwin DLL's internal per_process and never NULL.  */
  if (u != NULL)
    uwasnull = false;	/* Caller allocated space for per_process structure.  */
  else
    {
      u = newu;		/* Using DLL built-in per_process.  */
      uwasnull = true;	/* Remember for later.  */
    }

  /* The version numbers are the main source of compatibility checking.
     As a backup to them, we use the size of the per_process struct.  */
  u->magic_biscuit = sizeof (per_process);

  /* cygwin.dll version number in effect at the time the app was created.  */
  u->dll_major = CYGWIN_VERSION_DLL_MAJOR;
  u->dll_minor = CYGWIN_VERSION_DLL_MINOR;
  u->api_major = CYGWIN_VERSION_API_MAJOR;
  u->api_minor = CYGWIN_VERSION_API_MINOR;

  u->ctors = &__CTOR_LIST__;
  u->dtors = &__DTOR_LIST__;
#ifdef __i386__
  u->envptr = &environ;
#endif
  if (uwasnull)
    _impure_ptr = u->impure_ptr;	/* Use field initialized in newer DLLs. */
  else
    u->impure_ptr_ptr = &_impure_ptr;	/* Older DLLs need this. */

  u->main = f;

  /* These functions are executed prior to main.  They are just stubs unless the
     user overrides them. */
  u->premain[0] = cygwin_premain0;
  u->premain[1] = cygwin_premain1;
  u->premain[2] = cygwin_premain2;
  u->premain[3] = cygwin_premain3;
  u->fmode_ptr = &_fmode;

  /* Unused */
  u->initial_sp = NULL;

  /* Remember whatever the user linked his application with - or
     point to entries in the dll.  */
  u->malloc = &malloc;
  u->free = &free;
  u->realloc = &realloc;
  u->calloc = &calloc;
  u->posix_memalign = &posix_memalign;

  /* Likewise for the C++ memory operators, if any, but not if we
     were dlopen()'d, as we might get dlclose()'d and that would
     leave stale function pointers behind.    */
  if (newu && newu->cxx_malloc && !__dynamically_loaded)
    {
      /* Inherit what we don't override.  */
#define CONDITIONALLY_OVERRIDE(MEMBER) \
      if (!__cygwin_cxx_malloc.MEMBER) \
	__cygwin_cxx_malloc.MEMBER = newu->cxx_malloc->MEMBER;
      CONDITIONALLY_OVERRIDE(oper_new);
      CONDITIONALLY_OVERRIDE(oper_new__);
      CONDITIONALLY_OVERRIDE(oper_delete);
      CONDITIONALLY_OVERRIDE(oper_delete__);
      CONDITIONALLY_OVERRIDE(oper_new_nt);
      CONDITIONALLY_OVERRIDE(oper_new___nt);
      CONDITIONALLY_OVERRIDE(oper_delete_nt);
      CONDITIONALLY_OVERRIDE(oper_delete___nt);
      /* Now update the resulting set into the global redirectors.  */
      *newu->cxx_malloc = __cygwin_cxx_malloc;
    }

  /* Setup the module handle so fork can get the path name.  */
  u->hmodule = GetModuleHandle (0);

  /* variables for fork */
#ifdef __x86_64__
  u->data_start = &__data_start__;
  u->data_end = &__data_end__;
  u->bss_start = &__bss_start__;
  u->bss_end = &__bss_end__;
#else
  u->data_start = &_data_start__;
  u->data_end = &_data_end__;
  u->bss_start = &_bss_start__;
  u->bss_end = &_bss_end__;
#endif
  u->pseudo_reloc_start = &__RUNTIME_PSEUDO_RELOC_LIST__;
  u->pseudo_reloc_end = &__RUNTIME_PSEUDO_RELOC_LIST_END__;
  u->image_base = &_image_base__;
  /* This is actually a dummy call to force the linker to load this
     symbol for older apps which need it.  */
  _pei386_runtime_relocator (NULL);
  return 1;
}
} /* "C" */
