/* Copyright (c) 2005 Jeff Johnston <jjohnstn@redhat.com> */
#include <sys/types.h>
#include "local.h"

/* Shared timezone information for libc/time functions.  */
static __tzinfo_type tzinfo;

__tzinfo_type *
__gettzinfo (void)
{
  return &tzinfo;
}
