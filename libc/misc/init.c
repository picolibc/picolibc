/*
 * Copyright (C) 2004 CodeSourcery, LLC
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

/* Iterate over all the init routines.  */
void
__libc_init_array(void)
{
    void (**fn)(void);
    void (**fn_end)(void);

#ifdef __INIT_FINI_FUNCS
    fn = __preinit_array_start;
    fn_end = __preinit_array_end;
    while (fn != fn_end)
        (*fn++)();

    if (_init)
        _init();

    fn = __init_array_start;
    fn_end = __init_array_end;
    while (fn != fn_end)
        (*fn++)();
#else
    /*
     * The init array immediately follows the preinit array,
     * so we can just run both in one loop
     */
    fn = __bothinit_array_start;
    fn_end = __bothinit_array_end;
    while (fn != fn_end)
        (*fn++)();
#endif
}
#endif
