/*
 * Definitions of some internal stuff for exception handling, including
 * a version of the all-important EXCEPTION_REGISTRATION_RECORD.
 */

#ifndef _EXUTIL_H_
#define _EXUTIL_H_

#include <windows.h>
#include <excpt.h>

#ifdef	__cplusplus
extern "C" {
#endif

void WalkExceptionHandlers ();
void DumpExceptionRecord (struct _EXCEPTION_RECORD* pExRec);

#ifdef	__cplusplus
}
#endif

#endif
