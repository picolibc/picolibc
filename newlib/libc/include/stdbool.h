#ifndef _STDBOOL_H_
#define _STDBOOL_H_

#ifndef __cplusplus

#undef bool
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define bool _Bool
#else
#define bool unsigned char
#endif

#undef false
#define false 0
#undef true
#define true 1

#define __bool_true_false_are_defined 1

#endif /* !__cplusplus */

#endif /* _STDBOOL_H_ */
