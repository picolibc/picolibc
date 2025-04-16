/* Copyright (c) 2016 Joel Sherrill <joel@rtems.org> */
/*
FUNCTION
<<random>>, <<srandom>>---pseudo-random numbers

INDEX
	random
INDEX
	srandom

SYNOPSIS
	#define _XOPEN_SOURCE 500
	#include <stdlib.h>
	long int random(void);
	void srandom(unsigned int <[seed]>);



DESCRIPTION
<<random>> returns a different integer each time it is called; each
integer is chosen by an algorithm designed to be unpredictable, so
that you can use <<random>> when you require a random number.
The algorithm depends on a static variable called the ``random seed'';
starting with a given value of the random seed always produces the
same sequence of numbers in successive calls to <<random>>.

You can set the random seed using <<srandom>>; it does nothing beyond
storing its argument in the static variable used by <<rand>>.  You can
exploit this to make the pseudo-random sequence less predictable, if
you wish, by using some other unpredictable value (often the least
significant parts of a time-varying value) as the random seed before
beginning a sequence of calls to <<rand>>; or, if you wish to ensure
(for example, while debugging) that successive runs of your program
use the same ``random'' numbers, you can use <<srandom>> to set the same
random seed at the outset.

RETURNS
<<random>> returns the next pseudo-random integer in sequence; it is a
number between <<0>> and <<2**31 - 1>> (inclusive).

<<srandom>> does not return a result.

NOTES
<<random>> and <<srandom>> are unsafe for multi-threaded applications.

_XOPEN_SOURCE may be any value >= 500.

PORTABILITY
<<random>> is required by XSI. This implementation uses the same
algorithm as <<rand>>.

<<random>> requires no supporting OS subroutines.
*/

#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdint.h>

__THREAD_LOCAL uint64_t _rand_next = 1;

long int
random (void)
{
  /* This multiplier was obtained from Knuth, D.E., "The Art of
     Computer Programming," Vol 2, Seminumerical Algorithms, Third
     Edition, Addison-Wesley, 1998, p. 106 (line 26) & p. 108 */
  _rand_next = _rand_next * 6364136223846793005ULL + 1;
  return (long int)((_rand_next >> 32) & 0x7fffffffL);
}
