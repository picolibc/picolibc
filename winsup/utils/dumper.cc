/* dumper.cc

   Written by Egor Duda <deo@logos-m.ru>

   This file is part of Cygwin.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License (file COPYING.dumper) for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include <ansidecl.h>
#define PACKAGE
#include <bfd.h>
#include <elf/common.h>
#include <elf/external.h>
#include <sys/procfs.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include <getopt.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <windows.h>
#define PSAPI_VERSION 1
#include <psapi.h>

#include "dumper.h"

#define NOTE_NAME_SIZE 16

#ifdef bfd_get_section_size
/* for bfd < 2.34 */
#define get_section_name(abfd, sect) bfd_get_section_name (abfd, sect)
#define get_section_size(sect) bfd_get_section_size(sect)
#define set_section_size(abfd, sect, size) bfd_set_section_size(abfd, sect, size)
#define set_section_flags(abfd, sect, flags) bfd_set_section_flags(abfd, sect, flags)
#else
/* otherwise bfd >= 2.34 */
#define get_section_name(afbd, sect) bfd_section_name (sect)
#define get_section_size(sect) bfd_section_size(sect)
#define set_section_size(abfd, sect, size) bfd_set_section_size(sect, size)
#define set_section_flags(abfd, sect, flags) bfd_set_section_flags(sect, flags)
#endif

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
BOOL nokill = FALSE;

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

  list = last = NULL;

  status_section = NULL;

  memory_num = module_num = thread_num = 0;

  hProcess = OpenProcess (PROCESS_ALL_ACCESS,
			  FALSE,	/* no inheritance */
			  pid);
  if (!hProcess)
    {
      fprintf (stderr, "Failed to open process #%u, error %ld\n",
	       (unsigned int) pid, (long) GetLastError ());
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
  if (hProcess)
    CloseHandle (hProcess);
  core_bfd = NULL;
  hProcess = NULL;
}

int
dumper::sane ()
{
  if (hProcess == NULL || core_bfd == NULL)
    return 0;
  return 1;
}

void
print_section_name (bfd* abfd, asection* sect, PTR obj)
{
  deb_printf (" %s", get_section_name (abfd, sect));
}

void
dumper::print_core_section_list ()
{
  deb_printf ("current sections:");
  bfd_map_over_sections (core_bfd, &print_section_name, NULL);
  deb_printf ("\n");
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
    {
      deb_printf ("Failed to read thread context (tid=%x), error %ld\n", tid, GetLastError ());
      return 0;
    }

  deb_printf ("added thread %u\n", tid);
  return 1;
}

int
dumper::add_mem_region (LPBYTE base, SIZE_T size)
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

  deb_printf ("added memory region %p-%p\n", base, base + size);
  return 1;
}

int
dumper::add_module (LPVOID base_address)
{
  if (!sane ())
    return 0;

  char *module_name = psapi_get_module_name (hProcess, base_address);
  if (module_name == NULL)
    return 1;

  process_entity *new_entity = add_process_entity_to_list (pr_ent_module);
  if (new_entity == NULL)
    return 0;
  new_entity->type = pr_ent_module;
  module_num++;

  new_entity->u.module.base_address = base_address;
  new_entity->u.module.name = module_name;

  deb_printf ("added module %p %s\n", base_address, module_name);
  return 1;
}

#define PAGE_BUFFER_SIZE 4096

void protect_dump(DWORD protect, char *buf)
{
  const char *pt[10];
  pt[0] = (protect & PAGE_READONLY) ? "RO " : "";
  pt[1] = (protect & PAGE_READWRITE) ? "RW " : "";
  pt[2] = (protect & PAGE_WRITECOPY) ? "WC " : "";
  pt[3] = (protect & PAGE_EXECUTE) ? "EX " : "";
  pt[4] = (protect & PAGE_EXECUTE_READ) ? "EXRO " : "";
  pt[5] = (protect & PAGE_EXECUTE_READWRITE) ? "EXRW " : "";
  pt[6] = (protect & PAGE_EXECUTE_WRITECOPY) ? "EXWC " : "";
  pt[7] = (protect & PAGE_GUARD) ? "GRD " : "";
  pt[8] = (protect & PAGE_NOACCESS) ? "NA " : "";
  pt[9] = (protect & PAGE_NOCACHE) ? "NC " : "";

  buf[0] = '\0';
  for (int i = 0; i < 10; i++)
    strcat (buf, pt[i]);
}

#define PSWSEI_ATTRIB_SHARED (0x1 << 15)

static BOOL
getRegionAttributes(HANDLE hProcess, LPVOID address, DWORD &attribs)
{
  PSAPI_WORKING_SET_EX_INFORMATION pswsei = { address };

  if (QueryWorkingSetEx(hProcess, &pswsei, sizeof(pswsei)))
    {
      attribs = pswsei.VirtualAttributes.Flags;
      return TRUE;
    }

  deb_printf("QueryWorkingSetEx failed status %08x\n", GetLastError());
  return FALSE;
}

