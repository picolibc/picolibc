/*
 * Copyright (c) 2000, Red Hat, Inc.
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     A copy of the GNU General Public License can be found at
 *     http://www.gnu.org/
 *
 * Written by DJ Delorie <dj@redhat.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

#ifdef __GNUC__
const char *help_text = "
Usage: ssp [options] low_pc high_pc command...

The SSP is a `single-step profiler' - it uses the debug API to
single-step your program, noting *everything* your program runs, not
just random places hit by random timer interrupts.  You must specify
the range of EIP values to profile.  For example, you could profile
just a function, or just a line of code, or the whole thing.
Use \"objdump -h\" to find the start of .text and the section following
it; this is what you want.

There are many options to ssp.  Since step-profiling makes your
program run about 1,000 times slower than normal, it's best to
understand all the options so that you can narrow down the parts
of your program you need to single-step.

-v = verbose messages about debug events.

-d, -e = disable/enable single-stepping by default.  Use
OutputDebugString (\"ssp on\") to enable stepping, or \"ssp off\" to
disable it.  Thus, you can profile a single function call or block.

-t = trace every EIP value to a file TRACE.SSP.  This gets big *fast*.
Use \"addr2line -C -f -s -e foo.exe < trace.ssp > lines.ssp\" and then
\"perl cvttrace\" to convert to symbolic traces.

-tc = trace every EIP value to the console.  *Lots* slower.

-s = trace sub-threads too.  Dangerous if you have race conditions.

-dll = enable dll profiling.  A chart of relative DLL usage is
produced after the run.

Examples:
  ssp 0x401000 0x403000 hello.exe
  ssp -v -d -dll 0x401000 0x440000 foo.exe

The output is a file \"gmon.out\" that can be read with gprof:
  gprof -b foo.exe

See ssp.txt in the cygwin sources for more information.
";
#else
char *help_text = "Usage: get cygwin!\n";
#endif

#define KERNEL_ADDR 0x77000000

#define TRACE_SSP 0

#define VERBOSE	1
#define TIMES	1000

/* from winsup/gmon.h */
struct gmonhdr {
	unsigned long	lpc;	/* base pc address of sample buffer */
	unsigned long	hpc;	/* max pc address of sampled buffer */
	int	ncnt;		/* size of sample buffer (plus this header) */
	int	version;	/* version number */
	int	profrate;	/* profiling clock rate */
	int	spare[3];	/* reserved */
};
#define GMONVERSION	0x00051879
#define HISTCOUNTER unsigned short

typedef struct {
  unsigned int base_address;
  int pcount;
  int scount;
  char *name;
} DllInfo;

typedef struct {
  unsigned int address;
  unsigned char real_byte;
} PendingBreakpoints;

int low_pc=0, high_pc=0;
unsigned int last_pc=0, pc, last_sp=0, sp;
int total_cycles, count;
char *cmd_line;
HANDLE hProcess;
PROCESS_INFORMATION procinfo;
STARTUPINFO startup;
CONTEXT context;
HISTCOUNTER *hits=0;
struct gmonhdr hdr;
int running = 1, profiling = 1;
char dll_name[1024], *dll_ptr, *cp;
int eip;
unsigned opcode_count = 0;

int stepping_enabled = 1;
int tracing_enabled = 0;
int trace_console = 0;
int trace_all_threads = 0;
int dll_counts = 0;
int verbose = 0;

#define MAXTHREADS 100
int active_thread_ids[MAXTHREADS];
HANDLE active_threads[MAXTHREADS];
int thread_step_flags[MAXTHREADS];
int thread_return_address[MAXTHREADS];
int num_active_threads = 0;
int suspended_count=0;

#define MAXDLLS 100
DllInfo dll_info[MAXDLLS];
int num_dlls=0;

#define MAXPENDS 100
PendingBreakpoints pending_breakpoints[MAXPENDS];
int num_breakpoints=0;

