/* ps.cc

   Copyright 1996, 1997, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/cygwin.h>

static char *
start_time (external_pinfo *child)
{
  time_t st = child->start_time;
  time_t t = time (NULL);
  static char stime[40] = {'\0'};
  char now[40];

  strncpy (stime, ctime (&st) + 4, 15);
  strcpy (now, ctime (&t) + 4);

  if ((t - st) < (24 * 3600))
    return (stime + 7);

  stime[6] = '\0';

  return stime;
}

int
main (int argc, char *argv[])
{
  external_pinfo *p;
  int aflag, lflag, fflag, uid;
  const char *dtitle = "  PID TTY     STIME COMMAND\n";
  const char *dfmt   = "%5d%4d%10s %s\n";
  const char *ftitle = "     UID   PID  PPID TTY     STIME COMMAND\n";
  const char *ffmt   = "%8.8s%6d%6d%4d%10s %s\n";
  const char *ltitle = "    PID  PPID  PGID   WINPID  UID TTY    STIME COMMAND\n";
  const char *lfmt   = "%c %5d %5d %5d %8u %4d %3d %8s %s\n";
  char ch;

  aflag = lflag = fflag = 0;
  uid = getuid ();

  while ((ch = getopt (argc, argv, "aelfu:")) != -1)
    switch (ch)
      {
      case 'a':
      case 'e':
        aflag = 1;
        break;
      case 'f':
        fflag = 1;
        break;
      case 'l':
        lflag = 1;
        break;
      case 'u':
        uid = atoi (optarg);
        if (uid == 0)
          {
            struct passwd *pw;

            if ((pw = getpwnam (optarg)))
              uid = pw->pw_uid;
            else
              {
                fprintf (stderr, "user %s unknown\n", optarg);
                exit (1);
              }
          }
        break;
      default:
        fprintf (stderr, "Usage %s [-aefl] [-u uid]\n", argv[0]);
        fprintf (stderr, "-f = show process uids, ppids\n");
        fprintf (stderr, "-l = show process uids, ppids, pgids, winpids\n");
        fprintf (stderr, "-u uid = list processes owned by uid\n");
        fprintf (stderr, "-a, -e = show processes of all users\n");
        exit (1);
      }

  if (lflag)
    printf (ltitle);
  else if (fflag)
    printf (ftitle);
  else
    printf (dtitle);

  (void) cygwin_internal (CW_LOCK_PINFO, 1000);

  for (int pid = 0;
       (p = (external_pinfo *) cygwin_internal (CW_GETPINFO,
						  pid | CW_NEXTPID));
       pid = p->pid)
    {
      if (p->process_state == PID_NOT_IN_USE)
        continue;
      if (!aflag && p->uid != uid)
        continue;
      char status = ' ';
      if (p->process_state & PID_STOPPED)
        status = 'S';
      else if (p->process_state & PID_TTYIN)
        status = 'I';
      else if (p->process_state & PID_TTYOU)
        status = 'O';

      char pname[MAX_PATH];
      if (p->process_state & PID_ZOMBIE)
        strcpy (pname, "<defunct>");
      else
        cygwin_conv_to_posix_path (p->progname, pname);

      char uname[128];

      if (fflag)
        {
          struct passwd *pw;

          if ((pw = getpwuid (p->uid)))
            strcpy (uname, pw->pw_name);
          else
            sprintf (uname, "%d", p->uid);
        }

      if (lflag)
        printf (lfmt, status, p->pid, p->ppid, p->pgid,
              p->dwProcessId, p->uid, p->ctty, start_time (p), pname);
      else if (fflag)
        printf (ffmt, uname, p->pid, p->ppid, p->ctty, start_time (p), pname);
      else
        printf (dfmt, p->pid, p->ctty, start_time (p), pname);

    }
  (void) cygwin_internal (CW_UNLOCK_PINFO);

  return 0;
}

