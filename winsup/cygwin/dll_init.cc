/* dll_init.cc

   Copyright 1998, 1999, 2000 Cygnus Solutions.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include "winsup.h"
#include "exceptions.h"
#include "dll_init.h"

extern void __stdcall check_sanity_and_sync (per_process *);

#ifdef _MT_SAFE
extern ResourceLocks _reslock NO_COPY;
extern MTinterface _mtinterf NO_COPY;
#endif /*_MT_SAFE*/

/* WARNING: debug can't be called before init !!!! */

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// the private structure

typedef enum   { NONE, LINK, LOAD } dllType;

struct dll
{
  per_process p;
  HMODULE handle;
  const char *name;
  dllType type;
};

//-----------------------------------------------------------------------------

#define MAX_DLL_BEFORE_INIT	100 // FIXME: enough ???
static dll _list_before_init[MAX_DLL_BEFORE_INIT];

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// local variables

static DllList _the;
static int _last = 0;
static int _max = MAX_DLL_BEFORE_INIT;
static dll *_list = _list_before_init;
static int _initCalled = 0;
static int _numberOfOpenedDlls = 0;
static int _forkeeMustReloadDlls = 0;
static int _in_forkee = 0;
static const char *_dlopenedLib = 0;
static int _dlopenIndex = -1;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static int __dll_global_dtors_recorded = 0;

static void
__dll_global_dtors()
{
  _the.doGlobalDestructorsOfDlls();
}

static void
doGlobalCTORS (per_process *p)
{
  void (**pfunc)() = p->ctors;

  /* Run ctors backwards, so skip the first entry and find how many
    there are, then run them.  */

  if (pfunc)
    {
      int i;
      for (i = 1; pfunc[i]; i++);

      for (int j = i - 1; j > 0; j-- )
	(pfunc[j]) ();
    }
}

static void
doGlobalDTORS (per_process *p)
{
  if (!p)
    return;
  void (**pfunc)() = p->dtors;
  for (int i = 1; pfunc[i]; i++)
    (pfunc[i]) ();
}

#define INC 500

static int
add (HMODULE h, char *name, per_process *p, dllType type)
{
  int ret = -1;

  if (p)
    check_sanity_and_sync (p);

  if (_last == _max)
    {
      if (!_initCalled) // we try to load more than MAX_DLL_BEFORE_INIT
	{
	  small_printf ("try to load more dll than max allowed=%d\n",
		       MAX_DLL_BEFORE_INIT);
	  ExitProcess (1);
	}

      dll* newArray = new dll[_max+INC];
      if (_list)
	{
	  memcpy (newArray, _list, _max * sizeof (dll));
	  if (_list != _list_before_init)
	    delete []_list;
	}
      _list = newArray;
      _max += INC;
    }

  _list[_last].name = name && type == LOAD ? strdup (name) : NULL;
  _list[_last].handle = h;
  _list[_last].p = *p;
  _list[_last].type = type;

  ret = _last++;
  return ret;
}

static int
initOneDll (per_process *p)
{
  /* global variable user_data must be initialized */

  /* FIXME: init environment (useful?) */
  *(p->envptr) = *(user_data->envptr);

  /* FIXME: need other initializations? */

  int ret = 1;
  if (!_in_forkee)
    {
      /* global contructors */
      doGlobalCTORS (p);

      /* entry point of dll (use main of per_process with null args...) */
      if (p->main)
	ret = (*(p->main)) (0, 0, 0);
    }

  return ret;
}

DllList&
DllList::the ()
{
  return _the;
}

void
DllList::currentDlOpenedLib (const char *name)
{
  if (_dlopenedLib != 0)
    small_printf ("WARNING: previous dlopen of %s wasn't correctly performed\n", _dlopenedLib);
  _dlopenedLib = name;
  _dlopenIndex = -1;
}

