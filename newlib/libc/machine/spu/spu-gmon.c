/*
(C) Copyright IBM Corp. 2008

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Author: Ken Werner <ken.werner@de.ibm.com>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <ea.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <spu_timer.h>
#include <limits.h>
#include <sys/linux_syscalls.h>

/* Magic cookie.  */
#define GMON_MAGIC_COOKIE "gmon"

/* Version number.  */
#define GMON_VERSION 1

/* Fraction of text space to allocate for histogram counters.  */
#define HISTFRACTION 4

/* Histogram counter type.  */
#define HISTCOUNTER unsigned short

/* Fraction of text space to allocate for "from" hash buckets. HASHFRACTION is
   based on the minimum number of bytes of separation between two subroutine
   call points in the object code.  */
#define HASHFRACTION 4

/* Percent of text space to allocate for tostructs with a minimum.  */
#define ARCDENSITY 3

/* Minimal amount of arcs.  */
#define MINARCS 50

/* Rounding macros.  */
#define ROUNDDOWN(x,y) (((x)/(y))*(y))
#define ROUNDUP(x,y)   ((((x)+(y)-1)/(y))*(y))

/* Sampling rate in Hertz.  */
#define SAMPLE_INTERVAL 100

/* Tag definitions for the gmon.out sub headers.  */
#define GMON_TAG_TIME_HIST 0
#define GMON_TAG_CG_ARC 1

struct tostruct
{
  uintptr_t selfpc;
  long count;
  unsigned short link;
};

struct gmon_hdr
{
  char cookie[4];
  int32_t version;
  char spare[3 * 4];
};

struct gmon_hist_hdr
{
  uintptr_t low_pc;
  uintptr_t high_pc;
  int32_t hist_size;
  int32_t prof_rate;
  char dimen[15];
  char dimen_abbrev;
} __attribute__ ((packed));

struct rawarc
{
  uintptr_t raw_frompc;
  uintptr_t raw_selfpc;
  long raw_count;
} __attribute__ ((packed));

/* start and end of the text section */
extern char _start;
extern char _etext;

/* EAR entry for the starting address of SPE executable image.  */
extern const unsigned long long _EAR_;
asm (".section .toe,\"a\",@nobits\n\r"
     ".align 4\n\r"
     ".type _EAR_, @object\n\r"
     ".size _EAR_, 16\n" "_EAR_: .space 16\n" ".previous");

/* froms are indexing tos */
static __ea unsigned short *froms;
static __ea struct tostruct *tos = 0;
static long tolimit = 0;
static uintptr_t s_lowpc = 0;
static uintptr_t s_highpc = 0;
static unsigned long s_textsize = 0;

static int fd;
static int hist_size;
static int timer_id;

void
__sample (int id)
{
  unsigned int pc;
  unsigned int pc_backup;
  off_t offset;
  unsigned short val;

  if (id != timer_id)
    return;

  /* Fetch program counter.  */
  pc = spu_read_srr0 () & ~3;
  pc_backup = pc;
  if (pc < s_lowpc || pc > s_highpc)
    return;
  pc -= (uintptr_t) & _start;
  offset = pc / HISTFRACTION * sizeof (HISTCOUNTER) + sizeof (struct gmon_hdr)
             + 1 + sizeof (struct gmon_hist_hdr);

  /* Read, increment and write the counter.  */
  if (pread (fd, &val, 2, offset) != 2)
    {
      perror ("can't read the histogram");
      return;
    }
  if (val < USHRT_MAX)
    ++val;
  if (pwrite (fd, &val, 2, offset) != 2)
    {
      perror ("can't write the histogram");
    }
}

static void
write_histogram (int fd)
{
  struct gmon_hist_hdr hist_hdr;
  u_char tag = GMON_TAG_TIME_HIST;
  hist_hdr.low_pc = s_lowpc;
  hist_hdr.high_pc = s_highpc;
  hist_hdr.hist_size = hist_size / sizeof (HISTCOUNTER); /* Amount of bins.  */
  hist_hdr.prof_rate = 100; /* Hertz.  */
  strncpy (hist_hdr.dimen, "seconds", sizeof (hist_hdr.dimen));
  hist_hdr.dimen_abbrev = 's';
  struct iovec iov[2] = {
    {&tag, sizeof (tag)},
    {&hist_hdr, sizeof (struct gmon_hist_hdr)}
  };
  if (writev (fd, iov, 2) != sizeof (struct gmon_hist_hdr) + sizeof (tag))
    perror ("can't write the histogram header");

  /* Skip the already written histogram data.  */
  lseek (fd, hist_size, SEEK_CUR);
}

