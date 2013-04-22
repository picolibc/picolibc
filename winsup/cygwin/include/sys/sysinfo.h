/* sys/sysinfo.h

   Copyright 2009, 2011, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* sys/sysinfo.h header file for Cygwin.  */

#ifndef _SYS_SYSINFO_H
#define _SYS_SYSINFO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

struct sysinfo {
  long uptime;                /* Seconds since boot */
  unsigned long loads[3];     /* 1, 5, and 15 minute load averages */
  unsigned long totalram;     /* Total usable main memory size */
  unsigned long freeram;      /* Available memory size */
  unsigned long sharedram;    /* Amount of shared memory */
  unsigned long bufferram;    /* Memory used by buffers */
  unsigned long totalswap;    /* Total swap space size */
  unsigned long freeswap;     /* swap space still available */
  unsigned short procs;       /* Number of current processes */
  unsigned long totalhigh;    /* Total high memory size */
  unsigned long freehigh;     /* Available high memory size */
  unsigned int mem_unit;      /* Memory unit size in bytes */
  char __f[10];               /* Pads structure to 64 bytes */
};

extern int sysinfo (struct sysinfo *);
extern int get_nprocs_conf (void);
extern int get_nprocs (void);
extern long get_phys_pages (void);
extern long get_avphys_pages (void);

__END_DECLS

#endif /* _SYS_SYSINFO_H */
