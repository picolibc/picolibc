/*
 * Copyright (C) 2010 CodeSourcery, Inc.
 *
 * Permission to use, copy, modify, and distribute this file
 * for any purpose is hereby granted without fee, provided that
 * the above copyright notice and this notice appears in all
 * copies.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Handle ELF .{pre_init,init,fini}_array sections.  */
#include <sys/types.h>
#include <sys/_initfini.h>

#ifdef __INIT_FINI_ARRAY

#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wanalyzer-out-of-bounds"
#endif

/* Run all the cleanup routines.  */
void
__libc_fini_array (void)
{
    void (**fn)(void);
    void (**fn_start)(void);

    fn = __fini_array_end;
    fn_start = __fini_array_start;
    while (fn != fn_start)
        (*--fn)();

#ifdef __INIT_FINI_FUNCS
    if (_fini)
        _fini ();
#endif
}
#endif
