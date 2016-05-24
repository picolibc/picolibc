/* dll_init.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

struct per_module
{
#ifndef __x86_64__
  char ***envptr;
#endif
  void (**ctors)(void);
  void (**dtors)(void);
  void *data_start;
  void *data_end;
  void *bss_start;
  void *bss_end;
  int (*main)(int, char **, char **);
  per_module &operator = (per_process *p)
  {
#ifndef __x86_64__
    envptr = p->envptr;
#endif
    ctors = p->ctors;
    dtors = p->dtors;
    data_start = p->data_start;
    data_end = p->data_end;
    bss_start = p->bss_start;
    bss_end = p->bss_end;
    main = p->main;
    return *this;
  }
  void run_ctors ();
  void run_dtors ();
};


typedef enum
{
  DLL_NONE,
  DLL_LINK,
  DLL_LOAD,
  DLL_ANY
} dll_type;

struct dll
{
  struct dll *next, *prev;
  per_module p;
  HMODULE handle;
  int count;
  bool has_dtors;
  dll_type type;
  long ndeps;
  dll** deps;
  DWORD image_size;
  void* preferred_base;
  PWCHAR modname;
  WCHAR name[1];
  void detach ();
  int init ();
  void run_dtors ()
  {
    if (has_dtors)
      {
	has_dtors = 0;
	p.run_dtors ();
      }
  }
};

#define MAX_DLL_BEFORE_INIT     100

class dll_list
{
  dll *end;
  dll *hold;
  dll_type hold_type;
  static muto protect;
public:
  dll start;
  int loaded_dlls;
  int reload_on_fork;
  dll *operator [] (const PWCHAR name);
  dll *alloc (HINSTANCE, per_process *, dll_type);
  dll *find (void *);
  void detach (void *);
  void init ();
  void load_after_fork (HANDLE);
  void reserve_space ();
  void load_after_fork_impl (HANDLE, dll* which, int retries);
  dll *find_by_modname (const PWCHAR name);
  void populate_deps (dll* d);
  void topsort ();
  void topsort_visit (dll* d, bool goto_tail);
  void append (dll* d);

  dll *inext ()
  {
    while ((hold = hold->next))
      if (hold_type == DLL_ANY || hold->type == hold_type)
	break;
    return hold;
  }

  dll *istart (dll_type t)
  {
    hold_type = t;
    hold = &start;
    return inext ();
  }
  void guard(bool lockit)
  {
    if (lockit)
      protect.acquire ();
    else
      protect.release ();
  }
  friend void dll_global_dtors ();
  dll_list () { protect.init ("dll_list"); }
};

/* References:
   http://msdn.microsoft.com/en-us/windows/hardware/gg463125
   http://msdn.microsoft.com/en-us/library/ms809762.aspx
*/
struct pefile
{
  IMAGE_DOS_HEADER dos_hdr;

  char* rva (ptrdiff_t offset) { return (char*) this + offset; }
  PIMAGE_NT_HEADERS pe_hdr () { return (PIMAGE_NT_HEADERS) rva (dos_hdr.e_lfanew); }
  PIMAGE_OPTIONAL_HEADER optional_hdr () { return &pe_hdr ()->OptionalHeader; }
  PIMAGE_DATA_DIRECTORY idata_dir (DWORD which)
  {
    PIMAGE_OPTIONAL_HEADER oh = optional_hdr ();
    return (which < oh->NumberOfRvaAndSizes)? oh->DataDirectory + which : 0;
  }
};

extern dll_list dlls;
void dll_global_dtors ();

/* These probably belong in a newlib header but we can keep them here
   for now.  */
extern "C" int __cxa_atexit(void (*)(void*), void*, void*);
extern "C" int __cxa_finalize(void*);
