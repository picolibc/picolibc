/* scandir.cc

   Written by Corinna Vinschen <corinna.vinschen@cityweb.de>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <dirent.h>
#include <stdlib.h>
#include "cygerrno.h"

extern "C" int
alphasort (const struct dirent **a, const struct dirent **b)
{
  return strcoll ((*a)->d_name, (*b)->d_name);
}

extern "C" int
versionsort (const struct dirent **a, const struct dirent **b)
{
  return strverscmp ((*a)->d_name, (*b)->d_name);
}

extern "C" int
scandir (const char *dir,
	 struct dirent ***namelist,
	 int (*select) (const struct dirent *),
	 int (*compar) (const struct dirent **, const struct dirent **))
{
  DIR *dirp;
  struct dirent *ent, *etmp, **nl = NULL, **ntmp;
  int count = 0;
  int allocated = 0;
  int err = 0;

  if (!(dirp = opendir (dir)))
    return -1;

  if (!compar)
    compar = alphasort;

  while ((ent = readdir (dirp)))
    {
      if (!select || select (ent))
	{
	  if (count == allocated)
	    {

	      if (allocated == 0)
		allocated = 10;
	      else
		allocated *= 2;

	      ntmp = (struct dirent **) realloc (nl, allocated * sizeof *nl);
	      if (!ntmp)
		{
		  err = ENOMEM;
		  break;
		}
	      nl = ntmp;
	  }

	  if (!(etmp = (struct dirent *) malloc (sizeof *ent)))
	    {
	      err = ENOMEM;
	      break;
	    }
	  *etmp = *ent;
	  nl[count++] = etmp;
	}
    }

  if (err != 0)
    {
      closedir (dirp);
      if (nl)
	{
	  while (count > 0)
	    free (nl[--count]);
	  free (nl);
	}
      /* Ignore errors from closedir() and what not else. */
      set_errno (err);
      return -1;
    }

  closedir (dirp);

  qsort (nl, count, sizeof *nl, (int (*)(const void *, const void *)) compar);
  *namelist = nl;
  return count;
}
