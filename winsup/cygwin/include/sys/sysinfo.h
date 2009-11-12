/* sys/sysinfo.h

   Copyright 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* sys/sysinfo.h header file for Cygwin.  */

#ifndef _SYS_SYSINFO_H
#define _SYS_SYSINFO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

extern int get_nprocs_conf (void);
extern int get_nprocs (void);
extern long get_phys_pages (void);
extern long get_avphys_pages (void);

__END_DECLS

#endif /* _SYS_SYSINFO_H */
