/* sysconf.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <sys/param.h>
#include <sys/sysinfo.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "pinfo.h"
#include "ntdll.h"
#include "tls_pbuf.h"
#include "cpuid.h"
#include "clock.h"

static long
get_open_max (int in)
{
  long max = getdtablesize ();
  if (max < OPEN_MAX)
    max = OPEN_MAX;
  return max;
}

static long
get_page_size (int in)
{
  return wincap.allocation_granularity ();
}

static bool
__nt_query_system (PSYSTEM_BASIC_INFORMATION psbi)
{
  NTSTATUS status;

  status = NtQuerySystemInformation (SystemBasicInformation, (PVOID) psbi,
				     sizeof *psbi, NULL);
  return NT_SUCCESS (status);
}

#define add_size(p,s) ((p) = ((__typeof__(p))((PBYTE)(p)+(s))))

static long
get_nproc_values (int in)
{
  if (!wincap.has_processor_groups ())	/* Pre Windows 7 */
    {
      SYSTEM_BASIC_INFORMATION sbi;

      if (!__nt_query_system (&sbi))
	return -1;
      switch (in)
	{
	case _SC_NPROCESSORS_CONF:
	  return sbi.NumberProcessors;
	case _SC_NPROCESSORS_ONLN:
	  {
	    int i = 0;
	    do
	     if (sbi.ActiveProcessors & 1)
	       i++;
	    while (sbi.ActiveProcessors >>= 1);
	    return i;
	  }
	}
    }

  tmp_pathbuf tp;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX lpi, plpi;
  DWORD lpi_size = NT_MAX_PATH;
  long cnt = 0;

  lpi = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) tp.c_get ();
  if (!GetLogicalProcessorInformationEx (RelationGroup, lpi, &lpi_size))
    return -1;
  plpi = lpi;
  for (DWORD size = lpi_size; size > 0;
       size -= plpi->Size, add_size (plpi, plpi->Size))
    if (plpi->Relationship == RelationGroup)
      {
	for (WORD i = 0; i < plpi->Group.MaximumGroupCount; ++i)
	  switch (in)
	    {
	    case _SC_NPROCESSORS_CONF:
	      cnt += plpi->Group.GroupInfo[0].MaximumProcessorCount;
	      break;
	    case _SC_NPROCESSORS_ONLN:
	      cnt += plpi->Group.GroupInfo[0].ActiveProcessorCount;
	      break;
	    }
      }
  return cnt;
}

static long
get_phys_pages (int in)
{
  SYSTEM_BASIC_INFORMATION sbi;

  if (!__nt_query_system (&sbi))
    return -1;
  return sbi.NumberOfPhysicalPages
	 / (wincap.allocation_granularity () / wincap.page_size ());
}

static long
get_avphys (int in)
{
  NTSTATUS status;
  SYSTEM_PERFORMANCE_INFORMATION spi;

  status = NtQuerySystemInformation (SystemPerformanceInformation,
				     (PVOID) &spi, sizeof spi, NULL);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      debug_printf ("NtQuerySystemInformation: status %y, %E", status);
      return -1;
    }
  return spi.AvailablePages
	 / (wincap.allocation_granularity () / wincap.page_size ());
}

enum cache_level
{
  LevelNone,
  Level1I,
  Level1D,
  Level2,
  Level3,
  Level4
};

struct cpuid2_cache_desc
{
  uint8_t	desc;
  cache_level	level;
  uint32_t	size;
  uint32_t	assoc;
  uint32_t	linesize;
};

