/*
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/*
** Include appropriate header file for platform.
** Copyright (C) 2008 Analog Devices, Inc.
*/

#ifndef __ADI_PLATFORM_H
#define __ADI_PLATFORM_H

#ifdef __ASSEMBLY__

#if defined (__ADSPBF531__)
#include <cdefBF531.h>
#elif defined (__ADSPBF532__)
#include <cdefBF532.h>
#elif defined (__ADSPBF533__)
#include <cdefBF533.h>
#elif defined (__ADSPBF534__)
#include <cdefBF534.h>
#elif defined (__ADSPBF535__)
#include <cdefBF535.h>
#elif defined (__ADSPBF536__)
#include <cdefBF536.h>
#elif defined (__ADSPBF537__)
#include <cdefBF537.h>
#elif defined (__ADSPBF538__)
#include <cdefBF538.h>
#elif defined (__ADSPBF539__)
#include <cdefBF539.h>
#elif defined (__ADSPBF561__)
#include <cdefBF561.h>
#elif defined (__AD6531__)
#include <cdefAD6531.h>
#elif defined (__AD6532__)
#include <cdefAD6532.h>
#elif defined (__AD6723__)
#include <cdefAD6723.h>
#elif defined (__AD6900__)
#include <cdefAD6900.h>
#elif defined (__AD6901__)
#include <cdefAD6901.h>
#elif defined (__AD6902__)
#include <cdefAD6902.h>
#elif defined (__AD6903__)
#include <cdefAD6903.h>
#elif defined (__AD6904__)
#include <cdefAD6904.h>
#elif defined (__ADSPBF522__)
#include <cdefBF522.h>
#elif defined (__ADSPBF525__)
#include <cdefBF525.h>
#elif defined (__ADSPBF527__)
#include <cdefBF527.h>
#elif defined (__ADSPBF542__) || defined (__ADSPBF541__)
#include <cdefBF542.h>
#elif defined (__ADSPBF544__)
#include <cdefBF544.h>
#elif defined (__ADSPBF547__)
#include <cdefBF547.h>
#elif defined (__ADSPBF548__)
#include <cdefBF548.h>
#elif defined (__ADSPBF549__)
#include <cdefBF549.h>
#else
#error Processor Type Not Supported
#endif


#else

#if defined (__ADSPBF531__)
#include <defBF531.h>
#elif defined (__ADSPBF532__)
#include <defBF532.h>
#elif defined (__ADSPBF533__)
#include <defBF533.h>
#elif defined (__ADSPBF534__)
#include <defBF534.h>
#elif defined (__ADSPBF535__)
#include <defBF535.h>
#elif defined (__ADSPBF536__)
#include <defBF536.h>
#elif defined (__ADSPBF537__)
#include <defBF537.h>
#elif defined (__ADSPBF538__)
#include <defBF538.h>
#elif defined (__ADSPBF539__)
#include <defBF539.h>
#elif defined (__ADSPBF561__)
#include <defBF561.h>
#elif defined (__AD6531__)
#include <defAD6531.h>
#elif defined (__AD6532__)
#include <defAD6532.h>
#elif defined (__AD6723__)
#include <defAD6723.h>
#elif defined (__AD6900__)
#include <defAD6900.h>
#elif defined (__AD6901__)
#include <defAD6901.h>
#elif defined (__AD6902__)
#include <defAD6902.h>
#elif defined (__AD6903__)
#include <defAD6903.h>
#elif defined (__AD6904__)
#include <defAD6904.h>
#elif defined (__ADSPBF522__)
#include <defBF522.h>
#elif defined (__ADSPBF525__)
#include <defBF525.h>
#elif defined (__ADSPBF527__)
#include <defBF527.h>
#elif defined (__ADSPBF542__) || defined (__ADSPBF541__)
#include <defBF542.h>
#elif defined (__ADSPBF544__)
#include <defBF544.h>
#elif defined (__ADSPBF547__)
#include <defBF547.h>
#elif defined (__ADSPBF548__)
#include <defBF548.h>
#elif defined (__ADSPBF549__)
#include <defBF549.h>

#else
#error Processor Type Not Supported
#endif

#endif /* __ASSEMBLY__ */

#endif /* __INC_BLACKFIN__ */

