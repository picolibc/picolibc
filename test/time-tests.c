/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 R. Diez
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define TIME_TM_YEAR_BASE 1900


static void
init_struct_tm ( struct tm * stm )
{
  if ( sizeof( struct tm ) != 9 * sizeof( int ) )
  {
    puts("Error: struct tm has changed, so these tests probably need adjusting.");
    exit(1);
  }

  stm->tm_sec   = 0;
  stm->tm_min   = 0;
  stm->tm_hour  = 0;
  stm->tm_mday  = 0;
  stm->tm_mon   = 0;
  stm->tm_year  = 0;
  stm->tm_wday  = 0;
  stm->tm_yday  = 0;
  stm->tm_isdst = 0;
}


int
main(void)
{
  // Time zone ART3 is America/Buenos_Aires.
  // There is no daylight saving time (aka sommer time).

  if ( 0 != setenv( "TZ", "ART3", 1 ) )
  {
    puts("Error calling setenv().");
    exit(1);
  }

  tzset();


  struct tm dtForTimegm1;
  init_struct_tm( &dtForTimegm1 );
  // This is some arbitrary point in time.
  dtForTimegm1.tm_mday  = 3;  // 3rd
  dtForTimegm1.tm_mon   = 1;  // of February
  dtForTimegm1.tm_year  = 1970 - TIME_TM_YEAR_BASE;

  // Make a copy, because mktime() and friends modify the fields of the tm structure passed as an argument.
  struct tm dtForMktime2 = dtForTimegm1;

  // timegm() should be not affected by the time zone.

  time_t t1 = timegm( &dtForTimegm1 );

  if ( t1 != 2851200 ||
       dtForTimegm1.tm_wday != 2 )  // 2 means Tuesday.
  {
    puts("Test t1 failed.");
    exit(1);
  }

  // mktime() is affected by the time zone. Check that the offset is the expected one.

  time_t t2 = mktime( &dtForMktime2 );

  if ( t2 != t1 + 3 * 60 * 60 ||  // The chosen time zone has a positive 3-hour difference.
       dtForMktime2.tm_wday != 2 )
  {
    puts("Test t2 failed.");
    exit(1);
  }


  // The European Central Time goes the other direction (negative)
  // and has a daylight saving time (aka sommer time).

  if ( 0 != setenv( "TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1 ) )
  {
    puts("Error calling setenv().");
    exit(1);
  }

  tzset();


  struct tm dtForTimegm3;
  init_struct_tm( &dtForTimegm3 );

  // This is some arbitrary point in time with daylight saving time.
  dtForTimegm3.tm_mday  = 1;   // 1st
  dtForTimegm3.tm_mon   = 4;   // of May
  dtForTimegm3.tm_year  = 2021 - TIME_TM_YEAR_BASE;
  dtForTimegm3.tm_isdst = 1;  // Daylight saving time is in effect. That only affects local time.

  struct tm dtForMktime4 = dtForTimegm3;

  time_t t3 = timegm( &dtForTimegm3 );

  if ( t3 != 1619827200           ||
       dtForTimegm3.tm_wday  != 6 ||  // 6 means Saturday.
       dtForTimegm3.tm_isdst != 0  )  // timegm() should reset the daylight saving time flag.
  {
    puts("Test t3 failed.");
    exit(1);
  }

  time_t t4 = mktime( &dtForMktime4 );

  // The offset is -2 hours: 1 hour because of the time zone, and 1 hour because of the sommer time.
  if ( t4 != t3 - 2 * 60 * 60     ||
       dtForMktime4.tm_wday  != 6 ||  // 6 means Saturday.
       dtForMktime4.tm_isdst != 1  )  // mktime() should leave the daylight saving time flag set.
  {
    puts("Test t4 failed.");
    exit(1);
  }


  // Test the first non-negative time_t value of 0,
  // which is the "UNIX Epoch time" of 1st Januar 1970 00:00:00.

  struct tm dtForTimegm5;
  init_struct_tm( &dtForTimegm5 );

  dtForTimegm5.tm_mday  = 1;   // 1st day of the month.
  dtForTimegm5.tm_year  = 1970 - TIME_TM_YEAR_BASE;
  dtForTimegm5.tm_isdst = -123;  // A negative value makes mktime() calculate this flag,
                                 // but timegm() should unconditionally reset it.

  time_t t5 = timegm( &dtForTimegm5 );

  if ( t5 != 0                    ||
       dtForTimegm5.tm_wday  != 4 ||  // 4 means Thursday.
       dtForTimegm5.tm_isdst != 0  )
  {
    puts("Test t5 failed.");
    exit(1);
  }


  // Test the last time_t value of 0x7FFFFFFF before the 32-bit signed integer overflow,
  // aka "year 2038 problem". That corresponds to 2038-01-19 03:14:07.

  struct tm dtForTimegm6;
  init_struct_tm( &dtForTimegm6 );

  dtForTimegm6.tm_sec  =  7;
  dtForTimegm6.tm_min  = 14;
  dtForTimegm6.tm_hour =  3,
  dtForTimegm6.tm_mon   = 0;  // January.
  dtForTimegm6.tm_mday  = 19;
  dtForTimegm6.tm_year  = 2038 - TIME_TM_YEAR_BASE;
  dtForTimegm6.tm_isdst = 123;  // Some value that should be reset.

  struct tm dtForTimegm7 = dtForTimegm6;  // Reuse the date for the next test.

  time_t t6 = timegm( &dtForTimegm6 );

  if ( t6 != 0x7FFFFFFF            ||
       dtForTimegm6.tm_wday  != 2  ||  // 2 means Tuesday.
       dtForTimegm6.tm_yday  != 18 ||
       dtForTimegm6.tm_isdst != 0   )
  {
    puts("Test t6 failed.");
    exit(1);
  }


  // Test the next time_t value of 0x80000000 right after
  // the 32-bit signed integer overflow, aka "year 2038 problem".

  dtForTimegm7.tm_sec++;

  time_t t7 = timegm( &dtForTimegm7 );

  if ( t7 != 0x80000000            ||
       dtForTimegm7.tm_wday  != 2  ||  // 2 means Tuesday.
       dtForTimegm7.tm_yday  != 18 ||
       dtForTimegm7.tm_isdst != 0   )
  {
    puts("Test t7 failed.");
    exit(1);
  }


  // Test the 29th of February in a leap year far in the future.

  struct tm dtForTimegm8;
  init_struct_tm( &dtForTimegm8 );

  dtForTimegm8.tm_mon   = 1;  // February.
  dtForTimegm8.tm_mday  = 29;
  dtForTimegm8.tm_year  = 2104 - TIME_TM_YEAR_BASE;

  time_t t8 = timegm( &dtForTimegm8 );

  if ( t8 != 4233686400           ||  // That is much higher than INT32_MAX.
       dtForTimegm8.tm_wday != 5  ||  // 5 means Friday.
       dtForTimegm8.tm_yday != 59  )
  {
    puts("Test t8 failed.");
    exit(1);
  }


  return 0;
}
