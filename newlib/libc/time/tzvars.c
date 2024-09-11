/* Copyright 2005 Tom Walsh <tom@openhardware.net> */
#define _GNU_SOURCE
#include <time.h>

/* Global timezone variables.  */

/* Default timezone to GMT */
char *tzname[2] = {"GMT", "GMT"};
int _daylight = 0;
long _timezone = 0;


