/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef __LEONBARE_KERNEL_DEBUG_H__
#define __LEONBARE_KERNEL_DEBUG_H__

#include <asm-leon/leondbg.h>

/*
  #define LBDEBUG_DO_DEBUG
  #define LBDEBUG_DO_ASSERT
*/

#define LBDEBUG_ALWAYS_NR  (1<<0)
#define LBDEBUG_ASSERT_NR  (1<<1)
#define LBDEBUG_FNCALL_NR  (1<<2)
#define LBDEBUG_FNEXIT_NR  (1<<3)
#define LBDEBUG_SCHED_NR   (1<<4)
#define LBDEBUG_QUEUE_NR   (1<<5)
#define LBDEBUG_THREAD_NR  (1<<6)

#define LBDEBUG_PRINTF dbgleon_printf	/*leonbare_debug_printf */

#ifdef LBDEBUG_DO_DEBUG
#ifndef __ASSEMBLER__
extern int leonbare_debug;
#endif
# define PDEBUG_FLAGS_CHECK(c) ((c)&leonbare_debug)
# define PDEBUG_FLAGS_SET(c) leonbare_debug |= c
#else
# define PDEBUG_FLAGS_CHECK(c) 0
# define PDEBUG_FLAGS_SET(c)
#endif

#ifdef LBDEBUG_DO_DEBUG
# define LBDEBUG(x, fmt, args...) do { if (PDEBUG_FLAGS_CHECK(x)) { LBDEBUG_PRINTF(fmt,args); } } while(0)
#else
# define LBDEBUG(x, fmt, args...)
#endif

#ifdef LBDEBUG_DO_ASSERT
# define LBPASSERT(x, fmt, args...) if (!(x)) { LBDEBUG_PRINTF(fmt,args); while(1); }
#else
# define LBPASSERT(x, fmt, args...)
#endif

#ifndef LBDEBUG___FUNCTION__
#define LBDEBUG___FUNCTION__ __FUNCTION__
#endif

#ifndef LBDEBUG___FUNCTION_WIDTH__
#define LBDEBUG___FUNCTION_WIDTH__ "28"
#endif

#ifdef LBDEBUG_DO_FILE
#ifndef LBDEBUG___FILE__
#define LBDEBUG___FILE__ __FILE__
#endif
#ifndef LBDEBUG___FILE_WIDTH__
#define LBDEBUG___FILE_WIDTH__ "28"
#endif
#define LBDEBUG___FILE_APPEND     ,__FILE__
#define LBDEBUG___FILE_FMT_APPEND ":%" LBDEBUG___FILE_WIDTH__ "s"
#else
#define LBDEBUG___FILE_APPEND
#define LBDEBUG___FILE_FMT_APPEND
#endif

#ifdef LBDEBUG_DO_DEBUG
# define LBDEBUG_HEADER(code)                                                   \
  if (PDEBUG_FLAGS_CHECK(code)) {                                               \
    register unsigned int _GETSP asm("sp");                                     \
    LBDEBUG_PRINTF("[sp:%08x self(%08x):", _GETSP, LEONBARE_KR_CURRENT);        \
    LBDEBUG_PRINTF("%10s",LEONBARE_TH_NAME_DBG(LEONBARE_KR_CURRENT));           \
    LBDEBUG_PRINTF(" %03d @ %" LBDEBUG___FUNCTION_WIDTH__ "s()" LBDEBUG___FILE_FMT_APPEND "]:" , __LINE__,LBDEBUG___FUNCTION__ LBDEBUG___FILE_APPEND); \
  }

# define LBDEBUG_HEADER_PRINTF(code,fmt,args...)                                \
  if (PDEBUG_FLAGS_CHECK(code)) {                                               \
    LBDEBUG_HEADER(code);                                                       \
    LBDEBUG_PRINTF(fmt,args);                                                   \
  }

# define LBDEBUG_CODE_PRINTF(code,fmt,args...)                                  \
  if (PDEBUG_FLAGS_CHECK(code)) {                                               \
    LBDEBUG_PRINTF(fmt,args);                                                   \
  }
#else
# define LBDEBUG_HEADER(code)
# define LBDEBUG_HEADER_PRINTF(code,fmt,args...)
# define LBDEBUG_CODE_PRINTF(code,fmt,args...)
#endif

#define LBDEBUG_FNCALL LBDEBUG_HEADER_PRINTF(LBDEBUG_FNCALL_NR,"enter\n",0)
#define LBDEBUG_FNEXIT LBDEBUG_HEADER_PRINTF(LBDEBUG_FNEXIT_NR,"exit\n",0)

#ifndef __ASSEMBLER__

int leonbare_debug_printf (const char *fmt, ...);

#endif

#endif
