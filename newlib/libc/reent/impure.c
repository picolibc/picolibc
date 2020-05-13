/*
Copyright (c) 1994 Cygnus Support.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
endorse or promote products derived from this software without
specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <reent.h>

/* Note that there is a copy of this in sys/reent.h.  */
#ifndef __ATTRIBUTE_IMPURE_PTR__
#define __ATTRIBUTE_IMPURE_PTR__
#endif

#ifndef __ATTRIBUTE_IMPURE_DATA__
#define __ATTRIBUTE_IMPURE_DATA__
#endif

/* Redeclare these symbols locally as weak so that the file containing
   their definitions (along with a lot of other stuff) isn't sucked in
   unless they are actually used by other compilation units.  This is
   important to reduce image size for targets with very small amounts
   of memory.  */
#ifdef _REENT_SMALL
extern const struct __sFILE_fake __sf_fake_stdin _ATTRIBUTE ((weak));
extern const struct __sFILE_fake __sf_fake_stdout _ATTRIBUTE ((weak));
extern const struct __sFILE_fake __sf_fake_stderr _ATTRIBUTE ((weak));
#endif

static struct _reent __ATTRIBUTE_IMPURE_DATA__ impure_data = _REENT_INIT (impure_data);
#ifdef __CYGWIN__
extern struct _reent reent_data __attribute__ ((alias("impure_data")));
#endif
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _impure_ptr = &impure_data;
struct _reent *const __ATTRIBUTE_IMPURE_PTR__ _global_impure_ptr = &impure_data;
