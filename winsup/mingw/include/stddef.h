/*
 * This is just an RC_INVOKED guard for the real stddef.h
 * included in gcc system dir.  One day we will delete this file.
 */
#ifndef RC_INVOKED
#include_next<stddef.h>
#endif
