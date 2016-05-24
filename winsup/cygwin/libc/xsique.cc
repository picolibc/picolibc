/* xsique.cc.  XSI insque and remque functions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <search.h>

extern "C" void
insque (void *velement, void *vpred)
{
  if (!velement)
    return;

  struct qelem *element = (struct qelem *) velement;
  struct qelem *pred = (struct qelem *) vpred;
  struct qelem *succ;

  if (pred)
    {
      if ((succ = element->q_forw = pred->q_forw))
	succ->q_back = element;
      pred->q_forw = element;
    }
  else
    element->q_forw = NULL;
  element->q_back = pred;
}

extern "C" void
remque (void *velement)
{
  if (!velement)
    return;

  struct qelem *pred = ((struct qelem *) velement)->q_back;
  struct qelem *succ = ((struct qelem *) velement)->q_forw;

  if (succ)
    succ->q_back = pred;
  if (pred)
    pred->q_forw = succ;
}