static const cpuid2_cache_desc cpuid2_cache_descriptor[] =
{
  { 0x06, Level1I,     8,  4, 32 },
  { 0x08, Level1I,    16,  4, 32 },
  { 0x09, Level1I,    32,  4, 64 },
  { 0x0a, Level1D,     8,  2, 32 },
  { 0x0c, Level1D,    16,  4, 32 },
  { 0x0d, Level1D,    16,  4, 64 },
  { 0x0e, Level1D,    24,  6, 64 },
  { 0x21, Level2,    256,  8, 64 },
  { 0x22, Level3,    512,  4, 64 },
  { 0x23, Level3,   1024,  8, 64 },
  { 0x25, Level3,   2048,  8, 64 },
  { 0x29, Level3,   4096,  8, 64 },
  { 0x2c, Level1D,    32,  8, 64 },
  { 0x30, Level1I,    32,  8, 64 },
  { 0x39, Level2,    128,  4, 64 },
  { 0x3a, Level2,    192,  6, 64 },
  { 0x3b, Level2,    128,  2, 64 },
  { 0x3c, Level2,    256,  4, 64 },
  { 0x3d, Level2,    384,  6, 64 },
  { 0x3e, Level2,    512,  4, 64 },
  { 0x3f, Level2,    256,  2, 64 },
  { 0x41, Level2,    128,  4, 32 },
  { 0x42, Level2,    256,  4, 32 },
  { 0x43, Level2,    512,  4, 32 },
  { 0x44, Level2,   1024,  4, 32 },
  { 0x45, Level2,   2048,  4, 32 },
  { 0x46, Level3,   4096,  4, 64 },
  { 0x47, Level3,   8192,  8, 64 },
  { 0x48, Level2,   3072, 12, 64 },
  { 0x49, Level3,   4096, 16, 64 },
  { 0x4a, Level3,   6144, 12, 64 },
  { 0x4b, Level3,   8192, 16, 64 },
  { 0x4c, Level3,  12288, 12, 64 },
  { 0x4d, Level3,  16384, 16, 64 },
  { 0x4e, Level2,   6144, 24, 64 },
  { 0x60, Level1D,    16,  8, 64 },
  { 0x66, Level1D,     8,  4, 64 },
  { 0x67, Level1D,    16,  4, 64 },
  { 0x68, Level1D,    32,  4, 64 },
  { 0x78, Level2,   1024,  4, 64 },
  { 0x79, Level2,    128,  8, 64 },
  { 0x7a, Level2,    256,  8, 64 },
  { 0x7b, Level2,    512,  8, 64 },
  { 0x7c, Level2,   1024,  8, 64 },
  { 0x7d, Level2,   2048,  8, 64 },
  { 0x7f, Level2,    512,  2, 64 },
  { 0x80, Level2,    512,  8, 64 },
  { 0x82, Level2,    256,  8, 32 },
  { 0x83, Level2,    512,  8, 32 },
  { 0x84, Level2,   1024,  8, 32 },
  { 0x85, Level2,   2048,  8, 32 },
  { 0x86, Level2,    512,  4, 64 },
  { 0x87, Level2,   1024,  8, 64 },
  { 0xd0, Level3,    512,  4, 64 },
  { 0xd1, Level3,   1024,  4, 64 },
  { 0xd2, Level3,   2048,  4, 64 },
  { 0xd6, Level3,   1024,  8, 64 },
  { 0xd7, Level3,   2048,  8, 64 },
  { 0xd8, Level3,   4096, 12, 64 },
  { 0xdc, Level3,   2048, 12, 64 },
  { 0xdd, Level3,   4096, 12, 64 },
  { 0xde, Level3,   8192, 12, 64 },
  { 0xe2, Level3,   2048, 16, 64 },
  { 0xe3, Level3,   4096, 16, 64 },
  { 0xe4, Level3,   8192, 16, 64 },
  { 0xea, Level3,  12288, 24, 64 },
  { 0xeb, Level3,  18432, 24, 64 },
  { 0xec, Level3,  24576, 24, 64 },
};

static int
cpuid2_cache_desc_compar (const void *key, const void *memb)
{
  cpuid2_cache_desc *ckey = (cpuid2_cache_desc *) key;
  cpuid2_cache_desc *cmemb = (cpuid2_cache_desc *) memb;
  return ckey->desc - cmemb->desc;
}

static long
get_cpu_cache_intel_cpuid2 (int in)
{
  uint32_t reg[4];
  long ret = 0;
  int num;

  cpuid (reg, reg + 1, reg + 2, reg + 3, 0x00000002);
  num = reg[0] & 0xff;
  for (int i = 0; i < num; ++i)
    {
      cpuid (reg, reg + 1, reg + 2, reg + 3, 0x00000002);
      for (int r = 0; r < 4; ++r)
	{
	  if (reg[r] & 0x80000000)
	    continue;
	  for (int b = (r == 0) ? 1 : 0; b < 4; ++b)
	    {
	      cpuid2_cache_desc key, *cdp;

	      key.desc = ((uint8_t *) &reg[r])[b];
	      cdp = (cpuid2_cache_desc *)
			bsearch (&key, cpuid2_cache_descriptor,
				 sizeof cpuid2_cache_descriptor
				 / sizeof *cpuid2_cache_descriptor,
				 sizeof *cpuid2_cache_descriptor,
				 cpuid2_cache_desc_compar);
	      if (!cdp)
		continue;
	      switch (in)
		{
		case _SC_LEVEL1_ICACHE_SIZE:
		  if (cdp->level == Level1I)
		    ret += cdp->size * 1024;
		  break;
		case _SC_LEVEL1_ICACHE_ASSOC:
		  if (cdp->level == Level1I)
		    return cdp->assoc;
		  break;
		case _SC_LEVEL1_ICACHE_LINESIZE:
		  if (cdp->level == Level1I)
		    return cdp->linesize;
		  break;
		case _SC_LEVEL1_DCACHE_SIZE:
		  if (cdp->level == Level1D)
		    ret += cdp->size * 1024;
		  break;
		case _SC_LEVEL1_DCACHE_ASSOC:
		  if (cdp->level == Level1D)
		    return cdp->assoc;
		  break;
		case _SC_LEVEL1_DCACHE_LINESIZE:
		  if (cdp->level == Level1D)
		    return cdp->linesize;
		  break;
		case _SC_LEVEL2_CACHE_SIZE:
		  if (cdp->level == Level2)
		    ret += cdp->size * 1024;
		  break;
		case _SC_LEVEL2_CACHE_ASSOC:
		  if (cdp->level == Level2)
		    return cdp->assoc;
		  break;
		case _SC_LEVEL2_CACHE_LINESIZE:
		  if (cdp->level == Level2)
		    return cdp->linesize;
		  break;
		case _SC_LEVEL3_CACHE_SIZE:
		  if (cdp->level == Level3)
		    ret += cdp->size * 1024;
		  break;
		case _SC_LEVEL3_CACHE_ASSOC:
		  if (cdp->level == Level3)
		    return cdp->assoc;
		  break;
		case _SC_LEVEL3_CACHE_LINESIZE:
		  if (cdp->level == Level3)
		    return cdp->linesize;
		  break;
		}
	    }
	}
    }
  return ret;
}

