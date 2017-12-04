/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <argz.h>
#include <envz.h>

error_t
envz_merge (char **envz,
       size_t *envz_len,
       const char *envz2,
       size_t envz2_len,
       int override)
{
  char *entry = NULL;
  char *name_str = NULL;
  char *val_str = NULL;
  char *name_iter = NULL;
  int retval = 0;

  while((entry = argz_next((char *)envz2, envz2_len, entry)) && !retval)
    {
      if (!override)
        {
          name_str = strdup (entry);
          name_iter = strchr(name_str, '=');
          if(name_iter)
            *name_iter = '\0';

          if(!envz_entry(*envz, *envz_len, name_str))
            {
              retval = argz_add(envz, envz_len, entry);
            }
          free(name_str);
        }
      else
        {
          name_str = strdup (entry);
          name_iter = strchr(name_str, '=');
          if(name_iter)
            {
              *name_iter = '\0';
              val_str = name_iter + 1;
            }
          else
            {
              val_str = NULL;
            }

          retval = envz_add(envz, envz_len, name_str, val_str);
          free(name_str);
        }
    }
  return retval;
}
