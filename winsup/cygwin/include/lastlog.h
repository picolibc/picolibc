#ifndef _LASTLOG_H
#define _LASTLOG_H

#include <utmp.h>

struct lastlog {
	long ll_time;
	char ll_line[UT_LINESIZE];
	char ll_host[UT_HOSTSIZE];
};

#endif
