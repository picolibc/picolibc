#include <reent.h>

/* Note that there is a copy of this in sys/reent.h.  */
#ifndef __ATTRIBUTE_IMPURE_PTR__
#define __ATTRIBUTE_IMPURE_PTR__
#endif

#ifndef __ATTRIBUTE_IMPURE_DATA__
#define __ATTRIBUTE_IMPURE_DATA__
#endif

#ifndef __DYNAMIC_REENT__
struct _reent __ATTRIBUTE_IMPURE_DATA__ _impure_data = _REENT_INIT(_impure_data);
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _impure_ptr = &_impure_data;
#endif
struct _reent *const __ATTRIBUTE_IMPURE_PTR__ _global_impure_ptr = &_impure_data;
