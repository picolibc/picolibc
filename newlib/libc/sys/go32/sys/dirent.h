/* This is file DIRENT.H */
/*
** Copyright (C) 1991 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _dirent_h_
#define _dirent_h_

#include <sys/dir.h>

struct dirent {
  unsigned short d_namlen;
  char		 d_name[14];
};

typedef struct {
  int num_read;
  char *name;
  struct ffblk ff;
  struct dirent de;
} DIR;

DIR *opendir(char *name);
struct dirent *readdir(DIR *dir);
long telldir(DIR *dir);
void seekdir(DIR *dir, long loc);
void rewinddir(DIR *dir);
int closedir(DIR *dir);

#endif
