/* wow64.h

   Copyright 2011, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __x86_64__
/* WOW64 only plays a role in the 32 bit version.  Don't use any of this
   in the 64 bit version. */

extern bool NO_COPY wow64_needs_stack_adjustment;

extern bool wow64_test_for_64bit_parent ();
extern PVOID wow64_revert_to_original_stack (PVOID &allocationbase);
extern void wow64_respawn_process () __attribute__ ((noreturn));

#endif /* !__x86_64__ */
