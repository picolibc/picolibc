/* dumper.cc

   Copyright 1999 Cygnus Solutions.

   Written by Egor Duda <deo@logos-m.ru>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <bfd.h>
#include <elf/common.h>
#include <elf/external.h>
#include <sys/procfs.h>
#include <sys/cygwin.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>

#include "dumper.h"

#define NOTE_NAME_SIZE 16

typedef struct _note_header
  {
    Elf_External_Note elf_note_header;
    char name[NOTE_NAME_SIZE - 1];	/* external note contains first byte of data */
  }
#ifdef __GNUC__
__attribute__ ((packed))
#endif
  note_header;

     BOOL verbose = FALSE;

     int deb_printf (const char *format,...)
{
  if (!verbose)
    return 0;
  va_list va;
  va_start (va, format);
  int ret_val = vprintf (format, va);
  va_end (va);
  return ret_val;
}

dumper::dumper (DWORD pid, DWORD tid, const char *file_name)
{
  this->file_name = strdup (file_name);

  this->pid = pid;
  this->tid = tid;
  core_bfd = NULL;
  excl_list = new exclusion (20);

  list = last = NULL;

  status_section = NULL;

  memory_num = module_num = thread_num = 0;

  hProcess = OpenProcess (PROCESS_ALL_ACCESS,
			  FALSE,	/* no inheritance */
			  pid);
  if (!hProcess)
    {
      fprintf (stderr, "Failed to open process #%lu\n", pid);
      return;
    }

  init_core_dump ();

  if (!sane ())
    dumper_abort ();
}

dumper::~dumper ()
{
  close ();
  free (file_name);
}

void
dumper::dumper_abort ()
{
  close ();
  unlink (file_name);
}

void
dumper::close ()
{
  if (core_bfd)
    bfd_close (core_bfd);
  if (excl_list)
    delete excl_list;
  if (hProcess)
    CloseHandle (hProcess);
  core_bfd = NULL;
  hProcess = NULL;
  excl_list = NULL;
}

int
dumper::sane ()
{
  if (hProcess == NULL || core_bfd == NULL || excl_list == NULL)
    return 0;
  return 1;
}

process_entity *
dumper::add_process_entity_to_list (process_entity_type type)
{
  if (!sane ())
    return NULL;

  process_entity *new_entity = (process_entity *) malloc (sizeof (process_entity));
  if (new_entity == NULL)
    return NULL;
  new_entity->next = NULL;
  new_entity->section = NULL;
  if (last == NULL)
    list = new_entity;
  else
    last->next = new_entity;
  last = new_entity;
  return new_entity;
}

int
dumper::add_thread (DWORD tid, HANDLE hThread)
{
  if (!sane ())
    return 0;

  CONTEXT *pcontext;

  process_entity *new_entity = add_process_entity_to_list (pr_ent_thread);
  if (new_entity == NULL)
    return 0;
  new_entity->type = pr_ent_thread;
  thread_num++;

  new_entity->u.thread.tid = tid;
  new_entity->u.thread.hThread = hThread;

  pcontext = &(new_entity->u.thread.context);
  pcontext->ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
  if (!GetThreadContext (hThread, pcontext))
    return 0;

  deb_printf ("added thread %u\n", tid);
  return 1;
}

int
dumper::add_mem_region (LPBYTE base, DWORD size)
{
  if (!sane ())
    return 0;

  if (base == NULL || size == 0)
    return 1;			// just ignore empty regions

  process_entity *new_entity = add_process_entity_to_list (pr_ent_memory);
  if (new_entity == NULL)
    return 0;
  new_entity->type = pr_ent_memory;
  memory_num++;

  new_entity->u.memory.base = base;
  new_entity->u.memory.size = size;

  deb_printf ("added memory region %08x-%08x\n", (DWORD) base, (DWORD) base + size);
  return 1;
}

/* split_add_mem_region scans list of regions to be excluded from dumping process
   (excl_list) and removes all "excluded" parts from given region. */
