/*
  Copyright 2001 Free Software Foundation, Inc.
  Written by Michael Chastain, <chastain@redhat.com>
  Changes by Corinna Vinschen, <corinna@vinschen.de>:
  - Using mkstemp to generate filenames.
  - Adding tests

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
#include <string.h>

#include "test.h"
#include "usctest.h"

const char *TCID = "mmaptest01";	/* Test program identifier. */
int TST_TOTAL = 7;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

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
  char fnam1[32];
  char fnam2[32];

  int fd1;
  char * buf1;

  int fd2;
  char * buf2;

  char buf3[20];

  unsigned i;

  strcpy (fnam1, "mmaptest01.1.XXXXXX");
  strcpy (fnam2, "mmaptest01.2.XXXXXX");

  /* create file 1 */
  //fd1  = open (fnam1, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0644);
  fd1 = mkstemp (fnam1);
  for (i = 0; i < count1; i++)
    write (fd1, line1, size1);
  close (fd1);

  /* create file 2 */
  //fd2  = open (fnam2, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0644);
  fd2 = mkstemp (fnam2);
  for (i = 0; i < count2; i++)
    write (fd2, line2, size2);
  close (fd2);

  /* mmap file 1 */
  fd1  = open (fnam1, O_RDONLY | O_BINARY, 0644);
  buf1 = mmap (0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd1, 0);
  close (fd1);

  /* mmap file 2 */
  fd2  = open (fnam2, O_RDONLY | O_BINARY, 0644);
  buf2 = mmap (0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd2, 0);
  close (fd2);

  /* the buffers have to be different */
  Tst_count = 0;
  tst_resm (buf1 == buf2 || !memcmp (buf1, buf2, 20) ? TFAIL : TPASS,
	"mmap uses unique buffers when mapping different already closed files");
  munmap (buf2, 4096);

  /* mmap file 1 twice with MAP_PRIVATE */
  fd2  = open (fnam1, O_RDONLY | O_BINARY, 0644);
  buf2 = mmap (0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd2, 0);
  close (fd2);

  tst_resm (buf1 == buf2 ? TFAIL : TPASS,
	    "mmap uses different buffers on MAP_PRIVATE mapping");

  tst_resm (memcmp (buf1, buf2, 20) ? TFAIL : TPASS,
	    "two private buffers of the same file are identical");

  buf1[0] = 0x7f;
  tst_resm (!memcmp (buf1, buf2, 20) ? TFAIL : TPASS,
	    "changes are private in MAP_PRIVATE mappings");

  munmap (buf1, 4096);
  munmap (buf2, 4096);

  /* mmap file 1 twice with MAP_SHARED */
  fd1  = open (fnam1, O_RDWR | O_BINARY, 0644);
  buf1 = mmap (0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);
  close (fd1);

  fd2  = open (fnam1, O_RDWR | O_BINARY, 0644);
  buf2 = mmap (0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);
  close (fd2);

  tst_resm (memcmp (buf1, buf2, 20) ? TFAIL : TPASS,
	    "two shared buffers of the same file are identical");

  buf1[0] = 0x7f;
  tst_resm (memcmp (buf1, buf2, 20) ? TFAIL : TPASS,
	    "changes are shared between MAP_SHARED mappings of the same file");
  if (buf1 == buf2) /* Win 9x weirdness */
    msync (buf2, 4096, MS_SYNC);
  else
    munmap (buf2, 4096);

  fd2  = open (fnam1, O_RDWR | O_BINARY, 0644);
  memset (buf3, 0, 20);
  read (fd2, buf3, 20);
  close (fd2);

  tst_resm (memcmp (buf1, buf3, 20) ? TFAIL : TPASS,
	    "changes are propagated to the mapped file on MAP_SHARED mapping");

  munmap (buf1, 4096);
  unlink (fnam1);
  unlink (fnam2);
  tst_exit ();
}

