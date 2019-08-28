/* envlock.h -- header file for env routines.  */

#ifndef _INCLUDE_ENVLOCK_H_
#define _INCLUDE_ENVLOCK_H_

#include <_ansi.h>

#define ENV_LOCK __env_lock()
#define ENV_UNLOCK __env_unlock()

void __env_lock (void);
void __env_unlock (void);

#endif /* _INCLUDE_ENVLOCK_H_ */