int
dumper::split_add_mem_region (LPBYTE base, DWORD size)
{
  if (!sane ())
    return 0;

  if (base == NULL || size == 0)
    return 1;			// just ignore empty regions

  LPBYTE last_base = base;

  for (process_mem_region * p = excl_list->region;
       p < excl_list->region + excl_list->last;
       p++)
    {
      if (p->base >= base + size || p->base + p->size <= base)
	continue;

      if (p->base <= base)
	{
	  last_base = p->base + p->size;
	  continue;
	}

      add_mem_region (last_base, p->base - last_base);
      last_base = p->base + p->size;
    }

  if (last_base < base + size)
    add_mem_region (last_base, base + size - last_base);

  return 1;
}

int
dumper::add_module (LPVOID base_address)
{
  if (!sane ())
    return 0;

  char *module_name = psapi_get_module_name (hProcess, (DWORD) base_address);
  if (module_name == NULL)
    return 1;

  process_entity *new_entity = add_process_entity_to_list (pr_ent_module);
  if (new_entity == NULL)
    return 0;
  new_entity->type = pr_ent_module;
  module_num++;

  new_entity->u.module.base_address = base_address;
  new_entity->u.module.name = module_name;

  parse_pe (module_name, excl_list);

  deb_printf ("added module %08x %s\n", base_address, module_name);
  return 1;
}

#define PAGE_BUFFER_SIZE 4096

int
dumper::collect_memory_sections ()
{
  if (!sane ())
    return 0;

  LPBYTE current_page_address;
  LPBYTE last_base = (LPBYTE) 0xFFFFFFFF;
  DWORD last_size = 0;
  DWORD done;

  char mem_buf[PAGE_BUFFER_SIZE];

  MEMORY_BASIC_INFORMATION mbi;

  if (hProcess == NULL)
    return 0;

  for (current_page_address = 0; current_page_address < (LPBYTE) 0xFFFF0000;)
    {
      if (!VirtualQueryEx (hProcess, current_page_address, &mbi, sizeof (mbi)))
	break;

      int skip_region_p = 0;

      if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD) ||
	  mbi.State != MEM_COMMIT)
	skip_region_p = 1;

      if (!skip_region_p)
	{
	  /* just to make sure that later we'll be able to read it.
	     According to MS docs either region is all-readable or
	     all-nonreadable */
	  if (!ReadProcessMemory (hProcess, current_page_address, mem_buf, sizeof (mem_buf), &done))
	    {
	      const char *pt[10];
	      pt[0] = (mbi.Protect & PAGE_READONLY) ? "RO " : "";
	      pt[1] = (mbi.Protect & PAGE_READWRITE) ? "RW " : "";
	      pt[2] = (mbi.Protect & PAGE_WRITECOPY) ? "WC " : "";
	      pt[3] = (mbi.Protect & PAGE_EXECUTE) ? "EX " : "";
	      pt[4] = (mbi.Protect & PAGE_EXECUTE_READ) ? "EXRO " : "";
	      pt[5] = (mbi.Protect & PAGE_EXECUTE_READWRITE) ? "EXRW " : "";
	      pt[6] = (mbi.Protect & PAGE_EXECUTE_WRITECOPY) ? "EXWC " : "";
	      pt[7] = (mbi.Protect & PAGE_GUARD) ? "GRD " : "";
	      pt[8] = (mbi.Protect & PAGE_NOACCESS) ? "NA " : "";
	      pt[9] = (mbi.Protect & PAGE_NOCACHE) ? "NC " : "";
	      char buf[10 * 6];
	      buf[0] = '\0';
	      for (int i = 0; i < 10; i++)
		strcat (buf, pt[i]);

	      deb_printf ("warning: failed to read memory at %08x-%08x. protect = %s\n",
			  (DWORD) current_page_address,
			  (DWORD) current_page_address + mbi.RegionSize,
			  buf);
	      skip_region_p = 1;
	    }
	}

      if (!skip_region_p)
	{
	  if (last_base + last_size == current_page_address)
	    last_size += mbi.RegionSize;
	  else
	    {
	      split_add_mem_region (last_base, last_size);
	      last_base = (LPBYTE) mbi.BaseAddress;
	      last_size = mbi.RegionSize;
	    }
	}
      else
	{
	  split_add_mem_region (last_base, last_size);
	  last_base = NULL;
	  last_size = 0;
	}

      current_page_address += mbi.RegionSize;
    }

  /* dump last sections, if any */
  split_add_mem_region (last_base, last_size);
  return 1;
};