static long
get_cpu_cache_intel_cpuid4 (int in)
{
  uint32_t eax, ebx, ecx, edx;
  long ret = 0;

  for (int idx = 0; ; ++idx)
    {
      uint32_t cache_type, cur_level, assoc, part, linesize, sets;

      cpuid (&eax, &ebx, &ecx, &edx, 0x00000004, idx);
      if ((cache_type = (eax & 0x1f))== 0)
	break;
      cur_level = ((eax >> 5) & 0x7);
      assoc = ((ebx >> 22) & 0x3ff) + 1;
      part = ((ebx >> 12) & 0x3ff) + 1;
      linesize = (ebx & 0xfff) + 1;
      sets = ecx + 1;
      switch (in)
	{
	case _SC_LEVEL1_ICACHE_SIZE:
	  if (cur_level == 1 && cache_type == 2)
	    ret += assoc * part * linesize * sets;
	  break;
	case _SC_LEVEL1_ICACHE_ASSOC:
	  if (cur_level == 1 && cache_type == 2)
	    return assoc;
	case _SC_LEVEL1_ICACHE_LINESIZE:
	  if (cur_level == 1 && cache_type == 2)
	    return linesize;
	case _SC_LEVEL1_DCACHE_SIZE:
	  if (cur_level == 1 && cache_type == 1)
	    ret += assoc * part * linesize * sets;
	  break;
	case _SC_LEVEL1_DCACHE_ASSOC:
	  if (cur_level == 1 && cache_type == 1)
	    return assoc;
	case _SC_LEVEL1_DCACHE_LINESIZE:
	  if (cur_level == 1 && cache_type == 1)
	    return linesize;
	case _SC_LEVEL2_CACHE_SIZE:
	  if (cur_level == 2)
	    ret += assoc * part * linesize * sets;
	  break;
	case _SC_LEVEL2_CACHE_ASSOC:
	  if (cur_level == 2)
	    return assoc;
	case _SC_LEVEL2_CACHE_LINESIZE:
	  if (cur_level == 2)
	    return linesize;
	case _SC_LEVEL3_CACHE_SIZE:
	  if (cur_level == 3)
	    ret += assoc * part * linesize * sets;
	  break;
	case _SC_LEVEL3_CACHE_ASSOC:
	  if (cur_level == 3)
	    return assoc;
	case _SC_LEVEL3_CACHE_LINESIZE:
	  if (cur_level == 3)
	    return linesize;
	}
    }
  return ret;
}

/* Also called from format_proc_cpuinfo */
long
get_cpu_cache_intel (int in, uint32_t maxf)
{
  long ret = 0;

  switch (in)
    {
    case _SC_LEVEL1_ICACHE_SIZE:
    case _SC_LEVEL1_ICACHE_ASSOC:
    case _SC_LEVEL1_ICACHE_LINESIZE:
    case _SC_LEVEL1_DCACHE_SIZE:
    case _SC_LEVEL1_DCACHE_ASSOC:
    case _SC_LEVEL1_DCACHE_LINESIZE:
    case _SC_LEVEL2_CACHE_SIZE:
    case _SC_LEVEL2_CACHE_ASSOC:
    case _SC_LEVEL2_CACHE_LINESIZE:
    case _SC_LEVEL3_CACHE_SIZE:
    case _SC_LEVEL3_CACHE_ASSOC:
    case _SC_LEVEL3_CACHE_LINESIZE:
      if (maxf >= 4)
	ret = get_cpu_cache_intel_cpuid4 (in);
      else if (maxf >= 2)
	ret = get_cpu_cache_intel_cpuid2 (in);
      break;
    default:
      break;
    }
  return ret;
}

static const long assoc[16] = {  0,  1,  2,  2,  4,  4,   8,      8,
				16, 16, 32, 48, 64, 96, 128, 0x8000 };

