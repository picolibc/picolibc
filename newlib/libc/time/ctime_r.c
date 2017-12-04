/*
 * ctime_r.c
 */

#include <time.h>

char *
ctime_r (const time_t * tim_p,
        char * result)

{
  struct tm tm;
  return asctime_r (localtime_r (tim_p, &tm), result);
}
