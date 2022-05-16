#include <reent.h>

#ifndef _REENT_THREAD_LOCAL

/* Redeclare these symbols locally as weak so that the file containing
   their definitions (along with a lot of other stuff) isn't sucked in
   unless they are actually used by other compilation units.  This is
   important to reduce image size for targets with very small amounts
   of memory.  */
#ifdef _REENT_SMALL
extern __FILE __sf[3] _ATTRIBUTE ((weak));
#endif

struct _reent __ATTRIBUTE_IMPURE_DATA__ _impure_data = _REENT_INIT (_impure_data);
#ifdef __CYGWIN__
extern struct _reent reent_data __attribute__ ((alias("_impure_data")));
#endif
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _impure_ptr = &_impure_data;

#endif /* _REENT_THREAD_LOCAL */