/* Also called from format_proc_cpuinfo */
long
get_cpu_cache_amd (int in, uint32_t maxe)
{
  uint32_t eax, ebx, ecx, edx;
  long ret = 0;

  if (in >= _SC_LEVEL1_ICACHE_SIZE && in <= _SC_LEVEL1_DCACHE_LINESIZE
      && maxe >= 0x80000005)
    cpuid (&eax, &ebx, &ecx, &edx, 0x80000005);
  else if (in >= _SC_LEVEL2_CACHE_SIZE && in <= _SC_LEVEL3_CACHE_LINESIZE
	   && maxe >= 0x80000006)
    cpuid (&eax, &ebx, &ecx, &edx, 0x80000006);
  else
    return 0;

  switch (in)
    {
    case _SC_LEVEL1_ICACHE_SIZE:
      ret = (edx & 0xff000000) >> 14;
      break;
    case _SC_LEVEL1_ICACHE_ASSOC:
      ret = (edx & 0xff0000) >> 16;
      if (ret == 0xff)
	ret = 0x8000;
      break;
    case _SC_LEVEL1_ICACHE_LINESIZE:
      ret = (edx & 0xff);
      break;
    case _SC_LEVEL1_DCACHE_SIZE:
      ret = (ecx & 0xff000000) >> 14;
      break;
    case _SC_LEVEL1_DCACHE_ASSOC:
      ret = (ecx & 0xff0000) >> 16;
      if (ret == 0xff)
	ret = 0x8000;
      break;
    case _SC_LEVEL1_DCACHE_LINESIZE:
      ret = (ecx & 0xff);
      break;
    case _SC_LEVEL2_CACHE_SIZE:
      ret = (ecx & 0xffff0000) >> 6;
      break;
    case _SC_LEVEL2_CACHE_ASSOC:
      ret = assoc[(ecx & 0xf000) >> 12];
      break;
    case _SC_LEVEL2_CACHE_LINESIZE:
      ret = (ecx & 0xff);
      break;
    case _SC_LEVEL3_CACHE_SIZE:
      ret = (long) ((edx & 0xfffc0000) >> 18) * 512 * 1024;
      break;
    case _SC_LEVEL3_CACHE_ASSOC:
      ret = assoc[(edx & 0xf000) >> 12];
      break;
    case _SC_LEVEL3_CACHE_LINESIZE:
      ret = (edx & 0xff);
      break;
    default:
      break;
    }
  return ret;
}

static long
get_cpu_cache (int in)
{
  uint32_t maxf, vendor_id[4];
  cpuid (&maxf, &vendor_id[0], &vendor_id[2], &vendor_id[1], 0x00000000);

  vendor_id[3] = 0;
  if (!strcmp ((char*) vendor_id, "GenuineIntel"))
    return get_cpu_cache_intel (in, maxf & 0xffff);
  else if (!strcmp ((char*)vendor_id, "AuthenticAMD")
           || !strcmp((char*)vendor_id, "HygonGenuine"))
    {
      uint32_t maxe = 0, unused;
      cpuid (&maxe, &unused, &unused, &unused, 0x80000000);
      return get_cpu_cache_amd (in, maxe);
    }
  return 0;
}

enum sc_type { nsup, cons, func };

