/* Copyright 2005 Tom Walsh <tom@openhardware.net> */
#include "local.h"

/* Global timezone variables.  */

/* Default timezone to GMT */
char         __tzname_std[TZNAME_MAX + 2] = "GMT";
char         __tzname_dst[TZNAME_MAX + 2] = "GMT";
char * const tzname[2] = { __tzname_std, __tzname_dst };
int          daylight;
long         timezone;

/* Shared timezone information for libc/time functions.  */
tzinfo_t     __tzinfo;
