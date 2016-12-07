/* dll_init.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

struct per_module
{
#ifdef __i386__
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
#ifdef __i386__
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
  DLL_SELF, /* main-program.exe, cygwin1.dll */
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
  FILE_BASIC_INFORMATION fbi;
  FILE_INTERNAL_INFORMATION fii;
  PWCHAR forkable_ntname;
  WCHAR ntname[1]; /* must be the last data member */

  void detach ();
  int init ();
  void nominate_forkable (PCWCHAR);
  bool create_forkable ();
  void run_dtors ()
  {
    if (has_dtors)
      {
	has_dtors = 0;
	p.run_dtors ();
      }
  }
  PWCHAR forkedntname ()
  {
    return forkable_ntname && *forkable_ntname ? forkable_ntname : ntname;
  }
};

#define MAX_DLL_BEFORE_INIT     100

class dll_list
{
  /* forkables */
  enum
    {
      forkables_unknown,
      forkables_impossible,
      forkables_disabled,
      forkables_needless,
      forkables_needed,
      forkables_created,
    }
    forkables_needs;
  DWORD forkables_dirx_size;
  PWCHAR forkables_dirx_ntname;
  PWCHAR forkables_mutex_name;
  HANDLE forkables_mutex;
  void track_self ();
  dll *find_by_forkedntname (PCWCHAR ntname);
  size_t forkable_ntnamesize (dll_type, PCWCHAR fullntname, PCWCHAR modname);
  void prepare_forkables_nomination ();
  void update_forkables_needs ();
  bool update_forkables ();
  bool create_forkables ();
  void denominate_forkables ();
  bool close_mutex ();
  void try_remove_forkables (PWCHAR dirbuf, size_t dirlen, size_t dirbufsize);
  void set_forkables_inheritance (bool);
  void request_forkables ();

  dll *end;
  dll *hold;
  dll_type hold_type;
  static muto protect;
  /* Use this buffer under loader lock conditions only. */
  static WCHAR NO_COPY nt_max_path_buffer[NT_MAX_PATH];
public:
  static HANDLE ntopenfile (PCWCHAR ntname, NTSTATUS *pstatus = NULL,
			    ULONG openopts = 0, ACCESS_MASK access = 0,
			    HANDLE rootDir = NULL);
  static bool read_fii (HANDLE fh, PFILE_INTERNAL_INFORMATION pfii);
  static bool read_fbi (HANDLE fh, PFILE_BASIC_INFORMATION pfbi);
  static PWCHAR form_ntname (PWCHAR ntbuf, size_t bufsize, PCWCHAR name);
  static PWCHAR form_shortname (PWCHAR shortbuf, size_t bufsize, PCWCHAR name);
  static PWCHAR nt_max_path_buf ()
  {
    return nt_max_path_buffer;
  }
  static PCWCHAR buffered_shortname (PCWCHAR name)
  {
    form_shortname (nt_max_path_buffer, NT_MAX_PATH, name);
    return nt_max_path_buffer;
  }

  dll *main_executable;
  dll start;
  int loaded_dlls;
  int reload_on_fork;
  dll *operator [] (PCWCHAR ntname);
  dll *alloc (HINSTANCE, per_process *, dll_type);
  dll *find (void *);
  void detach (void *);
  void init ();
  void load_after_fork (HANDLE);
  void reserve_space ();
  void load_after_fork_impl (HANDLE, dll* which, int retries);
  dll *find_by_modname (PCWCHAR modname);
  void populate_deps (dll* d);
  void topsort ();
  void topsort_visit (dll* d, bool goto_tail);
  void append (dll* d);

  void release_forkables ();
  void cleanup_forkables ();
  bool setup_forkables (bool with_forkables)
  {
    if (forkables_needs == forkables_impossible)
      return true; /* short cut to not retry fork */
    /* Once used, always use forkables in current process chain. */
    if (forkables_needs != forkables_unknown)
      with_forkables = true;
    if (with_forkables)
      request_forkables ();
    return with_forkables;
  }

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