static void
add_breakpoint (unsigned int address)
{
  int i;
  DWORD rv;
  static char int3[] = { 0xcc };
  for (i=0; i<num_breakpoints; i++)
    {
      if (pending_breakpoints[i].address == address)
	return;
      if (pending_breakpoints[i].address == 0)
	break;
    }
  if (i == MAXPENDS)
    return;
  pending_breakpoints[i].address = address;
  ReadProcessMemory (hProcess,
		    (void *)address,
		    &(pending_breakpoints[i].real_byte),
		    1, &rv);

  WriteProcessMemory (hProcess,
		     (void *)address,
		     (LPVOID)int3, 1, &rv);
  if (i >= num_breakpoints)
    num_breakpoints = i+1;
}

static int
remove_breakpoint (unsigned int address)
{
  int i;
  DWORD rv;
  for (i=0; i<num_breakpoints; i++)
    {
      if (pending_breakpoints[i].address == address)
	{
	  pending_breakpoints[i].address = 0;
	  WriteProcessMemory (hProcess,
			     (void *)address,
			     &(pending_breakpoints[i].real_byte),
			     1, &rv);
	  return 1;
	}
    }
  return 0;
}

static HANDLE
lookup_thread_id (int threadId, int *tix)
{
  int i;
  for (i=0; i<num_active_threads; i++)
    if (active_thread_ids[i] == threadId)
      {
	if (tix) *tix = i;
	return active_threads[i];
      }
  return 0;
}

static void
set_step_threads (int threadId, int trace)
{
  int rv, tix;
  HANDLE thread = lookup_thread_id (threadId, &tix);

  rv = GetThreadContext (thread, &context);
  if (rv != -1)
    {
      thread_step_flags[tix] = trace;
      if (trace)
	context.EFlags |= 0x100; /* TRAP (single step) flag */
      else
	context.EFlags &= ~0x100; /* TRAP (single step) flag */
      SetThreadContext (thread, &context);
    }
}

static void
set_steps ()
{
  int i, s;
  for (i=0; i<num_active_threads; i++)
    {
      GetThreadContext (active_threads[i], &context);
      s = context.EFlags & 0x0100;
      if (!s && thread_step_flags[i])
	{
	  set_step_threads (active_thread_ids[i], 1);
	}
    }
}

static int
dll_sort (const void *va, const void *vb)
{
  DllInfo *a = (DllInfo *)va;
  DllInfo *b = (DllInfo *)vb;
  if (a->base_address < b->base_address)
    return -1;
  return 1;
}

static char *
addr2dllname (unsigned int addr)
{
  int i;
  for (i=num_dlls-1; i>=0; i--)
    {
      if (dll_info[i].base_address < addr)
	{
	  return dll_info[i].name;
	}
    }
  return (char *)"";
}

static void
dump_registers (HANDLE thread)
{
  context.ContextFlags = CONTEXT_FULL;
  GetThreadContext (thread, &context);
  printf ("eax %08lx ebx %08lx ecx %08lx edx %08lx eip\n",
	  context.Eax, context.Ebx, context.Ecx, context.Edx);
  printf ("esi %08lx edi %08lx ebp %08lx esp %08lx %08lx\n",
	  context.Esi, context.Esi, context.Ebp, context.Esp, context.Eip);
}

typedef struct Edge {
  struct Edge *next;
  unsigned int from_pc;
  unsigned int to_pc;
  unsigned int count;
} Edge;

Edge *edges[4096];

void
store_call_edge (unsigned int from_pc, unsigned int to_pc)
{
  Edge *e;
  unsigned int h = ((from_pc + to_pc)>>4) & 4095;
  for (e=edges[h]; e; e=e->next)
    if (e->from_pc == from_pc && e->to_pc == to_pc)
      break;
  if (!e)
    {
      e = (Edge *)malloc (sizeof (Edge));
      e->next = edges[h];
      edges[h] = e;
      e->from_pc = from_pc;
      e->to_pc = to_pc;
      e->count = 0;
    }
  e->count++;
}

