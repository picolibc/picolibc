/*
 * Copyright (c) 2000, 2001, 2002, 2009, 2011, 2013 Red Hat, Inc.
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

#ifdef __x86_64__

#include <stdio.h>

int
main (int argc, char **argv)
{
  fprintf (stderr, "%s: This application is unsuported on x86_64 so far.\n",
	   argv[0]);
  return 1;
}

#else

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>
#include <getopt.h>
#include <cygwin/version.h>

static char *prog_name;

static struct option longopts[] =
{
  {"console-trace", no_argument, NULL, 'c' },
  {"disable", no_argument, NULL, 'd' },
  {"enable", no_argument, NULL, 'e' },
  {"help", no_argument, NULL, 'h' },
  {"dll", no_argument, NULL, 'l' },
  {"sub-threads", no_argument, NULL, 's' },
  {"trace-eip", no_argument, NULL, 't' },
  {"verbose", no_argument, NULL, 'v' },
  {"version", no_argument, NULL, 'V' },
  {NULL, 0, NULL, 0}
};

static char opts[] = "+cdehlstvV";

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

unsigned low_pc=0, high_pc=0;
unsigned last_pc=0, pc, last_sp=0, sp;
int total_cycles, count;
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
DWORD active_thread_ids[MAXTHREADS];
HANDLE active_threads[MAXTHREADS];
DWORD thread_step_flags[MAXTHREADS];
DWORD thread_return_address[MAXTHREADS];
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
lookup_thread_id (DWORD threadId, int *tix)
{
  int i;
  *tix = 0;
  for (i=0; i<num_active_threads; i++)
    if (active_thread_ids[i] == threadId)
      {
	if (tix)
	  *tix = i;
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

  if (!CreateProcess (0, cmdline, 0, 0, 0,
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
  dll_info[0].name = cmdline;
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
		  if (!rv)
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

	  if (!rv)
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
		      *cp = tolower ((unsigned char) *cp);
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

static void
usage (FILE * stream)
{
  fprintf (stream , ""
  "Usage: %s [options] low_pc high_pc command...\n"
  "\n"
  "Single-step profile COMMAND\n"
  "\n"
  " -c, --console-trace  trace every EIP value to the console. *Lots* slower.\n"
  " -d, --disable        disable single-stepping by default; use\n"
  "                      OutputDebugString (\"ssp on\") to enable stepping\n"
  " -e, --enable         enable single-stepping by default; use\n"
  "                      OutputDebugString (\"ssp off\") to disable stepping\n"
  " -h, --help           output usage information and exit\n"
  " -l, --dll            enable dll profiling.  A chart of relative DLL usage\n"
  "                      is produced after the run.\n"
  " -s, --sub-threads    trace sub-threads too.  Dangerous if you have\n"
  "                      race conditions.\n"
  " -t, --trace-eip      trace every EIP value to a file TRACE.SSP.  This\n"
  "                      gets big *fast*.\n"
  " -v, --verbose        output verbose messages about debug events.\n"
  " -V, --version        output version information and exit\n"
  "\n"
  "Example: %s 0x401000 0x403000 hello.exe\n"
  "\n"
  "", prog_name, prog_name);
  if (stream == stdout)
    fprintf (stream , ""
    "SSP - The Single Step Profiler\n"
    "\n"
    "Original Author:  DJ Delorie <dj@redhat.com>\n"
    "\n"
    "The SSP is a program that uses the Win32 debug API to run a program\n"
    "one ASM instruction at a time.  It records the location of each\n"
    "instruction used, how many times that instruction is used, and all\n"
    "function calls.  The results are saved in a format that is usable by\n"
    "the profiling program \"gprof\", although gprof will claim the values\n"
    "are seconds, they really are instruction counts.  More on that later.\n"
    "\n"
    "Because the SSP was originally designed to profile the cygwin DLL, it\n"
    "does not automatically select a block of code to report statistics on.\n"
    "You must specify the range of memory addresses to keep track of\n"
    "manually, but it's not hard to figure out what to specify.  Use the\n"
    "\"objdump\" program to determine the bounds of the target's \".text\"\n"
    "section.  Let's say we're profiling cygwin1.dll.  Make sure you've\n"
    "built it with debug symbols (else gprof won't run) and run objdump\n"
    "like this:\n"
    "\n"
    "	objdump -h cygwin1.dll\n"
    "\n"
    "It will print a report like this:\n"
    "\n"
    "cygwin1.dll:     file format pei-i386\n"
    "\n"
    "Sections:\n"
    "Idx Name          Size      VMA       LMA       File off  Algn\n"
    "  0 .text         0007ea00  61001000  61001000  00000400  2**2\n"
    "                  CONTENTS, ALLOC, LOAD, READONLY, CODE, DATA\n"
    "  1 .data         00008000  61080000  61080000  0007ee00  2**2\n"
    "                  CONTENTS, ALLOC, LOAD, DATA\n"
    "  . . .\n"
    "\n"
    "The only information we're concerned with are the VMA of the .text\n"
    "section and the VMA of the section after it (sections are usually\n"
    "contiguous; you can also add the Size to the VMA to get the end\n"
    "address).  In this case, the VMA is 0x61001000 and the ending address\n"
    "is either 0x61080000 (start of .data method) or 0x0x6107fa00 (VMA+Size\n"
    "method).\n"
    "\n"
    "There are two basic ways to use SSP - either profiling a whole\n"
    "program, or selectively profiling parts of the program.\n"
    "\n"
    "To profile a whole program, just run ssp without options.  By default,\n"
    "it will step the whole program.  Here's a simple example, using the\n"
    "numbers above:\n"
    "\n"
    "	ssp 0x61001000 0x61080000 hello.exe\n"
    "\n"
    "This will step the whole program.  It will take at least 8 minutes on\n"
    "a PII/300 (yes, really).  When it's done, it will create a file called\n"
    "\"gmon.out\".  You can turn this data file into a readable report with\n"
    "gprof:\n"
    "\n"
    "	gprof -b cygwin1.dll\n"
    "\n"
    "The \"-b\" means 'skip the help pages'.  You can omit this until you're\n"
    "familiar with the report layout.  The gprof documentation explains\n"
    "a lot about this report, but ssp changes a few things.  For example,\n"
    "the first part of the report reports the amount of time spent in each\n"
    "function, like this:\n"
    "\n"
    "Each sample counts as 0.01 seconds.\n"
    "  %%   cumulative   self              self     total\n"
    " time   seconds   seconds    calls  ms/call  ms/call  name\n"
    " 10.02    231.22    72.43       46  1574.57  1574.57  strcspn\n"
    "  7.95    288.70    57.48      130   442.15   442.15  strncasematch\n"
    "\n"
    "The \"seconds\" columns are really CPU opcodes, 1/100 second per opcode.\n"
    "So, \"231.22\" above means 23,122 opcodes.  The ms/call values are 10x\n"
    "too big; 1574.57 means 157.457 opcodes per call.  Similar adjustments\n"
    "need to be made for the \"self\" and \"children\" columns in the second\n"
    "part of the report.\n"
    "\n"
    "OK, so now we've got a huge report that took a long time to generate,\n"
    "and we've identified a spot we want to work on optimizing.  Let's say\n"
    "it's the time() function.  We can use SSP to selectively profile this\n"
    "function by using OutputDebugString() to control SSP from within the\n"
    "program.  Here's a sample program:\n"
    "\n"
    "	#include <windows.h>\n"
    "	main()\n"
    "	{\n"
    "	  time_t t;\n"
    "	  OutputDebugString(\"ssp on\");\n"
    "	  time(&t);\n"
    "	  OutputDebugString(\"ssp off\");\n"
    "	}\n"
    "\n"
    "Then, add the \"-d\" option to ssp to default to *disabling* profiling.\n"
    "The program will run at full speed until the first OutputDebugString,\n"
    "then step until the second.\n"
    "\n"
    "	ssp -d 0x61001000 0x61080000 hello.exe\n"
    "\n"
    "You can then use gprof (as usual) to see the performance profile for\n"
    "just that portion of the program's execution.\n"
    "\n"
    "There are many options to ssp.  Since step-profiling makes your\n"
    "program run about 1,000 times slower than normal, it's best to\n"
    "understand all the options so that you can narrow down the parts\n"
    "of your program you need to single-step.\n"
    "\n"
    "\"-v\" - verbose.  This prints messages about threads starting and\n"
    "stopping, OutputDebugString calls, DLLs loading, etc.\n"
    "\n"
    "\"-t\" and \"-c\" - tracing.  With -t, *every* step's address is written\n"
    "to the file \"trace.ssp\".  This can be used to help debug functions,\n"
    "since it can trace multiple threads.  Clever use of scripts can match\n"
    "addresses with disassembled opcodes if needed.  Warning: creates\n"
    "*huge* files, very quickly.  \"-c\" prints each address to the console,\n"
    "useful for debugging key chunks of assembler.\n"
    "Use \"addr2line -C -f -s -e foo.exe < trace.ssp > lines.ssp\" and then\n"
    "\"perl cvttrace\" to convert to symbolic traces.\n"
    "\n"
    "\"-s\" - subthreads.  Usually, you only need to trace the main thread,\n"
    "but sometimes you need to trace all threads, so this enables that.\n"
    "It's also needed when you want to profile a function that only a\n"
    "subthread calls.  However, using OutputDebugString automatically\n"
    "enables profiling on the thread that called it, not the main thread.\n"
    "\n"
    "\"-l\" - dll profiling.  Generates a pretty table of how much time was\n"
    "spent in each dll the program used.  No sense optimizing a function in\n"
    "your program if most of the time is spent in the DLL.\n"
    "\n"
    "Typically use the -v, -s, and -l options:\n"
    "\n"
    "	ssp -v -s -l -d 0x61001000 0x61080000 hello.exe\n"
    "\n");
  if (stream == stderr)
    fprintf (stream, "Try '%s --help' for more information.\n", prog_name);
  exit (stream == stderr ? 1 : 0);
}

static void
print_version ()
{
  printf ("ssp (cygwin) %d.%d.%d\n"
	  "Single-Step Profiler\n"
	  "Copyright (C) 2000 - %s Red Hat, Inc.\n"
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
  int c, i;
  int total_pcount = 0, total_scount = 0;
  FILE *gmon;

  setbuf (stdout, 0);

  prog_name = program_invocation_short_name;

  while ((c = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (c)
    {
      case 'c':
	printf ("tracing *all* $eip to the console\n");
	trace_console = 1;
	break;
      case 'd':
	printf ("stepping disabled; enable via OutputDebugString (\"ssp on\")\n");
	stepping_enabled = 0;
	break;
      case 'e':
	printf ("stepping enabled; disable via OutputDebugString (\"ssp off\")\n");
	stepping_enabled = 1;
	break;
      case 'h':
	usage (stdout);
	break;
      case 'l':
	printf ("profiling dll usage\n");
	dll_counts = 1;
	break;
      case 's':
	printf ("tracing all sub-threads too, not just the main one\n");
	trace_all_threads = 1;
	break;
      case 't':
	printf ("tracing all $eip to trace.ssp\n");
	tracing_enabled = 1;
	break;
      case 'v':
	printf ("verbose messages enabled\n");
	verbose = 1;
	break;
      case 'V':
	print_version ();
	exit (0);
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n", prog_name);
	exit (1);
    }

  if ( (argc - optind) < 3 )
    usage (stderr);
  sscanf (argv[optind++], "%i", &low_pc);
  sscanf (argv[optind++], "%i", &high_pc);

  if (low_pc > high_pc-8)
    {
      fprintf (stderr, "Hey, low_pc must be lower than high_pc\n");
      exit (1);
    }

  hits = (HISTCOUNTER *)malloc (high_pc-low_pc+4);
  memset (hits, 0, high_pc-low_pc+4);

  fprintf (stderr, "prun: [%08x,%08x] Running '%s'\n",
	  low_pc, high_pc, argv[optind]);

  run_program (argv[optind]);

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

#endif /* !__x86_64 */
