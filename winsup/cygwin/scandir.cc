/* scandir.cc

   Copyright 1998 Cygnus Solutions.

   Written by Corinna Vinschen <corinna.vinschen@cityweb.de>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include "cygerrno.h"

extern "C"
int
scandir (const char *dir,
	 struct dirent ***namelist,
	 int (*select) (const struct dirent *),
	 int (*compar) (const struct dirent **, const struct dirent **))
{
  DIR *dirp;
  struct dirent *ent, *etmp, **nl = NULL, **ntmp;
  int count = 0;
  int allocated = 0;

  if (!(dirp = opendir (dir)))
    return -1;

  int prior_errno = get_errno ();
  set_errno (0);

  while ((ent = readdir (dirp)))
    {
      if (!select || select (ent))
	{

	  /* Ignore error from readdir/select. See POSIX specs. */
	  set_errno (0);

	  if (count == allocated)
	    {

	      if (allocated == 0)
		allocated = 10;
	      else
		allocated *= 2;

	      ntmp = (struct dirent **) realloc (nl, allocated * sizeof *nl);
	      if (!ntmp)
		{
		  set_errno (ENOMEM);
		  break;
		}
	      nl = ntmp;
	  }

	  if (!(etmp = (struct dirent *) malloc (sizeof *ent)))
	    {
	      set_errno (ENOMEM);
	      break;
	    }
	  *etmp = *ent;
	  nl[count++] = etmp;
	}
    }

  if ((prior_errno = get_errno ()) != 0)
    {
      closedir (dirp);
      if (nl)
	{
	  while (count > 0)
	    free (nl[--count]);
	  free (nl);
	}
      /* Ignore errors from closedir() and what not else. */
      set_errno (prior_errno);
      return -1;
    }

  closedir (dirp);
  set_errno (prior_errno);

  qsort (nl, count, sizeof *nl, (int (*)(const void *, const void *)) compar);
  if (namelist)
    *namelist = nl;
  return count;
}

extern "C"
int
alphasort (const struct dirent **a, const struct dirent **b)
{
  return strcoll ((*a)->d_name, (*b)->d_name);
}

