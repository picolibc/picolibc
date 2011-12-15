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


#include <sys/types.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <asm-leon/jiffies.h>
#include <asm-leon/param.h>

clock_t (*clock_custom) (void) = 0;

extern int *rtc;

clock_t
times (struct tms *buffer)
{
  clock_t v;
  if (clock_custom)
    v = clock_custom ();
  else
    v = -*rtc;

  buffer->tms_utime = v;	//-*rtc;
  buffer->tms_utime /= (CLOCK_TICK_RATE / HZ);
  buffer->tms_stime = 0;
  buffer->tms_cutime = 0;
  buffer->tms_cstime = 0;
}

clock_t
clock (void)
{
  if (clock_custom)
    return clock_custom ();
  return (-*rtc);
}
