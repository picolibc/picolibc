/* libstdcxx_wrapper.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details.  */


/* We provide these stubs to call into a user's
   provided ONDEE replacement if there is one - otherwise
   it must fall back to the standard libstdc++ version.  */

#include "winsup.h"
#include "cygwin-cxx.h"
#include "perprocess.h"

/* We are declaring asm names for the functions we define here, as we want
   to define the wrappers in this file.  GCC links everything with wrappers
   around the standard C++ memory management operators; these are the wrappers,
   but we want the compiler to know they are the malloc operators and not have
   it think they're just any old function matching 'extern "C" _wrap_*'.  */
#define MANGLED_ZNWX			"__wrap__Znwm"
#define MANGLED_ZNAX			"__wrap__Znam"
#define MANGLED_ZNWX_NOTHROW_T		"__wrap__ZnwmRKSt9nothrow_t"
#define MANGLED_ZNAX_NOTHROW_T		"__wrap__ZnamRKSt9nothrow_t"

extern void *operator new(std::size_t sz) noexcept (false)
			__asm__ (MANGLED_ZNWX);
extern void *operator new[](std::size_t sz) noexcept (false)
			__asm__ (MANGLED_ZNAX);
extern void operator delete(void *p) noexcept (true)
			__asm__ (_SYMSTR (__wrap__ZdlPv));
extern void operator delete[](void *p) noexcept (true)
			__asm__ (_SYMSTR (__wrap__ZdaPv));
extern void *operator new(std::size_t sz, const std::nothrow_t &nt) noexcept (true)
			__asm__ (MANGLED_ZNWX_NOTHROW_T);
extern void *operator new[](std::size_t sz, const std::nothrow_t &nt) noexcept (true)
			__asm__ (MANGLED_ZNAX_NOTHROW_T);
extern void operator delete(void *p, const std::nothrow_t &nt) noexcept (true)
			__asm__ (_SYMSTR (__wrap__ZdlPvRKSt9nothrow_t));
extern void operator delete[](void *p, const std::nothrow_t &nt) noexcept (true)
			__asm__ (_SYMSTR (__wrap__ZdaPvRKSt9nothrow_t));

extern void *
operator new(std::size_t sz) noexcept (false)
{
  return (*user_data->cxx_malloc->oper_new) (sz);
}

extern void *
operator new[](std::size_t sz) noexcept (false)
{
  return (*user_data->cxx_malloc->oper_new__) (sz);
}

extern void
operator delete(void *p) noexcept (true)
{
  (*user_data->cxx_malloc->oper_delete) (p);
}

extern void
operator delete[](void *p) noexcept (true)
{
  (*user_data->cxx_malloc->oper_delete__) (p);
}

extern void *
operator new(std::size_t sz, const std::nothrow_t &nt) noexcept (true)
{
  return (*user_data->cxx_malloc->oper_new_nt) (sz, nt);
}

extern void *
operator new[](std::size_t sz, const std::nothrow_t &nt) noexcept (true)
{
  return (*user_data->cxx_malloc->oper_new___nt) (sz, nt);
}

extern void
operator delete(void *p, const std::nothrow_t &nt) noexcept (true)
{
  (*user_data->cxx_malloc->oper_delete_nt) (p, nt);
}

extern void
operator delete[](void *p, const std::nothrow_t &nt) noexcept (true)
{
  (*user_data->cxx_malloc->oper_delete___nt) (p, nt);
}

