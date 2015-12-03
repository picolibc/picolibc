/* wow64.h

   Copyright 2011, 2012, 2015 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

extern bool NO_COPY wow64_needs_stack_adjustment;
extern bool wow64_test_for_64bit_parent ();
extern void wow64_respawn_process () __attribute__ ((noreturn));

#ifndef __x86_64__

extern PVOID wow64_revert_to_original_stack (PVOID &allocationbase);

#endif /* !__x86_64__ */
