/* This is file FTRUNCAT.C */
/*
** Copyright (C) 1991 DJ Delorie, 24 Kirsten Ave, Rochester NH 03867-2954
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <fcntl.h>

/*
** Note: this only works right if you close the file right after truncating!
*/

ftruncate(int fd, unsigned long where)
{
  lseek(fd, where, 0);
  write(fd, 0, 0);
}
