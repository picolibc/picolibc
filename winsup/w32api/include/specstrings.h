#ifndef _SPECSTRINGS_H
#define _SPECSTRINGS_H
#if __GNUC__ >=3
#pragma GCC system_header
#endif

/* __in and __out currently conflict with libstdc++, use with caution */

#define __in
#define __inout
#define __in_opt
#define __in_bcount(x)
#define __in_ecount(x)
#define __out
#define __out_ecount_part(x)
#define __out_ecount_part(x,y)
#define __struct_bcount(x)
#define __field_ecount_opt(x)
#define __out_bcount_opt(x)

#endif