int
DllList::recordDll (HMODULE h, per_process *p)
{
  int ret = -1;

  /* debug_printf ("Record a dll p=%p\n", p); see WARNING */
  dllType type = LINK;
  if (_initCalled)
    {
      type = LOAD;
      _numberOfOpenedDlls++;
      forkeeMustReloadDlls (1);
    }

  if (_in_forkee)
    {
      ret = 0;		// Just a flag
      goto out;
    }

  char buf[MAX_PATH];
  GetModuleFileName (h, buf, MAX_PATH);

  if (type == LOAD && _dlopenedLib !=0)
    {
    // it is not the current dlopened lib
    // so we insert one empty lib to preserve place for current dlopened lib
    if (!strcasematch (_dlopenedLib, buf))
      {
      if (_dlopenIndex == -1)
	_dlopenIndex = add (0, 0, 0, NONE);
      ret = add (h, buf, p, type);
      }
    else // it is the current dlopened lib
      {
	if (_dlopenIndex != -1)
	  {
	    _list[_dlopenIndex].handle = h;
	    _list[_dlopenIndex].p = *p;
	    _list[_dlopenIndex].type = type;
	    ret = _dlopenIndex;
	    _dlopenIndex = -1;
	  }
	else // it this case the dlopened lib doesn't need other lib
	  ret = add (h, buf, p, type);
	_dlopenedLib = 0;
      }
    }
  else
    ret = add (h, buf, p, type);

out:
  if (_initCalled) // main module is already initialized
    {
      if (!initOneDll (p))
	ret = -1;
    }
  return ret;
}

void
DllList::detachDll (int dll_index)
{
  if (dll_index != -1)
    {
      dll *aDll = &(_list[dll_index]);
      doGlobalDTORS (&aDll->p);
      if (aDll->type == LOAD)
	_numberOfOpenedDlls--;
      aDll->type = NONE;
    }
  else
    small_printf ("WARNING: try to detach an already detached dll ...\n");
}

void
DllList::initAll ()
{
  // init for destructors
  // because initAll isn't called in forked process, this exit function will
  // be recorded only once
  if (!__dll_global_dtors_recorded)
    {
      atexit (__dll_global_dtors);
      __dll_global_dtors_recorded = 1;
    }

  if (!_initCalled)
    {
      debug_printf ("call to DllList::initAll");
      for (int i = 0; i < _last; i++)
	{
	  per_process *p = &_list[i].p;
	  if (p)
	    initOneDll (p);
	}
      _initCalled = 1;
    }
}

void
DllList::doGlobalDestructorsOfDlls ()
{
  // global destructors in reverse order
  for (int i = _last - 1; i >= 0; i--)
    {
      if (_list[i].type != NONE)
	{
	  per_process *p = &_list[i].p;
	  if (p)
	    doGlobalDTORS (p);
	}
    }
}

int
DllList::numberOfOpenedDlls ()
{
  return _numberOfOpenedDlls;
}

int
DllList::forkeeMustReloadDlls ()
{
  return _forkeeMustReloadDlls;
}

void
DllList::forkeeMustReloadDlls (int i)
{
  _forkeeMustReloadDlls = i;
}

#define A64K (64 * 1024)

/* Mark every memory address up to "here" as reserved.  This may force
   Windows NT to load a DLL in the next available, lowest slot. */
void
reserve_upto (const char *name, DWORD here)
{
  DWORD size;
  MEMORY_BASIC_INFORMATION mb;
  for (DWORD start = 0x10000; start < here; start += size)
    if (!VirtualQuery ((void *) start, &mb, sizeof (mb)))
      size = 64 * 1024;
    else
      {
	size = A64K * ((mb.RegionSize + A64K - 1) / A64K);
	start = A64K * (((DWORD) mb.BaseAddress + A64K - 1) / A64K);

	if (start + size > here)
	  size = here - start;
	if (mb.State == MEM_FREE &&
	    !VirtualAlloc ((void *) start, size, MEM_RESERVE, PAGE_NOACCESS))
	  api_fatal ("couldn't allocate memory %p(%d) for '%s' alignment, %E\n",
		     start, size, name);
      }
}

