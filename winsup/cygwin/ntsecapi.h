#ifndef _CYGWIN_NTSECAPI_H
#define _CYGWIN_NTSECAPI_H

/* There's a bug in ntsecapi.h (Mingw as well as MSFT).  SystemFunction036
   is, in fact, a WINAPI function, but it's not defined as such.  Therefore
   we have to do it correctly here in the ntsecapi.h wrapper. */

#define SystemFunction036	__nonexistant_SystemFunction036__

#include_next <ntsecapi.h>

#undef SystemFunction036

#define RtlGenRandom SystemFunction036

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

BOOLEAN WINAPI RtlGenRandom (PVOID, ULONG);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CYGWIN_NTSECAPI_H */
