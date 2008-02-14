/* dll_init.h

   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

struct per_module
{
  char ***envptr;
  void (**ctors)(void);
  void (**dtors)(void);
  void *data_start;
  void *data_end;
  void *bss_start;
  void *bss_end;
  int (*main)(int, char **, char **);
  per_module &operator = (per_process *p)
  {
    envptr = p->envptr;
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
  dll_type type;
  int namelen;
  char name[NT_MAX_PATH];
  void detach ();
  int init ();
};

#define MAX_DLL_BEFORE_INIT     100

class dll_list
{
  dll *end;
  dll *hold;
  dll_type hold_type;
public:
  dll start;
  int tot;
  int loaded_dlls;
  int reload_on_fork;
  dll *operator [] (const char *name);
  dll *alloc (HINSTANCE, per_process *, dll_type);
  void detach (void *);
  void init ();
  void load_after_fork (HANDLE, dll *);
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
};

extern dll_list dlls;
void dll_global_dtors ();