static void
write_callgraph (int fd)
{
  int fromindex, endfrom;
  uintptr_t frompc;
  int toindex;
  struct rawarc rawarc;
  u_char tag = GMON_TAG_CG_ARC;
  endfrom = s_textsize / (HASHFRACTION * sizeof (*froms));
  for (fromindex = 0; fromindex < endfrom; ++fromindex)
    {
      if (froms[fromindex])
	{
	  frompc = s_lowpc + (fromindex * HASHFRACTION * sizeof (*froms));
	  for (toindex = froms[fromindex]; toindex != 0;
	       toindex = tos[toindex].link)
	    {
	      rawarc.raw_frompc = frompc;
	      rawarc.raw_selfpc = tos[toindex].selfpc;
	      rawarc.raw_count = tos[toindex].count;
	      struct iovec iov[2] = {
		{&tag, sizeof (tag)},
		{&rawarc, sizeof (struct rawarc)}
	      };
	      if (writev (fd, iov, 2) != sizeof (tag) + sizeof (struct rawarc))
                perror ("can't write the callgraph");
	    }
	}
    }
}

void
__mcleanup (void)
{
  struct gmon_hdr ghdr;

  /* Disable sampling.  */
  spu_timer_stop (timer_id);
  spu_timer_free (timer_id);
  spu_clock_stop ();

  /* Jump to the beginning of the gmon.out file.  */
  if (lseek (fd, 0, SEEK_SET) == -1)
    {
      perror ("Cannot seek to the beginning of the gmon.out file.");
      close (fd);
      return;
    }

  /* Write the gmon.out header.  */
  memset (&ghdr, '\0', sizeof (struct gmon_hdr));
  memcpy (&ghdr.cookie[0], GMON_MAGIC_COOKIE, sizeof (ghdr.cookie));
  ghdr.version = GMON_VERSION;
  if (write (fd, &ghdr, sizeof (struct gmon_hdr)) == -1)
    {
      perror ("Cannot write the gmon header to the gmon.out file.");
      close (fd);
      return;
    }

  /* Write the sampling buffer (histogram).  */
  write_histogram (fd);

  /* Write the call graph.  */
  write_callgraph (fd);

  close (fd);
}