static struct
{
  sc_type type;
  union
    {
      long c;
      long (*f)(int);
    };
} sca[] =
{
  {cons, {c:ARG_MAX}},			/*   0, _SC_ARG_MAX */
  {cons, {c:CHILD_MAX}},		/*   1, _SC_CHILD_MAX */
  {cons, {c:CLOCKS_PER_SEC}},		/*   2, _SC_CLK_TCK */
  {cons, {c:NGROUPS_MAX}},		/*   3, _SC_NGROUPS_MAX */
  {func, {f:get_open_max}},		/*   4, _SC_OPEN_MAX */
  {cons, {c:_POSIX_JOB_CONTROL}},	/*   5, _SC_JOB_CONTROL */
  {cons, {c:_POSIX_SAVED_IDS}},		/*   6, _SC_SAVED_IDS */
  {cons, {c:_POSIX_VERSION}},		/*   7, _SC_VERSION */
  {func, {f:get_page_size}},		/*   8, _SC_PAGESIZE */
  {func, {f:get_nproc_values}},		/*   9, _SC_NPROCESSORS_CONF */
  {func, {f:get_nproc_values}},		/*  10, _SC_NPROCESSORS_ONLN */
  {func, {f:get_phys_pages}},		/*  11, _SC_PHYS_PAGES */
  {func, {f:get_avphys}},		/*  12, _SC_AVPHYS_PAGES */
  {cons, {c:MQ_OPEN_MAX}},		/*  13, _SC_MQ_OPEN_MAX */
  {cons, {c:MQ_PRIO_MAX}},		/*  14, _SC_MQ_PRIO_MAX */
  {cons, {c:RTSIG_MAX}},		/*  15, _SC_RTSIG_MAX */
  {cons, {c:-1L}},			/*  16, _SC_SEM_NSEMS_MAX */
  {cons, {c:SEM_VALUE_MAX}},		/*  17, _SC_SEM_VALUE_MAX */
  {cons, {c:SIGQUEUE_MAX}},		/*  18, _SC_SIGQUEUE_MAX */
  {cons, {c:TIMER_MAX}},		/*  19, _SC_TIMER_MAX */
  {nsup, {c:0}},			/*  20, _SC_TZNAME_MAX */
  {cons, {c:_POSIX_ASYNCHRONOUS_IO}},	/*  21, _SC_ASYNCHRONOUS_IO */
  {cons, {c:_POSIX_FSYNC}},		/*  22, _SC_FSYNC */
  {cons, {c:_POSIX_MAPPED_FILES}},	/*  23, _SC_MAPPED_FILES */
  {cons, {c:-1L}},			/*  24, _SC_MEMLOCK */
  {cons, {c:_POSIX_MEMLOCK_RANGE}},	/*  25, _SC_MEMLOCK_RANGE */
  {cons, {c:_POSIX_MEMORY_PROTECTION}},	/*  26, _SC_MEMORY_PROTECTION */
  {cons, {c:_POSIX_MESSAGE_PASSING}},	/*  27, _SC_MESSAGE_PASSING */
  {cons, {c:-1L}},			/*  28, _SC_PRIORITIZED_IO */
  {cons, {c:_POSIX_REALTIME_SIGNALS}},	/*  29, _SC_REALTIME_SIGNALS */
  {cons, {c:_POSIX_SEMAPHORES}},	/*  30, _SC_SEMAPHORES */
  {cons, {c:_POSIX_SHARED_MEMORY_OBJECTS}},	/*  31, _SC_SHARED_MEMORY_OBJECTS */
  {cons, {c:_POSIX_SYNCHRONIZED_IO}},	/*  32, _SC_SYNCHRONIZED_IO */
  {cons, {c:_POSIX_TIMERS}},		/*  33, _SC_TIMERS */
  {cons, {c:AIO_LISTIO_MAX}},		/*  34, _SC_AIO_LISTIO_MAX */
  {cons, {c:AIO_MAX}},			/*  35, _SC_AIO_MAX */
  {cons, {c:AIO_PRIO_DELTA_MAX}},	/*  36, _SC_AIO_PRIO_DELTA_MAX */
  {nsup, {c:0}},			/*  37, _SC_DELAYTIMER_MAX */
  {cons, {c:PTHREAD_KEYS_MAX}},		/*  38, _SC_THREAD_KEYS_MAX */
  {cons, {c:PTHREAD_STACK_MIN}},	/*  39, _SC_THREAD_STACK_MIN */
  {cons, {c:-1L}},			/*  40, _SC_THREAD_THREADS_MAX */
  {cons, {c:TTY_NAME_MAX}},		/*  41, _SC_TTY_NAME_MAX */
  {cons, {c:_POSIX_THREADS}},		/*  42, _SC_THREADS */
  {cons, {c:_POSIX_THREAD_ATTR_STACKADDR}},/*  43, _SC_THREAD_ATTR_STACKADDR */
  {cons, {c:_POSIX_THREAD_ATTR_STACKSIZE}},/*  44, _SC_THREAD_ATTR_STACKSIZE */
  {cons, {c:_POSIX_THREAD_PRIORITY_SCHEDULING}},	/*  45, _SC_THREAD_PRIORITY_SCHEDULING */
  {cons, {c:-1L}},			/*  46, _SC_THREAD_PRIO_INHERIT */
  {cons, {c:-1L}},			/*  47, _SC_THREAD_PRIO_PROTECT */
  {cons, {c:_POSIX_THREAD_PROCESS_SHARED}},	/*  48, _SC_THREAD_PROCESS_SHARED */
  {cons, {c:_POSIX_THREAD_SAFE_FUNCTIONS}},	/*  49, _SC_THREAD_SAFE_FUNCTIONS */
  {cons, {c:16384L}},			/*  50, _SC_GETGR_R_SIZE_MAX */
  {cons, {c:16384L}},			/*  51, _SC_GETPW_R_SIZE_MAX */
  {cons, {c:LOGIN_NAME_MAX}},		/*  52, _SC_LOGIN_NAME_MAX */
  {cons, {c:PTHREAD_DESTRUCTOR_ITERATIONS}},	/*  53, _SC_THREAD_DESTRUCTOR_ITERATIONS */
  {cons, {c:_POSIX_ADVISORY_INFO}},	/*  54, _SC_ADVISORY_INFO */
  {cons, {c:ATEXIT_MAX}},		/*  55, _SC_ATEXIT_MAX */
  {cons, {c:_POSIX_BARRIERS}},		/*  56, _SC_BARRIERS */
  {cons, {c:BC_BASE_MAX}},		/*  57, _SC_BC_BASE_MAX */
  {cons, {c:BC_DIM_MAX}},		/*  58, _SC_BC_DIM_MAX */
  {cons, {c:BC_SCALE_MAX}},		/*  59, _SC_BC_SCALE_MAX */
  {cons, {c:BC_STRING_MAX}},		/*  60, _SC_BC_STRING_MAX */
  {cons, {c:_POSIX_CLOCK_SELECTION}},	/*  61, _SC_CLOCK_SELECTION */
  {nsup, {c:0}},			/*  62, _SC_COLL_WEIGHTS_MAX */
  {cons, {c:_POSIX_CPUTIME}},		/*  63, _SC_CPUTIME */
  {cons, {c:EXPR_NEST_MAX}},		/*  64, _SC_EXPR_NEST_MAX */
  {cons, {c:HOST_NAME_MAX}},		/*  65, _SC_HOST_NAME_MAX */
  {cons, {c:IOV_MAX}},			/*  66, _SC_IOV_MAX */
  {cons, {c:_POSIX_IPV6}},		/*  67, _SC_IPV6 */
  {cons, {c:LINE_MAX}},			/*  68, _SC_LINE_MAX */
  {cons, {c:_POSIX_MONOTONIC_CLOCK}},	/*  69, _SC_MONOTONIC_CLOCK */
  {cons, {c:_POSIX_RAW_SOCKETS}},	/*  70, _SC_RAW_SOCKETS */
  {cons, {c:_POSIX_READER_WRITER_LOCKS}},	/*  71, _SC_READER_WRITER_LOCKS */
  {cons, {c:_POSIX_REGEXP}},		/*  72, _SC_REGEXP */
  {cons, {c:RE_DUP_MAX}},		/*  73, _SC_RE_DUP_MAX */
  {cons, {c:_POSIX_SHELL}},		/*  74, _SC_SHELL */
  {cons, {c:_POSIX_SPAWN}},		/*  75, _SC_SPAWN */
  {cons, {c:_POSIX_SPIN_LOCKS}},	/*  76, _SC_SPIN_LOCKS */
  {cons, {c:-1L}},			/*  77, _SC_SPORADIC_SERVER */
  {nsup, {c:0}},			/*  78, _SC_SS_REPL_MAX */
  {cons, {c:SYMLOOP_MAX}},		/*  79, _SC_SYMLOOP_MAX */
  {cons, {c:_POSIX_THREAD_CPUTIME}},	/*  80, _SC_THREAD_CPUTIME */
  {cons, {c:-1L}},			/*  81, _SC_THREAD_SPORADIC_SERVER */
  {cons, {c:_POSIX_TIMEOUTS}},		/*  82, _SC_TIMEOUTS */
  {cons, {c:-1L}},			/*  83, _SC_TRACE */
  {cons, {c:-1L}},			/*  84, _SC_TRACE_EVENT_FILTER */
  {nsup, {c:0}},			/*  85, _SC_TRACE_EVENT_NAME_MAX */
  {cons, {c:-1L}},			/*  86, _SC_TRACE_INHERIT */
  {cons, {c:-1L}},			/*  87, _SC_TRACE_LOG */
  {nsup, {c:0}},			/*  88, _SC_TRACE_NAME_MAX */
  {nsup, {c:0}},			/*  89, _SC_TRACE_SYS_MAX */
  {nsup, {c:0}},			/*  90, _SC_TRACE_USER_EVENT_MAX */
  {cons, {c:-1L}},			/*  91, _SC_TYPED_MEMORY_OBJECTS */
  {cons, {c:_POSIX_V6_ILP32_OFF32}},	/*  92, _SC_V6_ILP32_OFF32 */
  {cons, {c:_POSIX_V6_ILP32_OFFBIG}},	/*  93, _SC_V6_ILP32_OFFBIG */
  {cons, {c:_POSIX_V6_LP64_OFF64}},	/*  94, _SC_V6_LP64_OFF64 */
  {cons, {c:_POSIX_V6_LPBIG_OFFBIG}},	/*  95, _SC_V6_LPBIG_OFFBIG */
  {cons, {c:_XOPEN_CRYPT}},		/*  96, _SC_XOPEN_CRYPT */
  {cons, {c:_XOPEN_ENH_I18N}},		/*  97, _SC_XOPEN_ENH_I18N */
  {cons, {c:-1L}},			/*  98, _SC_XOPEN_LEGACY */
  {cons, {c:-1L}},			/*  99, _SC_XOPEN_REALTIME */
  {cons, {c:STREAM_MAX}},		/* 100, _SC_STREAM_MAX */
  {cons, {c:_POSIX_PRIORITY_SCHEDULING}},	/* 101, _SC_PRIORITY_SCHEDULING */
  {cons, {c:-1L}},			/* 102, _SC_XOPEN_REALTIME_THREADS */
  {cons, {c:_XOPEN_SHM}},		/* 103, _SC_XOPEN_SHM */
  {cons, {c:-1L}},			/* 104, _SC_XOPEN_STREAMS */
  {cons, {c:-1L}},			/* 105, _SC_XOPEN_UNIX */
  {cons, {c:_XOPEN_VERSION}},		/* 106, _SC_XOPEN_VERSION */
  {cons, {c:_POSIX2_CHAR_TERM}},	/* 107, _SC_2_CHAR_TERM */
  {cons, {c:_POSIX2_C_BIND}},		/* 108, _SC_2_C_BIND */
  {cons, {c:_POSIX2_C_DEV}},		/* 109, _SC_2_C_DEV */
  {cons, {c:-1L}},			/* 110, _SC_2_FORT_DEV */
  {cons, {c:-1L}},			/* 111, _SC_2_FORT_RUN */
  {cons, {c:-1L}},			/* 112, _SC_2_LOCALEDEF */
  {cons, {c:-1L}},			/* 113, _SC_2_PBS */
  {cons, {c:-1L}},			/* 114, _SC_2_PBS_ACCOUNTING */
  {cons, {c:-1L}},			/* 115, _SC_2_PBS_CHECKPOINT */
  {cons, {c:-1L}},			/* 116, _SC_2_PBS_LOCATE */
  {cons, {c:-1L}},			/* 117, _SC_2_PBS_MESSAGE */
  {cons, {c:-1L}},			/* 118, _SC_2_PBS_TRACK */
  {cons, {c:_POSIX2_SW_DEV}},		/* 119, _SC_2_SW_DEV */
  {cons, {c:_POSIX2_UPE}},		/* 120, _SC_2_UPE */
  {cons, {c:_POSIX2_VERSION}},		/* 121, _SC_2_VERSION */
  {cons, {c:-1L}},			/* 122, _SC_THREAD_ROBUST_PRIO_INHERIT */
  {cons, {c:-1L}},			/* 123, _SC_THREAD_ROBUST_PRIO_PROTECT */
  {cons, {c:-1L}},			/* 124, _SC_XOPEN_UUCP */
  {func, {f:get_cpu_cache}},		/* 125, _SC_LEVEL1_ICACHE_SIZE */
  {func, {f:get_cpu_cache}},		/* 126, _SC_LEVEL1_ICACHE_ASSOC */
  {func, {f:get_cpu_cache}},		/* 127, _SC_LEVEL1_ICACHE_LINESIZE */
  {func, {f:get_cpu_cache}},		/* 128, _SC_LEVEL1_DCACHE_SIZE */
  {func, {f:get_cpu_cache}},		/* 129, _SC_LEVEL1_DCACHE_ASSOC */
  {func, {f:get_cpu_cache}},		/* 130, _SC_LEVEL1_DCACHE_LINESIZE */
  {func, {f:get_cpu_cache}},		/* 131, _SC_LEVEL2_CACHE_SIZE */
  {func, {f:get_cpu_cache}},		/* 132, _SC_LEVEL2_CACHE_ASSOC */
  {func, {f:get_cpu_cache}},		/* 133, _SC_LEVEL2_CACHE_LINESIZE */
  {func, {f:get_cpu_cache}},		/* 134, _SC_LEVEL3_CACHE_SIZE */
  {func, {f:get_cpu_cache}},		/* 135, _SC_LEVEL3_CACHE_ASSOC */
  {func, {f:get_cpu_cache}},		/* 136, _SC_LEVEL3_CACHE_LINESIZE */
  {func, {f:get_cpu_cache}},		/* 137, _SC_LEVEL4_CACHE_SIZE */
  {func, {f:get_cpu_cache}},		/* 138, _SC_LEVEL4_CACHE_ASSOC */
  {func, {f:get_cpu_cache}},		/* 139, _SC_LEVEL4_CACHE_LINESIZE */
};