int
dumper::collect_memory_sections ()
{
  if (!sane ())
    return 0;

  LPBYTE current_page_address;
  LPBYTE last_base = (LPBYTE) -1;
  SIZE_T last_size = (SIZE_T) 0;
  SIZE_T done;

  char mem_buf[PAGE_BUFFER_SIZE];

  MEMORY_BASIC_INFORMATION mbi;

  if (hProcess == NULL)
    return 0;

  for (current_page_address = 0; current_page_address < (LPBYTE) -1;)
    {
      if (!VirtualQueryEx (hProcess, current_page_address, &mbi, sizeof (mbi)))
	break;

      int skip_region_p = 0;
      const char *disposition = "dumped";

      if (mbi.Type & MEM_IMAGE)
	{
	  DWORD attribs = 0;
	  if (getRegionAttributes(hProcess, current_page_address, attribs))
	    {
	      if (attribs & PSWSEI_ATTRIB_SHARED)
		{
		  skip_region_p = 1;
		  disposition = "skipped due to shared MEM_IMAGE";
		}
	    }
	  /*
	    The undocumented MemoryWorkingSetExInformation is allegedly
	    supported since XP, so should always succeed, but if it fails,
	    fallback to looking at region protection.
	   */
	  else if (!(mbi.Protect & (PAGE_EXECUTE_READWRITE | PAGE_READWRITE)))
	    {
	      skip_region_p = 1;
	      disposition = "skipped due to non-writeable MEM_IMAGE";
	    }
	}

      if (mbi.Protect & PAGE_NOACCESS)
	{
	  skip_region_p = 1;
	  disposition = "skipped due to noaccess";
	}

      if (mbi.Protect & PAGE_GUARD)
	{
	  skip_region_p = 1;
	  disposition = "skipped due to guardpage";
	}

      if (mbi.State != MEM_COMMIT)
	{
	  skip_region_p = 1;
	  disposition = "skipped due to uncommited";
	}

      {
	char buf[10 * 6];
	protect_dump(mbi.Protect, buf);

	const char *state = "";
	const char *type = "";

	if (mbi.State & MEM_COMMIT)
	  {
	    state = "COMMIT";
	  }
	else if (mbi.State & MEM_FREE)
	  {
	    state = "FREE";
	    type = "FREE";
	  }
	else if (mbi.State & MEM_RESERVE)
	  {
	    state = "RESERVE";
	  }

	if (mbi.Type & MEM_IMAGE)
	  {
	    type = "IMAGE";
	  }
	else if (mbi.Type & MEM_MAPPED)
	  {
	    type = "MAPPED";
	  }
	else if (mbi.Type & MEM_PRIVATE)
	  {
	    type = "PRIVATE";
	  }

	deb_printf ("region 0x%016lx-0x%016lx (protect = %-8s, state = %-7s, type = %-7s, %s)\n",
		    current_page_address,
		    current_page_address + mbi.RegionSize,
		    buf, state, type, disposition);
      }

      if (!skip_region_p)
	{
	  /* just to make sure that later we'll be able to read it.
	     According to MS docs either region is all-readable or
	     all-nonreadable */
	  if (!ReadProcessMemory (hProcess, current_page_address, mem_buf, sizeof (mem_buf), &done))
	    {
	      DWORD err = GetLastError ();

	      deb_printf ("warning: failed to read memory at %p-%p, error %ld.\n",
			  current_page_address,
			  current_page_address + mbi.RegionSize,
			  err);
	      skip_region_p = 1;
	    }
	}

      if (!skip_region_p)
	{
	  if (last_base + last_size == current_page_address)
	    last_size += mbi.RegionSize;
	  else
	    {
	      add_mem_region (last_base, last_size);
	      last_base = (LPBYTE) mbi.BaseAddress;
	      last_size = mbi.RegionSize;
	    }
	}
      else
	{
	  add_mem_region (last_base, last_size);
	  last_base = NULL;
	  last_size = 0;
	}

      current_page_address += mbi.RegionSize;
    }

  /* dump last sections, if any */
  add_mem_region (last_base, last_size);
  return 1;
};

