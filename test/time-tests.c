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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <../newlib/libc/time/local.h>  // For YEAR_BASE.


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
  dtForTimegm1.tm_year  = 1970 - YEAR_BASE;

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
  dtForTimegm3.tm_year  = 2021 - YEAR_BASE;
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


  return 0;
}
