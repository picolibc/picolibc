/* clock.c
 * Support file for nvptx in newlib.
 */
#include <time.h>

clock_t
clock ()
{
  unsigned long long now;
#if __PTX_SM__ >= 310
  asm volatile("mov.u64 %0, %%globaltimer;" : "=r"(now));
  return now/((1000000000ull)/CLOCKS_PER_SEC);
#else
  asm volatile("mov.u64 %0, %%clock64;" : "=r"(now));
  // Assume a GPU base clock frequency of 1250MHz.
  return now/((1250000000ull)/CLOCKS_PER_SEC);
#endif
}