int
dumper::dump_memory_region (asection * to, process_mem_region * memory)
{
  if (!sane ())
    return 0;

  SIZE_T size = memory->size;
  SIZE_T todo;
  SIZE_T done;
  LPBYTE pos = memory->base;
  DWORD sect_pos = 0;

  if (to == NULL || memory == NULL)
    return 0;

  char mem_buf[PAGE_BUFFER_SIZE];

  while (size > 0)
    {
      todo = MIN (size, PAGE_BUFFER_SIZE);
      if (!ReadProcessMemory (hProcess, pos, mem_buf, todo, &done))
	{
	  deb_printf ("Failed to read process memory at %x(%x), error %ld\n", pos, todo, GetLastError ());
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#pragma GCC diagnostic ignored "-Warray-bounds"
  strncpy (header.elf_note_header.name, "win32thread", NOTE_NAME_SIZE);
#pragma GCC diagnostic pop

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
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wstringop-overflow=1"
#pragma GCC diagnostic ignored "-Warray-bounds"
  strncpy (header.elf_note_header.name, "win32module", NOTE_NAME_SIZE);
#pragma GCC diagnostic pop

#ifdef __x86_64__
  module_pstatus_ptr->data_type = NOTE_INFO_MODULE64;
#else
  module_pstatus_ptr->data_type = NOTE_INFO_MODULE;
#endif
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
      fprintf (stderr, "Cannot attach to process #%u, error %ld",
	       (unsigned int) pid, (long) GetLastError ());
      return 0;
    }

  DEBUG_EVENT current_event;

  while (1)
    {
      if (!WaitForDebugEvent (&current_event, INFINITE))
	return 0;

      deb_printf ("got debug event %d\n", current_event.dwDebugEventCode);

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

	  /* We're done */
	  goto failed;

	  break;

	default:

	  break;

	}

      ContinueDebugEvent (current_event.dwProcessId,
			  current_event.dwThreadId,
			  DBG_CONTINUE);
    }

failed:
  if (nokill)
    {
      if (!DebugActiveProcessStop (pid))
	{
	  fprintf (stderr, "Cannot detach from process #%u, error %ld",
		   (unsigned int) pid, (long) GetLastError ());
	}
    }
  /* Otherwise, the debuggee is terminated when this process exits
     (as DebugSetProcessKillOnExit() defaults to TRUE) */

  return 0;
}

