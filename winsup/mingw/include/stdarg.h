/*
 * This is just an RC_INVOKED guard for the real stdarg.h
 * included in gcc system dir.  One day we will delete this file.
 */

#ifndef RC_INVOKED 
#include_next<stdarg.h>
#endif /* RC_INVOKED */ 
