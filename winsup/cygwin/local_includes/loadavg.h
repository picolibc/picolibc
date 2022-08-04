/* loadavg.h: load average support.

  This file is part of Cygwin.

  This software is a copyrighted work licensed under the terms of the
  Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
  details. */

#ifndef LOADAVG_H
#define LOADAVG_H

class loadavginfo
{
  double loadavg[3];
  time_t last_time;

 public:
  void initialize ();
  int fetch (double _loadavg[], int nelem);
  void update_loadavg ();
  void calc_load (int index, int delta_time, int decay_time, double n);
};

#endif /* LOADAVG_H */