#define SC_MIN _SC_ARG_MAX
#define SC_MAX _SC_LEVEL4_CACHE_LINESIZE

/* sysconf: POSIX 4.8.1.1 */
/* Allows a portable app to determine quantities of resources or
   presence of an option at execution time. */
long int
sysconf (int in)
{
  if (in >= SC_MIN && in <= SC_MAX)
    {
      switch (sca[in].type)
	{
	case nsup:
	  break;
	case cons:
	  return sca[in].c;
	case func:
	  return sca[in].f (in);
	}
    }
  /* Unimplemented sysconf name or invalid option value. */
  set_errno (EINVAL);
  return -1L;
}

#define ls(s)	sizeof(s),s

static struct
{
  size_t l;
  const char *s;
} csa[] =
{
  {ls ("/bin:/usr/bin")},		/* _CS_PATH */
  {0, NULL},				/* _CS_POSIX_V6_ILP32_OFF32_CFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_ILP32_OFF32_LDFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_ILP32_OFF32_LIBS */
  {0, NULL},				/* _CS_XBS5_ILP32_OFF32_LINTFLAGS */
#ifdef __x86_64__
  {0, NULL},				/* _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_ILP32_OFFBIG_LIBS */
  {0, NULL},				/* _CS_XBS5_ILP32_OFFBIG_LINTFLAGS */
  {ls ("")},				/* _CS_POSIX_V6_LP64_OFF64_CFLAGS */
  {ls ("")},				/* _CS_POSIX_V6_LP64_OFF64_LDFLAGS */
  {ls ("")},				/* _CS_POSIX_V6_LP64_OFF64_LIBS */
  {ls ("")},				/* _CS_XBS5_LP64_OFF64_LINTFLAGS */
  {ls ("")},				/* _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS */
  {ls ("")},				/* _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS */
  {ls ("")},				/* _CS_POSIX_V6_LPBIG_OFFBIG_LIBS */
  {ls ("")},				/* _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS */
  {ls ("POSIX_V6_LP64_OFF64")},		/* _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS */
#else
  {ls ("")},				/* _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS */
  {ls ("")},				/* _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS */
  {ls ("")},				/* _CS_POSIX_V6_ILP32_OFFBIG_LIBS */
  {ls ("")},				/* _CS_XBS5_ILP32_OFFBIG_LINTFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_LP64_OFF64_CFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_LP64_OFF64_LDFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_LP64_OFF64_LIBS */
  {0, NULL},				/* _CS_XBS5_LP64_OFF64_LINTFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS */
  {0, NULL},				/* _CS_POSIX_V6_LPBIG_OFFBIG_LIBS */
  {0, NULL},				/* _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS */
  {ls ("POSIX_V6_ILP32_OFFBIG")},	/* _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS */
#endif
  {ls ("")},				/* _CS_POSIX_V7_THREADS_CFLAGS */
  {ls ("")},				/* _CS_POSIX_V7_THREADS_LDFLAGS */
  {ls ("POSIXLY_CORRECT=1")},		/* _CS_V7_ENV */
  {ls ("")},				/* _CS_LFS_CFLAGS */
  {ls ("")},				/* _CS_LFS_LDFLAGS */
  {ls ("")},				/* _CS_LFS_LIBS */
  {ls ("")},				/* _CS_LFS_LINTFLAGS */
};