int
dumper::dump_memory_region (asection * to, process_mem_region * memory)
{
  if (!sane ())
    return 0;

  DWORD size = memory->size;
  DWORD todo;
  DWORD done;
  LPBYTE pos = memory->base;
  DWORD sect_pos = 0;

  if (to == NULL || memory == NULL)
    return 0;

  char mem_buf[PAGE_BUFFER_SIZE];

  while (size > 0)
    {
      todo = min (size, PAGE_BUFFER_SIZE);
      if (!ReadProcessMemory (hProcess, pos, mem_buf, todo, &done))
	{
	  deb_printf ("Error reading process memory at %x(%x) %u\n", pos, todo, GetLastError ());
	  return 0;
	}
      size -= done;
      pos += done;
      if (!bfd_set_section_contents (core_bfd, to, mem_buf, sect_pos, done))
	{
	  bfd_perror ("writing memory region to bfd");
	  dumper_abort ();
	  return 0;
	};
      sect_pos += done;
    }
  return 1;
}

int
dumper::dump_thread (asection * to, process_thread * thread)
{
  if (!sane ())
    return 0;

  if (to == NULL || thread == NULL)
    return 0;

  win32_pstatus thread_pstatus;

  note_header header;
  bfd_putl32 (NOTE_NAME_SIZE, header.elf_note_header.namesz);
  bfd_putl32 (sizeof (thread_pstatus), header.elf_note_header.descsz);
  bfd_putl32 (NT_WIN32PSTATUS, header.elf_note_header.type);
  strncpy ((char *) &header.elf_note_header.name, "win32thread", NOTE_NAME_SIZE);

  thread_pstatus.data_type = NOTE_INFO_THREAD;
  thread_pstatus.data.thread_info.tid = thread->tid;

  if (tid == 0)
    {
      /* this is a special case. we don't know, which thread
         was active when exception occured, so let's blame
         the first one */
      thread_pstatus.data.thread_info.is_active_thread = TRUE;
      tid = (DWORD) - 1;
    }
  else if (tid > 0 && thread->tid == tid)
    thread_pstatus.data.thread_info.is_active_thread = TRUE;
  else
    thread_pstatus.data.thread_info.is_active_thread = FALSE;

  memcpy (&(thread_pstatus.data.thread_info.thread_context),
	  &(thread->context),
	  sizeof (thread->context));

  if (!bfd_set_section_contents (core_bfd, to, &header,
				 0,
				 sizeof (header)) ||
      !bfd_set_section_contents (core_bfd, to, &thread_pstatus,
				 sizeof (header),
				 sizeof (thread_pstatus)))
    {
      bfd_perror ("writing thread info to bfd");
      dumper_abort ();
      return 0;
    };
  return 1;
}

int
dumper::dump_module (asection * to, process_module * module)
{
  if (!sane ())
    return 0;

  if (to == NULL || module == NULL)
    return 0;

  struct win32_pstatus *module_pstatus_ptr;

  int note_length = sizeof (struct win32_pstatus) + strlen (module->name);

  char *buf = (char *) malloc (note_length);

  if (!buf)
    {
      fprintf (stderr, "Error alloating memory. Dumping aborted.\n");
      goto out;
    };

  module_pstatus_ptr = (struct win32_pstatus *) buf;

  note_header header;
  bfd_putl32 (NOTE_NAME_SIZE, header.elf_note_header.namesz);
  bfd_putl32 (note_length, header.elf_note_header.descsz);
  bfd_putl32 (NT_WIN32PSTATUS, header.elf_note_header.type);
  strncpy ((char *) &header.elf_note_header.name, "win32module", NOTE_NAME_SIZE);

  module_pstatus_ptr->data_type = NOTE_INFO_MODULE;
  module_pstatus_ptr->data.module_info.base_address = module->base_address;
  module_pstatus_ptr->data.module_info.module_name_size = strlen (module->name) + 1;
  strcpy (module_pstatus_ptr->data.module_info.module_name, module->name);

  if (!bfd_set_section_contents (core_bfd, to, &header,
				 0,
				 sizeof (header)) ||
      !bfd_set_section_contents (core_bfd, to, module_pstatus_ptr,
				 sizeof (header),
				 note_length))
    {
      bfd_perror ("writing module info to bfd");
      goto out;
    };
  return 1;

out:
  if (buf)
    free (buf);
  dumper_abort ();
  return 0;

}

