/*
FUNCTION
<<time>>---get current calendar time (as single number)

INDEX
	time

ANSI_SYNOPSIS
	#include <time.h>
	time_t time(time_t *<[t]>);

TRAD_SYNOPSIS
	#include <time.h>
	time_t time(<[t]>)
	time_t *<[t]>;

DESCRIPTION
<<time>> looks up the best available representation of the current
time and returns it, encoded as a <<time_t>>.  It stores the same
value at <[t]> unless the argument is <<NULL>>.

RETURNS
A <<-1>> result means the current time is not available; otherwise the
result represents the current time.

PORTABILITY
ANSI C requires <<time>>.
This function is a wrapper for <<gettimeofday>>; this pollutes the
namespace, and should be dealt with in the future.
*/

#include <_ansi.h>
#include <sys/types.h>
#include <sys/time.h>

time_t
_DEFUN(time,(t),
	time_t *t)
{
  struct timeval now;

  if (gettimeofday(&now, (struct timezone *)0) >= 0)
  {
    if (t) 
    {
      *t = now.tv_sec;
    }
    
    return now.tv_sec;

  }
  return -1 ;	
}
