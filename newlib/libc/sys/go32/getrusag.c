/* This is file GETRUSAG.C */
/*
** Copyright (C) 1991 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <sys/time.h>
#include <sys/resource.h>

static struct timeval old_time = {0,0};

int getrusage(int who, struct rusage *rusage)
{
  struct timeval now;
  bzero(rusage, sizeof(struct rusage));
  if (old_time.tv_sec == 0)
    gettimeofday(&old_time, 0);
  gettimeofday(&now, 0);
  rusage->ru_utime.tv_usec = now.tv_usec - old_time.tv_usec;
  rusage->ru_utime.tv_sec = now.tv_sec - old_time.tv_sec;
  if (rusage->ru_utime.tv_usec < 0)
  {
    rusage->ru_utime.tv_usec += 1000000;
    rusage->ru_utime.tv_sec -= 1;
  }
  return 0;
}