#define CS_MIN _CS_PATH
#define CS_MAX _CS_LFS_LINTFLAGS

extern "C" size_t
confstr (int in, char *buf, size_t len)
{
  if (in >= CS_MIN && in <= CS_MAX)
    {
      if (csa[in].l && len)
	{
	  buf[0] = 0;
	  strncat (buf, csa[in].s, MIN (len, csa[in].l) - 1);
	}
      return csa[in].l;
    }
  /* Invalid option value. */
  set_errno (EINVAL);
  return 0;
}

extern "C" int
get_nprocs_conf (void)
{
  return get_nproc_values (_SC_NPROCESSORS_CONF);
}

extern "C" int
get_nprocs (void)
{
  return get_nproc_values (_SC_NPROCESSORS_ONLN);
}

extern "C" long
get_phys_pages (void)
{
  return get_phys_pages (_SC_PHYS_PAGES);
}

extern "C" long
get_avphys_pages (void)
{
  return get_avphys (_SC_AVPHYS_PAGES);
}

extern "C" int
sysinfo (struct sysinfo *info)
{
  unsigned long long uptime = 0ULL, totalram = 0ULL, freeram = 0ULL,
		totalswap = 0ULL, freeswap = 0ULL;
  MEMORYSTATUSEX memory_status;
  PSYSTEM_PAGEFILE_INFORMATION spi = NULL;
  ULONG sizeof_spi = 512;
  PSYSTEM_TIMEOFDAY_INFORMATION stodi = NULL;
  const ULONG sizeof_stodi = sizeof (SYSTEM_TIMEOFDAY_INFORMATION);
  NTSTATUS status = STATUS_SUCCESS;
  winpids pids ((DWORD) 0);

  if (!info)
    {
      set_errno (EFAULT);
      return -1;
    }

  stodi = (PSYSTEM_TIMEOFDAY_INFORMATION) malloc (sizeof_stodi);
  status = NtQuerySystemInformation (SystemTimeOfDayInformation, (PVOID) stodi,
				     sizeof_stodi, NULL);
  if (NT_SUCCESS (status))
    uptime = (stodi->CurrentTime.QuadPart - stodi->BootTime.QuadPart)
	     / NS100PERSEC;
  else
    debug_printf ("NtQuerySystemInformation(SystemTimeOfDayInformation), "
		  "status %y", status);

  if (stodi)
    free (stodi);

  memory_status.dwLength = sizeof (MEMORYSTATUSEX);
  GlobalMemoryStatusEx (&memory_status);
  totalram = memory_status.ullTotalPhys / wincap.page_size ();
  freeram = memory_status.ullAvailPhys / wincap.page_size ();

  spi = (PSYSTEM_PAGEFILE_INFORMATION) malloc (sizeof_spi);
  if (spi)
    {
      status = NtQuerySystemInformation (SystemPagefileInformation, (PVOID) spi,
					 sizeof_spi, &sizeof_spi);
      if (status == STATUS_INFO_LENGTH_MISMATCH)
	{
	  free (spi);
	  spi = (PSYSTEM_PAGEFILE_INFORMATION) malloc (sizeof_spi);
	  if (spi)
	    status = NtQuerySystemInformation (SystemPagefileInformation,
					       (PVOID) spi, sizeof_spi,
					       &sizeof_spi);
	}
    }
  if (!spi || !NT_SUCCESS (status))
    {
      debug_printf ("NtQuerySystemInformation(SystemPagefileInformation), "
		    "status %y", status);
      totalswap = (memory_status.ullTotalPageFile - memory_status.ullTotalPhys)
		  / wincap.page_size ();
      freeswap = (memory_status.ullAvailPageFile - memory_status.ullTotalPhys)
		 / wincap.page_size ();
    }
  else
    {
      PSYSTEM_PAGEFILE_INFORMATION spp = spi;
      do
	{
	  totalswap += spp->CurrentSize;
	  freeswap += spp->CurrentSize - spp->TotalUsed;
	}
      while (spp->NextEntryOffset
	     && (spp = (PSYSTEM_PAGEFILE_INFORMATION)
			   ((char *) spp + spp->NextEntryOffset)));
    }
  if (spi)
    free (spi);

  info->uptime = (long) uptime;
  info->totalram = (unsigned long) totalram;
  info->freeram = (unsigned long) freeram;
  info->totalswap = (unsigned long) totalswap;
  info->freeswap = (unsigned long) freeswap;
  info->procs = (unsigned short) pids.npids;
  info->mem_unit = (unsigned int) wincap.page_size ();

  /* FIXME: unsupported */
  info->loads[0] = 0UL;
  info->loads[1] = 0UL;
  info->loads[2] = 0UL;
  info->sharedram = 0UL;
  info->bufferram = 0UL;
  info->totalhigh = 0UL;
  info->freehigh = 0UL;

  return 0;
}