/* Release all of the memory previously allocated by "upto" above.
   Note that this may also free otherwise reserved memory.  If that becomes
   a problem, we'll have to keep track of the memory that we reserve above. */
void
release_upto (const char *name, DWORD here)
{
  DWORD size;
  MEMORY_BASIC_INFORMATION mb;
  for (DWORD start = 0x10000; start < here; start += size)
    if (!VirtualQuery ((void *) start, &mb, sizeof (mb)))
      size = 64 * 1024;
    else
      {
	size = mb.RegionSize;
	if (!(mb.State == MEM_RESERVE && mb.AllocationProtect == PAGE_NOACCESS &&
	    ((void *) start < user_data->heapbase || (void *) start > user_data->heaptop)))
	  continue;
	if (!VirtualFree ((void *) start, 0, MEM_RELEASE))
	  api_fatal ("couldn't release memory %p(%d) for '%s' alignment, %E\n",
		     start, size, name);
      }
}

/* Reload DLLs after a fork.  Iterates over the list of dynamically loaded DLLs
   and attempts to load them in the same place as they were loaded in the parent. */
void
DllList::forkeeLoadDlls ()
{
  _initCalled = 1;
  _in_forkee = 1;
  int try2 = 0;
  for (int i = 0; i < _last; i++)
    if (_list[i].type == LOAD)
      {
	const char *name = _list[i].name;
	HMODULE handle = _list[i].handle;
	HMODULE h = LoadLibraryEx (name, NULL, DONT_RESOLVE_DLL_REFERENCES);

	if (h == handle)
	  {
	    FreeLibrary (h);
	    LoadLibrary (name);
	  }
	else if (try2)
	  api_fatal ("unable to remap %s to same address as parent -- %p", name, h);
	else
	  {
	    FreeLibrary (h);
	    reserve_upto (name, (DWORD) handle);
	    try2 = 1;
	    i--;
	    continue;
	  }
	if (try2)
	  {
	    release_upto (name, (DWORD) handle);
	    try2 = 0;
	  }
      }
  _in_forkee = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// iterators

DllListIterator::DllListIterator (int type) : _type (type), _index (-1)
{
  operator++ ();
}

DllListIterator::~DllListIterator ()
{
}

DllListIterator::operator per_process* ()
{
  return &_list[index ()].p;
}

void
DllListIterator::operator++ ()
{
  _index++;
  while (_index < _last && (int) (_list[_index].type) != _type)
    _index++;
  if (_index == _last)
    _index = -1;
}

LinkedDllIterator::LinkedDllIterator () : DllListIterator ((int) LINK)
{
}

LinkedDllIterator::~LinkedDllIterator ()
{
}

LoadedDllIterator::LoadedDllIterator () : DllListIterator ((int) LOAD)
{
}

LoadedDllIterator::~LoadedDllIterator ()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// the extern symbols

extern "C"
{
  /* This is an exported copy of environ which can be used by DLLs
     which use cygwin.dll.  */
  extern struct _reent reent_data;
};

extern "C"
int
dll_dllcrt0 (HMODULE h, per_process *p)
{
  struct _reent reent_data;
  if (p == NULL)
    p = &__cygwin_user_data;
  else
    *(p->impure_ptr_ptr) = &reent_data;

  /* Partially initialize Cygwin guts for non-cygwin apps. */
  if (dynamically_loaded && user_data->magic_biscuit == 0)
    dll_crt0 (p);
  return _the.recordDll (h, p);
}

/* OBSOLETE: This function is obsolescent and will go away in the
   future.  Cygwin can now handle being loaded from a noncygwin app
   using the same entry point. */

extern "C"
int
dll_noncygwin_dllcrt0 (HMODULE h, per_process *p)
{
  return dll_dllcrt0 (h, p);
}

extern "C"
void
cygwin_detach_dll (int dll_index)
{
  _the.detachDll (dll_index);
}

extern "C"
void
dlfork (int val)
{
  _the.forkeeMustReloadDlls (val);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