void
write_call_edges (FILE *f)
{
  int h;
  Edge *e;
  for (h=0; h<4096; h++)
    for (e=edges[h]; e; e=e->next)
      fwrite (&(e->from_pc), 1, 3*sizeof (unsigned int), f);
}

char *
wide_strdup (char *cp)
{
  unsigned short *s = (unsigned short *)cp;
  int len;
  char *rv;
  for (len=0; s[len]; len++);
  rv = (char *)malloc (len+1);
  for (len=0; s[len]; len++)
    rv[len] = s[len];
  rv[len] = 0;
  return rv;
}

void
run_program (char *cmdline)
{
  FILE *tracefile = 0;
  int tix, i;
  HANDLE hThread;
  char *string;

  memset (&startup, 0, sizeof (startup));
  startup.cb = sizeof (startup);

  if (!CreateProcess (0, cmd_line, 0, 0, 0,
		     CREATE_NEW_PROCESS_GROUP
		     | CREATE_SUSPENDED
		     | DEBUG_PROCESS
		     | DEBUG_ONLY_THIS_PROCESS,
		     0, 0, &startup, &procinfo))
    {
      fprintf (stderr, "Can't create process: error %ld\n", GetLastError ());
      exit (1);
    }

  hProcess = procinfo.hProcess;
#if 0
  printf ("procinfo: %08x %08x %08x %08x\n",
	 hProcess, procinfo.hThread, procinfo.dwProcessId, procinfo.dwThreadId);
#endif

  active_threads[0] = procinfo.hThread;
  active_thread_ids[0] = procinfo.dwThreadId;
  thread_step_flags[0] = stepping_enabled;
  num_active_threads = 1;

  dll_info[0].base_address = 0;
  dll_info[0].pcount = 0;
  dll_info[0].scount = 0;
  dll_info[0].name = cmd_line;
  num_dlls = 1;

  SetThreadPriority (procinfo.hThread, THREAD_PRIORITY_IDLE);

  context.ContextFlags = CONTEXT_FULL;

  ResumeThread (procinfo.hThread);

  total_cycles = 0;

  if (tracing_enabled)
    {
      tracefile = fopen ("trace.ssp", "w");
      if (!tracefile)
	{
	  tracing_enabled = 0;
	  perror ("trace.ssp");
	}
    }

  running = 1;
  while (running)
    {
      int src, dest;
      DWORD rv;
      DEBUG_EVENT event;
      int contv = DBG_CONTINUE;

      event.dwDebugEventCode = -1;
      if (!WaitForDebugEvent (&event, INFINITE))
	{
	  printf ("idle...\n");
	}

      hThread = lookup_thread_id (event.dwThreadId, &tix);

#if 0
      printf ("DE: %x/%d %d %d ",
	     hThread, tix,
	     event.dwDebugEventCode, num_active_threads);
      for (src=0; src<num_active_threads; src++)
	{
	  int sc = SuspendThread (active_threads[src]);
	  int rv = GetThreadContext (active_threads[src], &context);
	  ResumeThread (active_threads[src]);
	  printf (" [%x,%x,%x]",
		 active_threads[src], context.Eip, active_thread_ids[src]);
	}
      printf ("\n");
#endif

      switch (event.dwDebugEventCode)
	{

	case CREATE_PROCESS_DEBUG_EVENT:
	  break;

	case CREATE_THREAD_DEBUG_EVENT:
	  if (verbose)
	    printf ("create thread %08lx at %08x %s\n",
		   event.dwThreadId,
		   (int)event.u.CreateThread.lpStartAddress,
		   addr2dllname ((unsigned int)event.u.CreateThread.lpStartAddress));

	  active_thread_ids[num_active_threads] = event.dwThreadId;
	  active_threads[num_active_threads] = event.u.CreateThread.hThread;
	  thread_return_address[num_active_threads] = 0;
	  num_active_threads++;

	  if (trace_all_threads && stepping_enabled)
	    {
	      thread_step_flags[num_active_threads-1] = stepping_enabled;
	      add_breakpoint ((int)event.u.CreateThread.lpStartAddress);
	    }

	  break;

	case EXIT_THREAD_DEBUG_EVENT:
	  if (verbose)
	    printf ("exit thread %08lx, code=%ld\n",
		   event.dwThreadId,
		   event.u.ExitThread.dwExitCode);

	  for (src=0, dest=0; src<num_active_threads; src++)
	    if (active_thread_ids[src] != event.dwThreadId)
	      {
		active_thread_ids[dest] = active_thread_ids[src];
		active_threads[dest] = active_threads[src];
		dest++;
	      }
	  num_active_threads = dest;
	  break;

	case EXCEPTION_DEBUG_EVENT:
	  rv = GetThreadContext (hThread, &context);
	  switch (event.u.Exception.ExceptionRecord.ExceptionCode)
	    {
	    case STATUS_BREAKPOINT:
	      if (remove_breakpoint ((int)event.u.Exception.ExceptionRecord.ExceptionAddress))
		{
		  context.Eip --;
		  if (rv != -1)
		    SetThreadContext (hThread, &context);
		  if (ReadProcessMemory (hProcess, (void *)context.Esp, &rv, 4, &rv))
		      thread_return_address[tix] = rv;
		}
	      set_step_threads (event.dwThreadId, stepping_enabled);
	    case STATUS_SINGLE_STEP:
	      opcode_count++;
	      pc = (unsigned int)event.u.Exception.ExceptionRecord.ExceptionAddress;
	      sp = (unsigned int)context.Esp;
	      if (tracing_enabled)
		fprintf (tracefile, "%08x %08lx\n", pc, event.dwThreadId);
	      if (trace_console)
		{
		  printf ("%d %08x\n", tix, pc);
		  fflush (stdout);
		}

	      if (dll_counts)
		{
		  int i;
		  for (i=num_dlls-1; i>=0; i--)
		    {
		      if (dll_info[i].base_address < context.Eip)
			{
			  if (hThread == procinfo.hThread)
			    dll_info[i].pcount++;
			  else
			    dll_info[i].scount++;
			  break;
			}
		    }
		}

	      if (pc < last_pc || pc > last_pc+10)
		{
		  static int ncalls=0;
		  static int qq=0;
		  if (++qq % 100 == 0)
		    fprintf (stderr, " %08x %d %d \r",
			    pc, ncalls, opcode_count);
		  
		  if (sp == last_sp-4)
		    {
		      ncalls++;
		      store_call_edge (last_pc, pc);
		      if (last_pc < KERNEL_ADDR && pc > KERNEL_ADDR)
			{
			  int retaddr;
			  DWORD rv;
			  ReadProcessMemory (hProcess,
					    (void *)sp,
					    (LPVOID)&(retaddr),
					    4, &rv);
#if 0
			  printf ("call last_pc = %08x pc = %08x rv = %08x\n",
				 last_pc, pc, retaddr);
			  /* experimental - try to skip kernel calls for speed */
			  add_breakpoint (retaddr);
			  set_step_threads (event.dwThreadId, 0);
#endif
			}
		    }
		}

	      total_cycles++;
	      last_sp = sp;
	      last_pc = pc;
	      if (pc >= low_pc && pc < high_pc)
		hits[(pc - low_pc)/2] ++;
	      break;
	    default:
	      if (verbose)
		{
		  printf ("exception %ld, ", event.u.Exception.dwFirstChance);
		  printf ("code: %lx flags: %lx\n",
			 event.u.Exception.ExceptionRecord.ExceptionCode,
			 event.u.Exception.ExceptionRecord.ExceptionFlags);
		  if (event.u.Exception.dwFirstChance == 1)
		    dump_registers (hThread);
		}
	      contv = DBG_EXCEPTION_NOT_HANDLED;
	      running = 0;
	      break;
	    }

	  if (rv != -1)
	    {
	      if (pc == thread_return_address[tix])
		{
		  if (context.EFlags & 0x100)
		    {
		      context.EFlags &= ~0x100; /* TRAP (single step) flag */
		      SetThreadContext (hThread, &context);
		    }
		}
	      else if (stepping_enabled)
		{
		  if (!(context.EFlags & 0x100))
		    {
		      context.EFlags |= 0x100; /* TRAP (single step) flag */
		      SetThreadContext (hThread, &context);
		    }
		}
	    }
	  break;

	case OUTPUT_DEBUG_STRING_EVENT:
	  string = (char *)malloc (event.u.DebugString.nDebugStringLength+1);
	  i = ReadProcessMemory (hProcess,
			    event.u.DebugString.lpDebugStringData,
			    (LPVOID)string,
			    event.u.DebugString.nDebugStringLength,
			    &rv);
	  if (!i)
	    {
	      printf ("error reading memory: %ld %ld\n", rv, GetLastError ());
	    }
	  if (verbose)
	    printf ("ODS: %x/%d \"%s\"\n",
		   (int)hThread, tix, string);

	  if (strcmp (string, "ssp on") == 0)
	    {
	      stepping_enabled = 1;
	      set_step_threads (event.dwThreadId, 1);
	    }

	  if (strcmp (string, "ssp off") == 0)
	    {
	      stepping_enabled = 0;
	      set_step_threads (event.dwThreadId, 0);
	    }

	  break;


	case LOAD_DLL_DEBUG_EVENT:
	  if (verbose)
	    printf ("load dll %08x:",
		   (int)event.u.LoadDll.lpBaseOfDll);
	  
	  dll_ptr = (char *)"( u n k n o w n ) \0\0";
	  if (event.u.LoadDll.lpImageName)
	    {
	      ReadProcessMemory (hProcess,
				event.u.LoadDll.lpImageName,
				(LPVOID)&src,
				sizeof (src),
				&rv);
	      if (src)
		{
		  ReadProcessMemory (hProcess,
				    (void *)src,
				    (LPVOID)dll_name,
				    sizeof (dll_name),
				    &rv);
		  dll_name[rv] = 0;
		  dll_ptr = dll_name;
		  for (cp=dll_name; *cp; cp++)
		    {
		      if (*cp == '\\' || *cp == '/')
			{
			  dll_ptr = cp+1;
			}
		      *cp = tolower (*cp);
		    }
		}
	    }


	  dll_info[num_dlls].base_address
	    = (unsigned int)event.u.LoadDll.lpBaseOfDll;
	  dll_info[num_dlls].pcount = 0;
	  dll_info[num_dlls].scount = 0;
	  dll_info[num_dlls].name = wide_strdup (dll_ptr);
	  if (verbose)
	    printf (" %s\n", dll_info[num_dlls].name);
	  num_dlls++;
	  qsort (dll_info, num_dlls, sizeof (DllInfo), dll_sort);

	  break;

	case UNLOAD_DLL_DEBUG_EVENT:
	  if (verbose)
	    printf ("unload dll\n");
	  break;

	case EXIT_PROCESS_DEBUG_EVENT:
	  if (verbose)
	    printf ("process %08lx %08lx exit %ld\n",
		   event.dwProcessId, event.dwThreadId,
		   event.u.ExitProcess.dwExitCode);

	  running = 0;
	  break;
	}

      set_steps ();
      ContinueDebugEvent (event.dwProcessId, event.dwThreadId, contv);
    }

  count = 0;
  for (pc=low_pc; pc<high_pc; pc+=2)
    {
      count += hits[(pc - low_pc)/2];
    }
  printf ("total cycles: %d, counted cycles: %d\n", total_cycles, count);

  if (tracing_enabled)
    fclose (tracefile);

}

