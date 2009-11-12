/* sysconf.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <sys/sysinfo.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "ntdll.h"

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
  return getpagesize ();
}

static long
get_nproc_values (int in)
{
  NTSTATUS ret;
  SYSTEM_BASIC_INFORMATION sbi;
  if ((ret = NtQuerySystemInformation (SystemBasicInformation, (PVOID) &sbi,
				       sizeof sbi, NULL)) != STATUS_SUCCESS)
    {
      __seterrno_from_nt_status (ret);
      debug_printf ("NtQuerySystemInformation: ret %d, Dos(ret) %E",
		    ret);
      return -1;
    }
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
    case _SC_PHYS_PAGES:
      return sbi.NumberOfPhysicalPages
	     / (getpagesize () / getsystempagesize ());
    }
  return -1;
}

static long
get_avphys (int in)
{
  NTSTATUS ret;
  SYSTEM_PERFORMANCE_INFORMATION spi;
  if ((ret = NtQuerySystemInformation (SystemPerformanceInformation,
				       (PVOID) &spi, sizeof spi, NULL))
      != STATUS_SUCCESS)
    {
      __seterrno_from_nt_status (ret);
      debug_printf ("NtQuerySystemInformation: ret %d, Dos(ret) %E",
		    ret);
      return -1;
    }
  return spi.AvailablePages / (getpagesize () / getsystempagesize ());
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
  {func, {f:get_nproc_values}},		/*  11, _SC_PHYS_PAGES */
  {func, {f:get_avphys}},		/*  12, _SC_AVPHYS_PAGES */
  {cons, {c:MQ_OPEN_MAX}},		/*  13, _SC_MQ_OPEN_MAX */
  {cons, {c:MQ_PRIO_MAX}},		/*  14, _SC_MQ_PRIO_MAX */
  {cons, {c:RTSIG_MAX}},		/*  15, _SC_RTSIG_MAX */
  {cons, {c:-1L}},			/*  16, _SC_SEM_NSEMS_MAX */
  {cons, {c:SEM_VALUE_MAX}},		/*  17, _SC_SEM_VALUE_MAX */
  {cons, {c:SIGQUEUE_MAX}},		/*  18, _SC_SIGQUEUE_MAX */
  {cons, {c:TIMER_MAX}},		/*  19, _SC_TIMER_MAX */
  {nsup, {c:0}},			/*  20, _SC_TZNAME_MAX */
  {cons, {c:-1L}},			/*  21, _SC_ASYNCHRONOUS_IO */
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
  {nsup, {c:0}},			/*  34, _SC_AIO_LISTIO_MAX */
  {nsup, {c:0}},			/*  35, _SC_AIO_MAX */
  {nsup, {c:0}},			/*  36, _SC_AIO_PRIO_DELTA_MAX */
  {nsup, {c:0}},			/*  37, _SC_DELAYTIMER_MAX */
  {cons, {c:PTHREAD_KEYS_MAX}},		/*  38, _SC_THREAD_KEYS_MAX */
  {cons, {c:PTHREAD_STACK_MIN}},	/*  39, _SC_THREAD_STACK_MIN */
  {cons, {c:-1L}},			/*  40, _SC_THREAD_THREADS_MAX */
  {cons, {c:TTY_NAME_MAX}},		/*  41, _SC_TTY_NAME_MAX */
  {cons, {c:_POSIX_THREADS}},		/*  42, _SC_THREADS */
  {cons, {c:-1L}},			/*  43, _SC_THREAD_ATTR_STACKADDR */
  {cons, {c:_POSIX_THREAD_ATTR_STACKSIZE}},	/*  44, _SC_THREAD_ATTR_STACKSIZE */
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
  {cons, {c:-1L}},			/*  56, _SC_BARRIERS */
  {cons, {c:BC_BASE_MAX}},		/*  57, _SC_BC_BASE_MAX */
  {cons, {c:BC_DIM_MAX}},		/*  58, _SC_BC_DIM_MAX */
  {cons, {c:BC_SCALE_MAX}},		/*  59, _SC_BC_SCALE_MAX */
  {cons, {c:BC_STRING_MAX}},		/*  60, _SC_BC_STRING_MAX */
  {cons, {c:-1L}},			/*  61, _SC_CLOCK_SELECTION */
  {nsup, {c:0}},			/*  62, _SC_COLL_WEIGHTS_MAX */
  {cons, {c:-1L}},			/*  63, _SC_CPUTIME */
  {cons, {c:EXPR_NEST_MAX}},		/*  64, _SC_EXPR_NEST_MAX */
  {cons, {c:HOST_NAME_MAX}},		/*  65, _SC_HOST_NAME_MAX */
  {cons, {c:IOV_MAX}},			/*  66, _SC_IOV_MAX */
  {cons, {c:_POSIX_IPV6}},		/*  67, _SC_IPV6 */
  {cons, {c:LINE_MAX}},			/*  68, _SC_LINE_MAX */
  {cons, {c:-1L}},			/*  69, _SC_MONOTONIC_CLOCK */
  {cons, {c:_POSIX_RAW_SOCKETS}},	/*  70, _SC_RAW_SOCKETS */
  {cons, {c:_POSIX_READER_WRITER_LOCKS}},	/*  71, _SC_READER_WRITER_LOCKS */
  {cons, {c:_POSIX_REGEXP}},		/*  72, _SC_REGEXP */
  {cons, {c:RE_DUP_MAX}},		/*  73, _SC_RE_DUP_MAX */
  {cons, {c:_POSIX_SHELL}},		/*  74, _SC_SHELL */
  {cons, {c:-1L}},			/*  75, _SC_SPAWN */
  {cons, {c:-1L}},			/*  76, _SC_SPIN_LOCKS */
  {cons, {c:-1L}},			/*  77, _SC_SPORADIC_SERVER */
  {nsup, {c:0}},			/*  78, _SC_SS_REPL_MAX */
  {cons, {c:SYMLOOP_MAX}},		/*  79, _SC_SYMLOOP_MAX */
  {cons, {c:-1L}},			/*  80, _SC_THREAD_CPUTIME */
  {cons, {c:-1L}},			/*  81, _SC_THREAD_SPORADIC_SERVER */
  {cons, {c:-1L}},			/*  82, _SC_TIMEOUTS */
  {cons, {c:-1L}},			/*  83, _SC_TRACE */
  {cons, {c:-1L}},			/*  84, _SC_TRACE_EVENT_FILTER */
  {nsup, {c:0}},			/*  85, _SC_TRACE_EVENT_NAME_MAX */
  {cons, {c:-1L}},			/*  86, _SC_TRACE_INHERIT */
  {cons, {c:-1L}},			/*  87, _SC_TRACE_LOG */
  {nsup, {c:0}},			/*  88, _SC_TRACE_NAME_MAX */
  {nsup, {c:0}},			/*  89, _SC_TRACE_SYS_MAX */
  {nsup, {c:0}},			/*  90, _SC_TRACE_USER_EVENT_MAX */
  {cons, {c:-1L}},			/*  91, _SC_TYPED_MEMORY_OBJECTS */
  {cons, {c:-1L}},			/*  92, _SC_V6_ILP32_OFF32 */
  {cons, {c:_POSIX_V6_ILP32_OFFBIG}},	/*  93, _SC_V6_ILP32_OFFBIG */
  {cons, {c:-1L}},			/*  94, _SC_V6_LP64_OFF64 */
  {cons, {c:-1L}},			/*  95, _SC_V6_LPBIG_OFFBIG */
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
  {cons, {c:_POSIX2_C_BIND}},		/* 109, _SC_2_C_DEV */
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
};

#define SC_MIN _SC_ARG_MAX
#define SC_MAX _SC_2_VERSION

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
  {0, NULL},				/* _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS */
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
};

#define CS_MIN _CS_PATH
#define CS_MAX _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS

extern "C" size_t
confstr (int in, char *buf, size_t len)
{
  if (in >= CS_MIN && in <= CS_MAX)
    {
      if (csa[in].l && len)
	{
	  buf[0] = 0;
	  strncat (buf, csa[in].s, min (len, csa[in].l) - 1);
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
  return get_nproc_values (_SC_PHYS_PAGES);
}

extern "C" long
get_avphys_pages (void)
{
  return get_avphys (_SC_AVPHYS_PAGES);
}
