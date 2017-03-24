/* bsd_log.h: Helps integrating BSD kernel code

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */
#ifndef _BSD_LOG_H
#define _BSD_LOG_H

#include <sys/types.h>
#include <sys/syslog.h>

extern int32_t log_level;
extern tun_bool_t log_debug;
extern tun_bool_t log_syslog;
extern tun_bool_t log_stderr;

void loginit (tun_bool_t, tun_bool_t);
void _vlog (const char *, int, int, const char *, va_list);
void _log (const char *, int, int, const char *, ...);

#define vlog(l,f,a) _vlog(NULL,0,(l),(f),(a))
#define log(l,f,...) _log(NULL,0,(l),(f),##__VA_ARGS__)

#define _vdebug(F,L,f,a) \
			do { if (log_debug == TUN_TRUE) \
			  _vlog((F),(L),LOG_DEBUG,(f),(a)) \
			} while (0)

#define _debug(F,L,f,...) \
			do { if (log_debug == TUN_TRUE) \
			  _log((F),(L),LOG_DEBUG,(f),##__VA_ARGS__); \
			} while (0)

#define vdebug(f,a)	_vdebug(__FILE__,__LINE__,(f),(a))
#define debug(f,...)	_debug(__FILE__,__LINE__,(f),##__VA_ARGS__)

void _vpanic (const char *, int, const char *, va_list)
	__attribute__ ((noreturn));
void _panic (const char *, int, const char *, ...) __attribute__ ((noreturn));
#define vpanic(f,a) _vpanic(__FILE__,__LINE__,(f),(a))
#define panic(f,...) _panic(__FILE__,__LINE__,(f),##__VA_ARGS__)

#endif /* _BSD_LOG_H */
