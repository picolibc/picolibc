/*
  Copyright 2001 Free Software Foundation, Inc.
  Written by Michael Chastain, <chastain@redhat.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.

  This program demonstrates a bug in cygwin's mmap.
  I open one file, mmap it, and close it.
  I open a different file, mmap it, and close it.

  The second file re-uses the file handle (which is OK),
  but then it re-uses the buffer as well!  Ouch!

  This bug occurs on cygwin1.dll dated 2001-01-31.
  It causes gnu cpp to screw up its include file buffers.

  Compile with "gcc -o y1 y1.c".

  Output from a bad cygwin1.dll:

    y1.txt: 3 0x4660000 y1 y1 y1 y1 y1 y1 y1
    y2.txt: 3 0x4660000 y1 y1 y1 y1 y1 y1 y1

  Output from a good cygwin1.dll:

    y1.txt: 3 0x14060000 y1 y1 y1 y1 y1 y1 y1
    y2.txt: 3 0x14070000 y2 y2 y2 y2 y2 y2 y2

  Output from Red Hat Linux 7:

    y1.txt: 3 0x40017000 y1 y1 y1 y1 y1 y1 y1
    y2.txt: 3 0x40018000 y2 y2 y2 y2 y2 y2 y2
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

/* some systems have O_BINARY and some do not */
#ifndef O_BINARY
#define O_BINARY 0
#endif

/* filler for file 1 */
char const line1 [] = "y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1\n";
#define size1 (sizeof(line1) - 1)
#define count1 ((4096 / size1) + 1)

/* filler for file 2 */
char const line2 [] = "y2 y2 y2 y2 y2 y2 y2 y2 y2 y2 y2 y2 y2 y2 y2 y2 y2\n";
#define size2 (sizeof(line2) - 1)
#define count2 ((4096 / size2) + 1)

int main ()
{
  int fd1;
  char * buf1;

  int fd2;
  char * buf2;

  int i;

  /* create file 1 */
  fd1  = open ("y1.txt", O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0644);
  for (i = 0; i < count1; i++)
    write (fd1, line1, size1);
  close (fd1);

  /* create file 2 */
  fd2  = open ("y2.txt", O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0644);
  for (i = 0; i < count2; i++)
    write (fd2, line2, size2);
  close (fd2);

  /* mmap file 1 */
  fd1  = open ("y1.txt", O_RDONLY | O_BINARY, 0644);
  buf1 = mmap (0, 4096, PROT_READ, MAP_PRIVATE, fd1, 0);
  close (fd1);

  /* mmap file 2 */
  fd2  = open ("y2.txt", O_RDONLY | O_BINARY, 0644);
  buf2 = mmap (0, 4096, PROT_READ, MAP_PRIVATE, fd2, 0);
  close (fd2);

  /* the buffers have to be different */
  if (buf1 == buf2 || !memcmp (buf1, buf2, 20))
    return 1;

  return 0;
}