int
dumper::collect_process_information ()
{
  if (!sane ())
    return 0;

  if (!DebugActiveProcess (pid))
    {
      fprintf (stderr, "Cannot attach to process #%lu", pid);
      return 0;
    }

  char event_name[sizeof ("cygwin_error_start_event") + 20];
  sprintf (event_name, "cygwin_error_start_event%16lx", pid);
  HANDLE sync_with_debugee = OpenEvent (EVENT_MODIFY_STATE, FALSE, event_name);

  DEBUG_EVENT current_event;

  while (1)
    {
      if (!WaitForDebugEvent (&current_event, 20000))
	return 0;

      switch (current_event.dwDebugEventCode)
	{
	case CREATE_THREAD_DEBUG_EVENT:

	  if (!add_thread (current_event.dwThreadId,
			   current_event.u.CreateThread.hThread))
	    goto failed;

	  break;

	case CREATE_PROCESS_DEBUG_EVENT:

	  if (!add_module (current_event.u.CreateProcessInfo.lpBaseOfImage) ||
	      !add_thread (current_event.dwThreadId,
			   current_event.u.CreateProcessInfo.hThread))
	    goto failed;

	  break;

	case EXIT_PROCESS_DEBUG_EVENT:

	  deb_printf ("debugee quits");
	  ContinueDebugEvent (current_event.dwProcessId,
			      current_event.dwThreadId,
			      DBG_CONTINUE);

	  return 1;

	  break;

	case LOAD_DLL_DEBUG_EVENT:

	  if (!add_module (current_event.u.LoadDll.lpBaseOfDll))
	    goto failed;

	  break;

	case EXCEPTION_DEBUG_EVENT:

	  collect_memory_sections ();

	  /* got all info. time to dump */

	  if (!prepare_core_dump ())
	    {
	      fprintf (stderr, "Failed to prepare core dump\n");
	      goto failed;
	    };

	  if (!write_core_dump ())
	    {
	      fprintf (stderr, "Failed to write core dump\n");
	      goto failed;
	    };

	  /* signal a debugee that we've finished */
	  if (sync_with_debugee)
	    SetEvent (sync_with_debugee);

	  break;

	default:

	  break;

	}

      ContinueDebugEvent (current_event.dwProcessId,
			  current_event.dwThreadId,
			  DBG_CONTINUE);
    }
failed:
  /* set debugee free */
  if (sync_with_debugee)
    SetEvent (sync_with_debugee);

  return 0;
}

int
dumper::init_core_dump ()
{
  bfd_init ();

  core_bfd = bfd_openw (file_name, "elf32-i386");
  if (core_bfd == NULL)
    {
      bfd_perror ("opening bfd");
      goto failed;
    }

  if (!bfd_set_format (core_bfd, bfd_core))
    {
      bfd_perror ("setting bfd format");
      goto failed;
    }

  return 1;

failed:
  dumper_abort ();
  return 0;

}

