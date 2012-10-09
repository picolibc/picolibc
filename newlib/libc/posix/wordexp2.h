/* Copyright (C) 2012 by  Peter Rosin. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
#ifndef _WORDEXP2_H_

struct ewords_entry {
  SLIST_ENTRY(ewords_entry) next;
  char ewords[1];
};

typedef struct {
  SLIST_HEAD(ewords_head, ewords_entry) list;
  char *we_wordv[1];
} ext_wordv_t;

#define WE_WORDV_TO_EXT_WORDV(wordv) \
  (ext_wordv_t *)((void *)(wordv) - offsetof(ext_wordv_t, we_wordv))

#endif /* !_WORDEXP2_H_ */
