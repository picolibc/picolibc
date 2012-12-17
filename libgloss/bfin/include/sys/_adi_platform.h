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
** Copyright (C) 2004-2009 Analog Devices Inc. All Rights Reserved.
*/

#ifndef __ADI_PLATFORM_H
#define __ADI_PLATFORM_H

#ifndef __ASSEMBLER__

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
#elif defined (__AD6905__)
#include <cdefAD6905.h>
#elif defined (__MT6906__)
#include <cdefMT6906.h>
#elif defined (__ADSPBF504__)
#include <cdefBF504.h>
#elif defined (__ADSPBF504F__)
#include <cdefBF504F.h>
#elif defined (__ADSPBF506__) || defined (__ADSPBF506F__)
#include <cdefBF506F.h>
#elif defined (__ADSPBF512__)
#include <cdefBF512.h>
#elif defined (__ADSPBF514__)
#include <cdefBF514.h>
#elif defined (__ADSPBF516__)
#include <cdefBF516.h>
#elif defined (__ADSPBF518__)
#include <cdefBF518.h>
#elif defined (__ADSPBF522__)
#include <cdefBF522.h>
#elif defined (__ADSPBF523__)
#include <cdefBF523.h>
#elif defined (__ADSPBF524__)
#include <cdefBF524.h>
#elif defined (__ADSPBF525__)
#include <cdefBF525.h>
#elif defined (__ADSPBF526__)
#include <cdefBF526.h>
#elif defined (__ADSPBF527__)
#include <cdefBF527.h>
#elif defined (__ADSPBF542__)
#include <cdefBF542.h>
#elif defined (__ADSPBF542M__)
#include <cdefBF542M.h>
#elif defined (__ADSPBF544__)
#include <cdefBF544.h>
#elif defined (__ADSPBF544M__)
#include <cdefBF544M.h>
#elif defined (__ADSPBF547__)
#include <cdefBF547.h>
#elif defined (__ADSPBF547M__)
#include <cdefBF547M.h>
#elif defined (__ADSPBF548__)
#include <cdefBF548.h>
#elif defined (__ADSPBF548M__)
#include <cdefBF548M.h>
#elif defined (__ADSPBF549__)
#include <cdefBF549.h>
#elif defined (__ADSPBF549M__)
#include <cdefBF549M.h>
#elif defined (__ADSPBF592A__)
#include <cdefBF592-A.h>
#elif defined (__ADSPBF606__)
#include <cdefBF606.h>
#elif defined (__ADSPBF607__)
#include <cdefBF607.h>
#elif defined (__ADSPBF608__)
#include <cdefBF608.h>
#elif defined (__ADSPBF609__)
#include <cdefBF609.h>
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
#elif defined (__AD6905__)
#include <defAD6905.h>
#elif defined (__MT6906__)
#include <defMT6906.h>
#elif defined (__ADSPBF504__)
#include <defBF504.h>
#elif defined (__ADSPBF504F__)
#include <defBF504F.h>
#elif defined (__ADSPBF506__) || defined (__ADSPBF506F__)
#include <defBF506F.h>
#elif defined (__ADSPBF512__)
#include <defBF512.h>
#elif defined (__ADSPBF514__)
#include <defBF514.h>
#elif defined (__ADSPBF516__)
#include <defBF516.h>
#elif defined (__ADSPBF518__)
#include <defBF518.h>
#elif defined (__ADSPBF522__)
#include <defBF522.h>
#elif defined (__ADSPBF523__)
#include <defBF523.h>
#elif defined (__ADSPBF524__)
#include <defBF524.h>
#elif defined (__ADSPBF525__)
#include <defBF525.h>
#elif defined (__ADSPBF526__)
#include <defBF526.h>
#elif defined (__ADSPBF527__)
#include <defBF527.h>
#elif defined (__ADSPBF542__)
#include <defBF542.h>
#elif defined (__ADSPBF542M__)
#include <defBF542M.h>
#elif defined (__ADSPBF544__)
#include <defBF544.h>
#elif defined (__ADSPBF544M__)
#include <defBF544M.h>
#elif defined (__ADSPBF547__)
#include <defBF547.h>
#elif defined (__ADSPBF547M__)
#include <defBF547M.h>
#elif defined (__ADSPBF548__)
#include <defBF548.h>
#elif defined (__ADSPBF548M__)
#include <defBF548M.h>
#elif defined (__ADSPBF549__)
#include <defBF549.h>
#elif defined (__ADSPBF549M__)
#include <defBF549M.h>
#elif defined (__ADSPBF592A__)
#include <defBF592-A.h>
#elif defined (__ADSPBF606__)
#include <defBF606.h>
#elif defined (__ADSPBF607__)
#include <defBF607.h>
#elif defined (__ADSPBF608__)
#include <defBF608.h>
#elif defined (__ADSPBF609__)
#include <defBF609.h>

#else
#error Processor Type Not Supported
#endif

#endif /* __ASSEMBLER__ */

#endif /* __INC_BLACKFIN__ */