void
__monstartup (unsigned long long spu_id)
{
  char filename[64];
  s_lowpc =
    ROUNDDOWN ((uintptr_t) & _start, HISTFRACTION * sizeof (HISTCOUNTER));
  s_highpc =
    ROUNDUP ((uintptr_t) & _etext, HISTFRACTION * sizeof (HISTCOUNTER));
  s_textsize = s_highpc - s_lowpc;

  hist_size = s_textsize / HISTFRACTION * sizeof (HISTCOUNTER);

  /* Allocate froms.  */
  froms = malloc_ea (s_textsize / HASHFRACTION);
  if (froms == NULL)
    {
      fprintf (stderr, "Cannot allocate ea memory for the froms array.\n");
      return;
    }
  memset_ea (froms, 0, s_textsize / HASHFRACTION);

  /* Determine tolimit.  */
  tolimit = s_textsize * ARCDENSITY / 100;
  if (tolimit < MINARCS)
    tolimit = MINARCS;

  /* Allocate tos. */
  tos = malloc_ea (tolimit * sizeof (struct tostruct));
  if (tos == NULL)
    {
      fprintf (stderr, "Cannot allocate ea memory for the tos array.\n");
      return;
    }
  memset_ea (tos, 0, tolimit * sizeof (struct tostruct));

  /* Determine the gmon.out file name.  */
  if (spu_id)
    snprintf (filename, sizeof (filename), "gmon-%d-%llu-%llu.out",
	      linux_getpid (), spu_id, _EAR_);
  else
    strncpy (filename, "gmon.out", sizeof (filename));
  /* Open the gmon.out file.  */
  fd = open (filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (fd == -1)
    {
      char errstr[128];
      snprintf (errstr, sizeof (errstr), "Cannot open file: %s", filename);
      perror (errstr);
      return;
    }
  /* Truncate the file up to the size where the histogram fits in.  */
  if (ftruncate (fd,
		 sizeof (struct gmon_hdr) + 1 +
		 sizeof (struct gmon_hist_hdr) + hist_size) == -1)
    {
      char errstr[128];
      snprintf (errstr, sizeof (errstr), "Cannot truncate file: %s", filename);
      perror (errstr);
      return;
    }

  /* Start the histogram sampler.  */
  spu_slih_register (MFC_DECREMENTER_EVENT, spu_clock_slih);
  timer_id = spu_timer_alloc (spu_timebase () / SAMPLE_INTERVAL, __sample);
  spu_clock_start ();
  spu_timer_start (timer_id);

  atexit (__mcleanup);
}

void
__mcount_internal (uintptr_t frompc, uintptr_t selfpc)
{
  /* sefpc: the address of the function just entered.  */
  /* frompc: the caller of the function just entered.  */
  unsigned int mach_stat;
  __ea unsigned short *frompcindex;
  unsigned short toindex;
  __ea struct tostruct *top;
  __ea struct tostruct *prevtop;

  /* Save current state and disable interrupts.  */
  mach_stat = spu_readch(SPU_RdMachStat);
  spu_idisable ();

  /* Sanity checks.  */
  if (frompc < s_lowpc || frompc > s_highpc)
    goto done;
  frompc -= s_lowpc;
  if (frompc > s_textsize)
    goto done;

  /* frompc indexes into the froms array the value at that position indexes
     into the tos array.  */
  frompcindex = &froms[(frompc) / (HASHFRACTION * sizeof (*froms))];
  toindex = *frompcindex;
  if (toindex == 0)
    {
      /* First time traversing this arc link of tos[0] incremented.  */
      toindex = ++tos[0].link;
      /* Sanity check.  */
      if (toindex >= tolimit)
	{
	  --tos[0].link;
	  goto done;
	}
      /* Save the index into the froms array for the next time we traverse this arc.  */
      *frompcindex = toindex;
      top = &tos[toindex];
      /* Sets the address of the function just entered.  */
      top->selfpc = selfpc;
      top->count = 1;
      top->link = 0;
      goto done;
    }

  /* toindex points to a tostruct */
  top = &tos[toindex];
  if (top->selfpc == selfpc)
    {
      /* The arc is at front of the chain. This is the most common case.  */
      top->count++;
      goto done;
    }

  /* top->selfpc != selfpc
     The pc we have got is not the pc we already stored (i.e. multiple function
     calls to the same fuction within a function. The arc is not at front of
     the chain.  */
  for (;;)
    {
      if (top->link == 0)
	{
	  /* We are at the end of the chain and selfpc was not found. Thus we create
	     a new tostruct and link it to the head of the chain.  */
	  toindex = ++tos[0].link;
	  /* Sanity check.  */
	  if (toindex >= tolimit)
	    {
	      --tos[0].link;
	      goto done;
	    }
	  top = &tos[toindex];
	  top->selfpc = selfpc;
	  top->count = 1;
	  /* Link back to the old tos entry.  */
	  top->link = *frompcindex;
	  /* Store a link to the new top in the froms array which makes the
	     current tos head of the chain.  */
	  *frompcindex = toindex;
	  goto done;
	}
      else
	{
	  /* Otherwise check the next arc on the chain.  */
	  prevtop = top;
	  top = &tos[top->link];
	  if (top->selfpc == selfpc)
	    {
	      /* selfpc matches; increment its count.  */
	      top->count++;
	      /* Move it to the head of the chain.  */
	      /* Save previous tos index.  */
	      toindex = prevtop->link;
	      /* Link the former to to the current tos.  */
	      prevtop->link = top->link;
	      /* Link back to the old tos entry.  */
	      top->link = *frompcindex;
	      /* Store a link to the new top in the froms array which makes the
	         current tos head of the chain.  */
	      *frompcindex = toindex;
	      goto done;
	    }
	}
    }
done:
  /* Enable interrupts if necessary.  */
  if (__builtin_expect (mach_stat & 1, 0))
    spu_ienable ();
}
