/* dumper.h

   Copyright 1999 Cygnus Solutions.

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _DUMPER_H_
#define _DUMPER_H_

#include <windows.h>

typedef struct
{
  LPBYTE base;
  DWORD size;
} process_mem_region;

typedef struct
{
  DWORD tid;
  HANDLE hThread;
  CONTEXT context;
} process_thread;

typedef struct
{
  LPVOID base_address;
  char* name;
} process_module;

enum process_entity_type
{
  pr_ent_memory,
  pr_ent_thread,
  pr_ent_module
};

typedef struct _process_entity
{
  process_entity_type type;
  union
    {
      process_thread thread;
      process_mem_region memory;
      process_module module;
    } u;
  asection* section;
  struct _process_entity* next;
} process_entity;

class exclusion
{
public:
  int last;
  int size;
  int step;
  process_mem_region* region;

  exclusion ( int step ) { last = size = 0;
			   this->step = step;
			   region = NULL; }
  ~exclusion () { free ( region ); }
  int add ( LPBYTE mem_base, DWORD mem_size );
  int sort_and_check ();
};

#define PAGE_BUFFER_SIZE 4096

class dumper
{
  DWORD pid;
  DWORD tid; /* thread id of active thread */
  HANDLE hProcess;
  process_entity* list;
  process_entity* last;
  exclusion* excl_list;

  char* file_name;
  bfd* core_bfd;

  asection* status_section;

  int memory_num;
  int module_num;
  int thread_num;

  void close ();
  void dumper_abort ();

  process_entity* add_process_entity_to_list ( process_entity_type type );
  int add_thread ( DWORD tid, HANDLE hThread );
  int add_mem_region ( LPBYTE base, DWORD size );

  /* break mem_region by excl_list and add add all subregions */
  int split_add_mem_region ( LPBYTE base, DWORD size );

  int add_module ( LPVOID base_address );

  int collect_memory_sections ();
  int dump_memory_region ( asection* to, process_mem_region* memory );
  int dump_thread ( asection* to, process_thread* thread );
  int dump_module ( asection* to, process_module* module );

public:
  int sane ();

  int collect_process_information ();

  dumper ( DWORD pid, DWORD tid, const char* name );
  ~dumper ();

  int init_core_dump ();
  int prepare_core_dump ();
  int write_core_dump ();
};

extern int deb_printf ( const char* format, ... );

extern char* psapi_get_module_name ( HANDLE hProcess, DWORD BaseAddress );

extern int parse_pe ( const char* file_name, exclusion* excl_list );

extern BOOL verbose;

#endif