#define WHITE(s) while (isspace (*s)) s++

int
main (int argc, char **argv)
{
  int i, n;
  int total_pcount, total_scount;
  FILE *gmon;

  setbuf (stdout, 0);

  cmd_line = GetCommandLine ();

  /* strip off the program part */
  n = sscanf (cmd_line, "%*s%n", &i);
  cmd_line += i;
  WHITE (cmd_line);

  /* parse arguments.  Ugly, but we need to preserve the tail of the line */
  while (*cmd_line == '-')
    {
      char opt[100];
      n = sscanf (cmd_line, " %s%n", opt, &i);
      cmd_line += i;
      WHITE (cmd_line);

      if (strcmp (opt, "-d") == 0)
	{
	  printf ("stepping disabled; enable via OutputDebugString (\"ssp on\")\n");
	  stepping_enabled = 0;
	}
      if (strcmp (opt, "-e") == 0)
	{
	  printf ("stepping enabled; disable via OutputDebugString (\"ssp off\")\n");
	  stepping_enabled = 1;
	}
      if (strcmp (opt, "-t") == 0)
	{
	  printf ("tracing all $eip to trace.ssp\n");
	  tracing_enabled = 1;
	}
      if (strcmp (opt, "-tc") == 0)
	{
	  printf ("tracing *all* $eip to the console\n");
	  trace_console = 1;
	}
      if (strcmp (opt, "-s") == 0)
	{
	  printf ("tracing all sub-threads too, not just the main one\n");
	  trace_all_threads = 1;
	}
      if (strcmp (opt, "-dll") == 0)
	{
	  printf ("profiling dll usage\n");
	  dll_counts = 1;
	}
      if (strcmp (opt, "-v") == 0)
	{
	  printf ("verbose messages enabled\n");
	  verbose = 1;
	}
    }

  n = sscanf (cmd_line, " %i %i %n", &low_pc, &high_pc, &i);
  if (n < 2)
    {
      fputs (help_text, stderr);
      exit (1);
    }
  cmd_line += i;

  if (low_pc > high_pc-8)
    {
      fprintf (stderr, "Hey, low_pc must be lower than high_pc\n");
      exit (1);
    }

  hits = (HISTCOUNTER *)malloc (high_pc-low_pc+4);
  memset (hits, 0, high_pc-low_pc+4);

  fprintf (stderr, "prun: [%08x,%08x] Running `%s'\n",
	  low_pc, high_pc, cmd_line);

  run_program (cmd_line);

  hdr.lpc = low_pc;
  hdr.hpc = high_pc;
  hdr.ncnt = high_pc-low_pc + sizeof (hdr);
  hdr.version = GMONVERSION;
  hdr.profrate = 100;

  gmon = fopen ("gmon.out", "wb");
  fwrite (&hdr, 1, sizeof (hdr), gmon);
  fwrite (hits, 1, high_pc-low_pc, gmon);
  write_call_edges (gmon);
  fclose (gmon);

  if (dll_counts)
    {
      /*      1234567 123% 1234567 123% 12345678 xxxxxxxxxxx */
      printf (" Main-Thread Other-Thread BaseAddr DLL Name\n");

      total_pcount = 0;
      total_scount = 0;
      for (i=0; i<num_dlls; i++)
	{
	  total_pcount += dll_info[i].pcount;
	  total_scount += dll_info[i].scount;
	}

      if (total_pcount == 0) total_pcount++;
      if (total_scount == 0) total_scount++;

      for (i=0; i<num_dlls; i++)
	if (dll_info[i].pcount || dll_info[i].scount)
	  {
	    printf ("%7d %3d%% %7d %3d%% %08x %s\n",
		   dll_info[i].pcount,
		   (dll_info[i].pcount*100)/opcode_count,
		   dll_info[i].scount,
		   (dll_info[i].scount*100)/opcode_count,
		   dll_info[i].base_address,
		   dll_info[i].name);
	  }
    }

  exit (0);
}