int
dumper::init_core_dump ()
{
  bfd_init ();

#ifdef __x86_64__
  const char *target = "elf64-x86-64";
#else
  const char *target = "elf32-i386";
#endif

  core_bfd = bfd_openw (file_name, target);
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

  if (!bfd_set_arch_mach (core_bfd, bfd_arch_i386, 0 /* = default */))
    {
      bfd_perror ("setting bfd architecture");
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
  SIZE_T sect_size;
  bfd_vma sect_vma;

  asection *new_section;

  for (process_entity * p = list; p != NULL; p = p->next)
    {
      sect_no++;

      unsigned long phdr_type = PT_LOAD;

      switch (p->type)
	{
	case pr_ent_memory:
	  sprintf (sect_name, ".mem/%u", sect_no);
	  sect_flags = SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD;
	  sect_size = p->u.memory.size;
	  sect_vma = (bfd_vma) (p->u.memory.base);
	  phdr_type = PT_LOAD;
	  break;

	case pr_ent_thread:
	  sprintf (sect_name, ".note/%u", sect_no);
	  sect_flags = SEC_HAS_CONTENTS | SEC_LOAD;
	  sect_size = sizeof (note_header) + sizeof (struct win32_pstatus);
	  sect_vma = 0;
	  phdr_type = PT_NOTE;
	  break;

	case pr_ent_module:
	  sprintf (sect_name, ".note/%u", sect_no);
	  sect_flags = SEC_HAS_CONTENTS | SEC_LOAD;
	  sect_size = sizeof (note_header) + sizeof (struct win32_pstatus) +
	    (bfd_size_type) (strlen (p->u.module.name));
	  sect_vma = 0;
	  phdr_type = PT_NOTE;
	  break;

	default:
	  continue;
	}

      if (p->type == pr_ent_module && status_section != NULL)
	{
	  if (!set_section_size (core_bfd,
				 status_section,
				 (get_section_size (status_section)
				  + sect_size)))
	    {
	      bfd_perror ("resizing status section");
	      goto failed;
	    };
	  continue;
	}

      deb_printf ("creating section (type%u) %s(%u), flags=%08x\n",
		  p->type, sect_name, sect_size, sect_flags);

      bfd_set_error (bfd_error_no_error);
      char *buf = strdup (sect_name);
      new_section = bfd_make_section (core_bfd, buf);
      if (new_section == NULL)
	{
	  if (bfd_get_error () == bfd_error_no_error)
	    fprintf (stderr, "error creating new section (%s), section already exists.\n", buf);
	  else
	    bfd_perror ("creating section");
	  goto failed;
	}

      if (!set_section_flags (core_bfd, new_section, sect_flags) ||
	  !set_section_size (core_bfd, new_section, sect_size))
	{
	  bfd_perror ("setting section attributes");
	  goto failed;
	};

      new_section->vma = sect_vma;
      new_section->lma = 0;
      new_section->output_section = new_section;
      new_section->output_offset = 0;
      p->section = new_section;
      int section_count = 1;

      bfd_boolean filehdr = 0;
      bfd_boolean phdrs = 0;

      bfd_vma at = 0;
      bfd_boolean valid_at = 0;

      flagword flags = 0;
      bfd_boolean valid_flags = 1;

      if (p->type == pr_ent_memory)
	{
	  MEMORY_BASIC_INFORMATION mbi;
	  if (!VirtualQueryEx (hProcess, (LPVOID)sect_vma, &mbi, sizeof (mbi)))
	    {
	      bfd_perror ("getting mem region flags");
	      goto failed;
	    }

	  static const struct
	  {
	    DWORD protect;
	    flagword flags;
	  } mappings[] =
	    {
	      { PAGE_READONLY, PF_R },
	      { PAGE_READWRITE, PF_R | PF_W },
	      { PAGE_WRITECOPY, PF_W },
	      { PAGE_EXECUTE, PF_X },
	      { PAGE_EXECUTE_READ, PF_X | PF_R },
	      { PAGE_EXECUTE_READWRITE, PF_X | PF_R | PF_W },
	      { PAGE_EXECUTE_WRITECOPY, PF_X | PF_W }
	    };

	  for (size_t i = 0;
	       i < sizeof (mappings) / sizeof (mappings[0]);
	       i++)
	    if ((mbi.Protect & mappings[i].protect) != 0)
	      flags |= mappings[i].flags;
	}

      if (!bfd_record_phdr (core_bfd, phdr_type,
			    valid_flags, flags,
			    valid_at, at,
			    filehdr, phdrs,
			    section_count, &new_section))
	{
	  bfd_perror ("recording program headers");
	  goto failed;
	}
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

      deb_printf ("writing section type=%u base=%p size=%p flags=%08x\n",
		  p->type,
		  p->section->vma,
		  get_section_size (p->section),
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

static void __attribute__ ((__noreturn__))
usage (FILE *stream, int status)
{
  fprintf (stream, "\
Usage: %s [OPTION] FILENAME WIN32PID\n\
\n\
Dump core from WIN32PID to FILENAME.core\n\
\n\
 -n, --nokill   don't terminate the dumped process\n\
 -d, --verbose  be verbose while dumping\n\
 -h, --help     output help information and exit\n\
 -q, --quiet    be quiet while dumping (default)\n\
 -V, --version  output version information and exit\n\
\n", program_invocation_short_name);
  exit (status);
}

struct option longopts[] = {
  {"nokill", no_argument, NULL, 'n'},
  {"verbose", no_argument, NULL, 'd'},
  {"help", no_argument, NULL, 'h'},
  {"quiet", no_argument, NULL, 'q'},
  {"version", no_argument, 0, 'V'},
  {0, no_argument, NULL, 0}
};
const char *opts = "ndhqV";

static void
print_version ()
{
  printf ("dumper (cygwin) %d.%d.%d\n"
	  "Core Dumper for Cygwin\n"
	  "Copyright (C) 1999 - %s Cygwin Authors\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

int
main (int argc, char **argv)
{
  int opt;
  const char *p = "";
  DWORD pid;

  while ((opt = getopt_long (argc, argv, opts, longopts, NULL) ) != EOF)
    switch (opt)
      {
      case 'n':
	nokill = TRUE;
	break;
      case 'd':
	verbose = TRUE;
	break;
      case 'q':
	verbose = FALSE;
	break;
      case 'h':
	usage (stdout, 0);
      case 'V':
	print_version ();
	exit (0);
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n",
		 program_invocation_short_name);
	exit (1);
      }

  if (argv && *(argv + optind) && *(argv + optind +1))
    {
      ssize_t len = cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_RELATIVE,
				      *(argv + optind), NULL, 0);
      char *win32_name = (char *) alloca (len);
      cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_RELATIVE,  *(argv + optind),
			win32_name, len);
      if ((p = strrchr (win32_name, '\\')))
	p++;
      else
	p = win32_name;
      pid = strtoul (*(argv + optind + 1), NULL, 10);
    }
  else
    {
      usage (stderr, 1);
      return -1;
    }

  char *core_file = (char *) malloc (strlen (p) + sizeof (".core"));
  if (!core_file)
    {
      fprintf (stderr, "error allocating memory\n");
      return -1;
    }
  sprintf (core_file, "%s.core", p);

  DWORD tid = 0;

  if (verbose)
    printf ("dumping process #%u to %s\n", (unsigned int) pid, core_file);

  dumper d (pid, tid, core_file);
  if (!d.sane ())
    return -1;
  d.collect_process_information ();
  free (core_file);

  return 0;
};
