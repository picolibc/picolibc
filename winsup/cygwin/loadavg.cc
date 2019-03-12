/* loadavg.cc: load average support.

  This file is part of Cygwin.

  This software is a copyrighted work licensed under the terms of the
  Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
  details. */

/*
  Emulate load average

  There's a fair amount of approximation done here, so don't try to use this to
  actually measure anything, but it should be good enough for programs to
  throttle their activity based on load.

  A global load average estimate is maintained in shared memory.  Access to that
  is guarded by a mutex.  This estimate is only updated at most every 5 seconds.

  We attempt to count running and runnable processes, but unlike linux we don't
  count processes in uninterruptible sleep (blocked on I/O).

  The number of running processes is estimated as (NumberOfProcessors) * (%
  Processor Time).  The number of runnable processes is estimated as
  ProcessorQueueLength.

  Note that PDH will only return data for '% Processor Time' afer the second
  call to PdhCollectQueryData(), as it's computed over an interval, so the first
  attempt to estimate load will fail and 0.0 will be returned.

  We also assume that '% Processor Time' averaged over the interval since the
  last time getloadavg() was called is a good approximation of the instantaneous
  '% Processor Time'.
*/

#include "winsup.h"
#include "shared_info.h"
#include "loadavg.h"

#include <math.h>
#include <time.h>
#include <sys/strace.h>
#include <pdh.h>

static PDH_HQUERY query;
static PDH_HCOUNTER counter1;
static PDH_HCOUNTER counter2;
static HANDLE mutex;

static bool load_init (void)
{
  static NO_COPY bool tried = false;
  static NO_COPY bool initialized = false;

  if (!tried) {
    tried = true;

    PDH_STATUS status;

    status = PdhOpenQueryW (NULL, 0, &query);
    if (status != STATUS_SUCCESS)
      {
	debug_printf ("PdhOpenQueryW, status %y", status);
	return false;
      }
    status = PdhAddEnglishCounterW (query,
				    L"\\Processor(_Total)\\% Processor Time",
				    0, &counter1);
    if (status != STATUS_SUCCESS)
      {
	debug_printf ("PdhAddEnglishCounterW(time), status %y", status);
	return false;
      }
    status = PdhAddEnglishCounterW (query,
				    L"\\System\\Processor Queue Length",
				    0, &counter2);

    if (status != STATUS_SUCCESS)
      {
	debug_printf ("PdhAddEnglishCounterW(queue length), status %y", status);
	return false;
      }

    mutex = CreateMutex(&sec_all_nih, FALSE, "cyg.loadavg.mutex");
    if (!mutex) {
      debug_printf("opening loadavg mutexfailed\n");
      return false;
    }

    initialized = true;
  }

  return initialized;
}

/* estimate the current load */
static bool get_load (double *load)
{
  *load = 0.0;

  PDH_STATUS ret = PdhCollectQueryData (query);
  if (ret != ERROR_SUCCESS)
    return false;

  /* Estimate the number of running processes as (NumberOfProcessors) * (%
     Processor Time) */
  PDH_FMT_COUNTERVALUE fmtvalue1;
  ret = PdhGetFormattedCounterValue (counter1, PDH_FMT_DOUBLE, NULL, &fmtvalue1);
  if (ret != ERROR_SUCCESS)
    return false;

  double running = fmtvalue1.doubleValue * wincap.cpu_count () / 100;

  /* Estimate the number of runnable processes using ProcessorQueueLength */
  PDH_FMT_COUNTERVALUE fmtvalue2;
  ret = PdhGetFormattedCounterValue (counter2, PDH_FMT_LONG, NULL, &fmtvalue2);
  if (ret != ERROR_SUCCESS)
    return false;

  LONG rql = fmtvalue2.longValue;

  *load = rql + running;
  return true;
}

/*
  loadavginfo shared-memory object
*/

void loadavginfo::initialize ()
{
  for (int i = 0; i < 3; i++)
    loadavg[i] = 0.0;

  last_time = 0;
}

void loadavginfo::calc_load (int index, int delta_time, int decay_time, double n)
{
  double df = 1.0 / exp ((double)delta_time/decay_time);
  loadavg[index] = (loadavg[index] * df) + (n * (1.0 - df));
}

void loadavginfo::update_loadavg ()
{
  double active_tasks;

  if (!get_load (&active_tasks))
    return;

  /* Don't recalculate the load average if less than 5 seconds has elapsed since
     the last time it was calculated */
  time_t curr_time = time (NULL);
  int delta_time = curr_time - last_time;
  if (delta_time < 5) {
    return;
  }

  if (last_time == 0) {
    /* Initialize the load average to the current load */
    for (int i = 0; i < 3; i++) {
      loadavg[i] = active_tasks;
    }
  } else {
    /* Compute the exponentially weighted moving average over ... */
    calc_load (0, delta_time, 60,  active_tasks); /* ... 1 min */
    calc_load (1, delta_time, 300, active_tasks); /* ... 5 min */
    calc_load (2, delta_time, 900, active_tasks); /* ... 15 min */
  }

  last_time = curr_time;
}

int loadavginfo::fetch (double _loadavg[], int nelem)
{
  if (!load_init ())
    return 0;

  WaitForSingleObject(mutex, INFINITE);

  update_loadavg ();

  memcpy (_loadavg, loadavg, nelem * sizeof(double));

  ReleaseMutex(mutex);

  return nelem;
}

/* getloadavg: BSD */
extern "C" int
getloadavg (double loadavg[], int nelem)
{
  /* The maximum number of samples is 3 */
  if (nelem > 3)
    nelem = 3;

  /* Return the samples and number of samples retrieved */
  return cygwin_shared->loadavg.fetch(loadavg, nelem);
}
