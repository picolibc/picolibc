/* cxx.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#if (__GNUC__ >= 3)

#include "winsup.h"
#include "cygwin-cxx.h"

/* These implementations of operators new and delete are used internally by
   the DLL, and are kept separate from the user's/libstdc++'s versions by
   use of LD's --wrap option.  */

void *
operator new (std::size_t s)
{
  void *p = calloc (1, s);
  return p;
}

void
operator delete (void *p)
{
  free (p);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++14-compat"
void
operator delete (void *p, size_t)
{
  ::operator delete(p);
}
#pragma GCC diagnostic pop

void *
operator new[] (std::size_t s)
{
  return ::operator new (s);
}

void
operator delete[] (void *p)
{
  ::operator delete (p);
}

/* Nothrow versions, provided only for completeness in the fallback array.  */

void *
operator new (std::size_t s, const std::nothrow_t &)
{
  void *p = calloc (1, s);
  return p;
}

void
operator delete (void *p, const std::nothrow_t &)
{
  free (p);
}

void *
operator new[] (std::size_t s, const std::nothrow_t &nt)
{
  return ::operator new (s, nt);
}

void
operator delete[] (void *p, const std::nothrow_t &nt)
{
  ::operator delete (p, nt);
}


extern "C" void
__cxa_pure_virtual (void)
{
  api_fatal ("pure virtual method called");
}

extern "C" void
__cxa_guard_acquire ()
{
}

extern "C" void
__cxa_guard_release ()
{
}

/* These routines are made available as last-resort fallbacks
   for the application.  Should not be used in practice; the
   entries in this struct get overwritten by each DLL as it
   is loaded, and libstdc++ will override the whole lot first
   thing of all.   */

struct per_process_cxx_malloc default_cygwin_cxx_malloc =
{
  &(operator new),
  &(operator new[]),
  &(operator delete),
  &(operator delete[]),
  &(operator new),
  &(operator new[]),
  &(operator delete),
  &(operator delete[]),
};


#endif