int
dumper::prepare_core_dump ()
{
  if (!sane ())
    return 0;

  int sect_no = 0;
  char sect_name[50];

  flagword sect_flags;
  DWORD sect_size;
  bfd_vma sect_vma;

  asection *new_section;

  for (process_entity * p = list; p != NULL; p = p->next)
    {
      sect_no++;

      switch (p->type)
	{
	case pr_ent_memory:
	  sprintf (sect_name, ".mem/%u", sect_no);
	  sect_flags = SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD;
	  sect_size = p->u.memory.size;
	  sect_vma = (bfd_vma) (p->u.memory.base);

	  break;

	case pr_ent_thread:
	  sprintf (sect_name, ".note/%u", sect_no);
	  sect_flags = SEC_HAS_CONTENTS | SEC_LOAD;
	  sect_size = sizeof (note_header) + sizeof (struct win32_pstatus);
	  sect_vma = 0;
	  break;

	case pr_ent_module:
	  sprintf (sect_name, ".note/%u", sect_no);
	  sect_flags = SEC_HAS_CONTENTS | SEC_LOAD;
	  sect_size = sizeof (note_header) + sizeof (struct win32_pstatus) +
	    (bfd_size_type) (strlen (p->u.module.name));
	  sect_vma = 0;
	  break;

	default:
	  continue;
	}

      if (p->type == pr_ent_module && status_section != NULL)
	{
	  if (!bfd_set_section_size (core_bfd,
				     status_section,
				     status_section->_raw_size + sect_size))
	    {
	      bfd_perror ("resizing status section");
	      goto failed;
	    };
	  continue;
	}

      deb_printf ("creating section (type%u) %s(%u), flags=%08x\n",
		  p->type, sect_name, sect_size, sect_flags);

      char *buf = strdup (sect_name);
      new_section = bfd_make_section (core_bfd, buf);

      if (new_section == NULL ||
	  !bfd_set_section_flags (core_bfd, new_section, sect_flags) ||
	  !bfd_set_section_size (core_bfd, new_section, sect_size))
	{
	  bfd_perror ("creating section");
	  goto failed;
	};

      new_section->vma = sect_vma;
      new_section->output_section = new_section;
      new_section->output_offset = 0;
      p->section = new_section;
    }

  return 1;

failed:
  dumper_abort ();
  return 0;
}

int
dumper::write_core_dump ()
{
  if (!sane ())
    return 0;

  for (process_entity * p = list; p != NULL; p = p->next)
    {
      if (p->section == NULL)
	continue;

      deb_printf ("writing section type=%u base=%08x size=%08x flags=%08x\n",
		  p->type,
		  p->section->vma,
		  p->section->_raw_size,
		  p->section->flags);

      switch (p->type)
	{
	case pr_ent_memory:
	  dump_memory_region (p->section, &(p->u.memory));
	  break;

	case pr_ent_thread:
	  dump_thread (p->section, &(p->u.thread));
	  break;

	case pr_ent_module:
	  dump_module (p->section, &(p->u.module));
	  break;

	default:
	  continue;

	}
    }
  return 1;
}

static void
usage ()
{
  fprintf (stderr, "Usage: dumper [-v] [-c filename] pid\n");
  fprintf (stderr, "-c filename -- dump core to filename.core\n");
  fprintf (stderr, "-d          -- print some debugging info while dumping\n");
  fprintf (stderr, "pid         -- win32-pid of process to dump\n");
}

int
main (int argc, char **argv)
{
  int opt;
  const char *p = "";
  DWORD pid;

  while ((opt = getopt (argc, argv, "dc:")) != EOF)
    switch (opt)
      {
      case 'd':
	verbose = TRUE;
	break;
      case 'c':
	char win32_name[MAX_PATH];
	cygwin_conv_to_win32_path (optarg, win32_name);
	if ((p = strrchr (win32_name, '\\')))
	  p++;
	else
	  p = win32_name;
	break;
      }

  char *core_file = (char *) malloc (strlen (p) + sizeof (".core"));
  if (!core_file)
    {
      fprintf (stderr, "error allocating memory\n");
      return -1;
    }
  sprintf (core_file, "%s.core", p);

  if (argv && *(argv + optind))
    pid = atoi (*(argv + optind));
  else
    {
      usage ();
      return -1;
    }

  DWORD tid = 0;

  if (verbose)
    printf ("dumping process #%lu to %s\n", pid, core_file);

  dumper d (pid, tid, core_file);
  if (!d.sane ())
    return -1;
  d.collect_process_information ();
  free (core_file);

  return 0;
};
