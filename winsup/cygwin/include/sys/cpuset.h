/* sys/cpuset.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_CPUSET_H_
#define _SYS_CPUSET_H_

typedef __SIZE_TYPE__ __cpu_mask;
#define __CPU_SETSIZE 1024  // maximum number of logical processors tracked
#define __NCPUBITS (8 * sizeof (__cpu_mask))  // max size of processor group
#define __CPU_GROUPMAX (__CPU_SETSIZE / __NCPUBITS)  // maximum group number

#define __CPUELT(cpu)   ((cpu) / __NCPUBITS)
#define __CPUMASK(cpu)  ((__cpu_mask) 1 << ((cpu) % __NCPUBITS))

typedef struct
{
  __cpu_mask __bits[__CPU_GROUPMAX];
} cpu_set_t;

#endif /* _SYS_CPUSET_H_ */
