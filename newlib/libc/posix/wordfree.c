/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#ifndef _NO_WORDEXP

#include <sys/param.h>
#include <sys/stat.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/queue.h>

#include <wordexp.h>
#include "wordexp2.h"

void
wordfree(wordexp_t *pwordexp)
{
  ext_wordv_t *wordv;

  if (pwordexp == NULL)
    return;

  if (pwordexp->we_wordv == NULL)
    return;

  wordv = WE_WORDV_TO_EXT_WORDV(pwordexp->we_wordv);
  while (!SLIST_EMPTY(&wordv->list)) {
    struct ewords_entry *entry = SLIST_FIRST(&wordv->list);
    SLIST_REMOVE_HEAD(&wordv->list, next);
    free(entry);
  }

  free(wordv);
  pwordexp->we_wordv = NULL;
}

#endif /* !_NO_WORDEXP  */
