/* Copyright (c) 1995, 2002, 2009 Xilinx, Inc.  All rights reserved. 
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
   
   1.  Redistributions source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
   
   2.  Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution. 
   
   3.  Neither the name of Xilinx nor the names of its contributors may be
   used to endorse or promote products derived from this software without
   specific prior written permission. 
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef DEBUG
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#else
typedef unsigned int size_t;
#define NULL 0
#endif

#define sbrk xil_sbrk

/* The only extern functions I need if not printing. */
extern  void* sbrk(size_t incr);
extern  void *memcpy(void *s1, const void *s2, size_t n);
extern  void *memset(void *s, int c, size_t n);


typedef unsigned char BOOLEAN;
const BOOLEAN FALSE=0;
const BOOLEAN TRUE =1;

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#define M_DBG_NORMAL 0
#define M_DBG_PARTIAL 1
#define M_DBG_FULL 2

/* debugging breakpoint aids */
static char xil_mem_null_free[] = "xil_mem_null_free";
static char xil_mem_chkcnt   [] = "xil_mem_chkcnt";

/* Flag values describing the state of a memory block.
/* Indicator for allocated blk */
#define M_ALLOCEDFLAG 0x5a
/* End-of-block if debug level */
#define M_ALLOCED 0xc99cc99c
/* Free block indicator. */
#define M_FREEFLAG 0xa5
/* End-of-block if debug level */
#define M_FREE 0x9cc99cc9
/* Zero length block. */
#define M_ZEROFLAG 0xaa

/* Header of a memory block. */
typedef unsigned char DATA_T;
typedef DATA_T *      DATA_P;
struct M_HEADER
{
  unsigned       dbglev:2;       /* Debug level this was created with. */
  unsigned       size:22;        /* Size of block / 8. 32 Meg max. */
  unsigned       flag:8;         /* Indicates whether allocated or freed. */
};
typedef struct M_HEADER* M_HEADERP;

BOOLEAN isalloced(M_HEADERP this)      
{ 
  return this->flag == M_ALLOCEDFLAG; 
}
BOOLEAN isfree(M_HEADERP this)         
{ 
  return this->flag == M_FREEFLAG; 
}
BOOLEAN iszero(M_HEADERP this)         
{ 
  return this->flag == M_ZEROFLAG; 
}

void           setalloced(M_HEADERP this)     { this->flag = M_ALLOCEDFLAG; }
void           setfree(M_HEADERP this)        { this->flag = M_FREEFLAG; }
void           setzero(M_HEADERP this)        { this->flag = M_ZEROFLAG; }

int            getdbglev(M_HEADERP this)      { return this->dbglev; }
void           setdbglev(M_HEADERP this, int d) { this->dbglev = d; }

size_t         getsize(M_HEADERP this)        { return this->size << 3; }  /* Alignment is 8. */
void           setsize(M_HEADERP this, size_t s){ this->size = s >> 3; }     

DATA_T *       getend(M_HEADERP this)         { return (((DATA_T *)this)+getsize(this)); }

/* Next pointer is after data in block. */
M_HEADERP     getnext(M_HEADERP this)        { return *(((M_HEADERP*)getend(this)) - 1); }
void           setnext(M_HEADERP this, M_HEADERP n) { *(((M_HEADERP*)getend(this)) - 1) = n; }

/* Routines used to set a flag at end of block if debuglevel != normal. */
/* Sentinel is right BEFORE the next pointer. */
unsigned long* getsentinel(M_HEADERP this);
void           setsentinel(M_HEADERP this, unsigned long lflag);
BOOLEAN        testsentinel(M_HEADERP this, unsigned long lflag);

/* Routines to handle data.  Depend on debug level. */
DATA_T *       getdata(M_HEADERP this)        { return (((DATA_T*)this)+sizeof(*this)); }
size_t         getdatasize(M_HEADERP this);

/* Fill data with a pattern. */
void           setdata(M_HEADERP this, int f);

/* Debug routines */
BOOLEAN        checkalloc(M_HEADERP this);    /* Is this a valid allocated memory pointer? */
BOOLEAN        checkfree(M_HEADERP this);     /* Is this a valid freelist entry? */



/* Get length of data. */
size_t 
getdatasize(M_HEADERP this)
{
  /* By default, size is size of block - size of header. */
  int tmp_size = getsize(this) - sizeof(struct M_HEADER);

  if (this->dbglev != M_DBG_NORMAL) 
    {
      /* Subtract size of sentinel, and next pointer. */
      tmp_size -= sizeof(long) + sizeof(M_HEADERP);
      /* If only eight bytes, no room for sentinel. */
      if (tmp_size < 0)
        tmp_size = 0;
    } 
  else 
    {
      /* Free block always has a next pointer.  Otherwise not. */
      if (isfree(this))
        tmp_size -= sizeof(M_HEADERP);
    }
  return tmp_size;
}

/* Set the data buffer to value f. */
void 
setdata(M_HEADERP this, int f)
{ 
  memset(getdata(this), f, getdatasize(this));
}

/* At the end of the block, there may be a longword with
   special meaning.  This is the sentinel.  If there is a sentinel,
   there is by definition a next pointer. */
unsigned long* 
getsentinel(M_HEADERP this)
{
  DATA_T* addr = (getend(this) - sizeof(M_HEADERP)); /* location of next pointer. */
  if (getdata(this) < addr)
    return ((unsigned long*)addr) - 1;        /* Right before next pointer. */
  else
    return NULL;                      /* Block too small.  No room for sent. */
}

void 
setsentinel(M_HEADERP this, unsigned long lflag)
{
  unsigned long* addr = getsentinel(this);
  if (addr)
    *addr = lflag;
}

BOOLEAN 
testsentinel(M_HEADERP this, unsigned long lflag)
{
  unsigned long* addr = getsentinel(this);
  if (addr)
    return *addr == lflag;
  else
    return TRUE;
}

/*  sizeof(struct M_HEADER)+sizeof(M_HEADERP);  Alignment */
#define M_BLOCKSIZE 8
/*  4096 / 8; // M_BLOCKSIZE ;      Number of freelist entries. */
#define M_FREESIZE 512
/*  64 * 1024;                 Size of incremental memory hunks allocated, */
#define M_BRKINC 2048

static M_HEADERP freelist[M_FREESIZE];       /* Free list. */

static M_HEADERP alloclist = NULL;           /* Pointer to linked list
                                                of Allocated blocks. */
static int mdebuglevel = M_DBG_NORMAL;

static DATA_T zerobuf[M_BLOCKSIZE] = { M_ZEROFLAG, M_ZEROFLAG, M_ZEROFLAG,
                                       M_ZEROFLAG, M_ZEROFLAG, M_ZEROFLAG, 
                                       M_ZEROFLAG, M_ZEROFLAG };
static M_HEADERP zeroblock = (M_HEADERP)zerobuf;

static unsigned long totalallocated = 0;        /* NOT actually malloced, but
                                                   rather the size of the pool. */

static unsigned long totalmalloc = 0;           /* Total amount malloced. */

static unsigned long highwater = 0;             /* Largest amount of memory
                                                   allocated at any time. */
static long nummallocs = 0;
static long numfrees = 0;
static long numreallocs = 0;

int m_prtflag  = 0;
int m_stopaddr = 0;
int m_stopcnt  = 0;
int m_reenter  = 0;
static int m_curcount = 0;

M_HEADERP 
getmemblock(size_t n)
{
  M_HEADERP block = (M_HEADERP) sbrk(n);
  if (block != NULL)
    totalallocated += n;

  return block;
}


static BOOLEAN 
die (char* msg)
{
  mdebuglevel = M_DBG_NORMAL;
#ifdef DEBUG
  printf ("%s\n", msg);
  exit (1);
#else
  /* Go into infinite loop. */
  for (;;)
    ;
#endif
  return FALSE;
}

int 
getfreeindex(size_t size)
{
  return MIN(size / M_BLOCKSIZE, M_FREESIZE - 1);
}

static
void coalesce(M_HEADERP h)
{
  /* Coalesce block h with free block any free blocks after it.
     Assumes that H is currently allocated.  Sentinel at end is
     set to allocated so if H is free, caller has to fix it. */
  for (;;) 
    {
      long i;
      M_HEADERP f;
      M_HEADERP next = (M_HEADERP)getend(h);

      if (next || isalloced(next))
        break; /* no more coalscing can be done. */
         
      /* Take it off the free list. */
      i = getfreeindex(getsize(next));
      f = freelist[i];
      if (f == next)
        freelist[i] = getnext(next);
      else 
	{
          while (f != NULL && getnext(f) != next)
            f = getnext(f);

          /* Didn't find it in the free list. */
          if (f == NULL)
            die ("Coalesce failed.");

          setnext(f, getnext(next));
        }

      /* Add two blocks together and start over. */
      setsize(h, getsize(h) + getsize(next));

      if (getdbglev(h) > M_DBG_NORMAL) 
	{
          setsentinel(h, M_ALLOCED);
        }
    } /* forever */
}

BOOLEAN 
checkalloc(M_HEADERP this)
{
  if (!isalloced(this))
    return die ("checkalloc: pointer header clobbered.");

  if (getdbglev(this) > M_DBG_NORMAL) 
    {
      if (!testsentinel(this, M_ALLOCED))
        return die ("checkalloc: pointer length overrun.");
    }
  return TRUE;
}

BOOLEAN 
checkfree(M_HEADERP this)
{
  DATA_T *d;
  int i;
  if (!isfree(this))
    die ("checkfree: pointer header clobbered.");

  if (getdbglev(this) > M_DBG_NORMAL) 
    {
      if (!testsentinel(this, M_FREE))
        die ("checkfree: pointer length overrun.");

      d = getdata(this);
      i = getdatasize(this);
      while (i-- > 0) {
        if (*d++ != M_FREEFLAG)
          die("checkfree: freed data clobbered.");
      }
    }
  return TRUE;
}

static void 
checkfreelist()
{
  long i;
  for (i = 0; i < M_FREESIZE; i += 1) 
    {
      M_HEADERP h = (M_HEADERP) freelist[i];
      while (h != NULL) 
        {
        checkfree(h);
        if (i != (M_FREESIZE - 1) && getsize(h) != (i * M_BLOCKSIZE))
          die ("checkfreelist: free list size mismatch.");
        h = getnext(h);
        }
    }
}

static void 
checkalloclist()
{
  M_HEADERP a = (M_HEADERP) alloclist;
  while (a != NULL) 
    {
      checkalloc(a);
      a = getnext(a);
    }
}

/* Free a block of memory.  This is done by adding to the free list. */
static void 
addtofreelist (M_HEADERP h)
{
  long i;
  /* Merge freed blocks together. */
  coalesce(h);

  /* link this block to the front of the appropriate free list. */
  i = getfreeindex(getsize(h));
  setnext(h, freelist[i]);
  freelist[i] = h;

  /* Set the flag info. */
  setfree(h);
  setdbglev(h, mdebuglevel);
  if (mdebuglevel > M_DBG_NORMAL) 
    {
      /* Fill with some meaningful (and testable) data. */
      setdata(h, M_FREEFLAG);
      setsentinel(h, M_FREE);
    }
}

void 
xil_malloc_verify()
{
  int i;
  for ( i = 0; i < M_BLOCKSIZE; i += 1) 
    {
      if (zerobuf[i] != M_ZEROFLAG)
        die ("malloc_verify: Zero block clobbered.");
    }
  checkfreelist();
  checkalloclist();
}

void 
xil_malloc_debug (int level)
{
  mdebuglevel = MAX (M_DBG_NORMAL, MIN (M_DBG_FULL, level));
}

void* 
xil_malloc (size_t nbytes)
{
  int i;
  int minf;
  int maxf;
  size_t msize;
  M_HEADERP p;
  M_HEADERP h;

  nummallocs += 1;

  if (nbytes == 0)
    return getdata(zeroblock);

  if (mdebuglevel == M_DBG_FULL)
    {
#ifdef DEBUG
      static unsigned do_cnt = ~0;
      static unsigned done_cnt = 0;
      if (do_cnt == ~0) 
	{
          char *x = (char *)getenv(xil_mem_chkcnt);
          do_cnt = 1;
          if (x)
            do_cnt = atoi(x);
        }
      if (do_cnt == 1 || done_cnt % do_cnt == 0)
        xil_malloc_verify();
      done_cnt++;
#else
      xil_malloc_verify();
#endif
    }

  nbytes += sizeof (struct M_HEADER);

  /* If debug, leave room for flag and next pointer. */
  if (mdebuglevel > M_DBG_NORMAL)
    nbytes += sizeof (long) + sizeof (M_HEADERP*);

  /* Round up to allocation unit */
  msize = ((nbytes + M_BLOCKSIZE - 1) / M_BLOCKSIZE) * M_BLOCKSIZE;

  /* Look around for a block of approximately the right size. */
  h = NULL;
  minf = getfreeindex(msize);
  maxf = MIN(minf * 2, M_FREESIZE);

  for (i = minf; i < M_FREESIZE; i += 1) 
    {
      if (i >= maxf)
        i = M_FREESIZE - 1;    /* Skip over blocks too large. */

      h = freelist[i];
      p = NULL;       /* Previous. */
      while (h != NULL) 
	{
          if (getsize(h) >= nbytes) 
	    {
              /* Take h out of linked list */
              if (p)
                setnext(p, getnext(h));
              else
                freelist[i] = getnext(h);

              if (!isfree(h))
                die ("malloc: freelist clobbered.\n");

              goto gotit;
            }
          else 
	    {
              p = h;
              h = getnext(h);
            }
        }
    }

  /* Didn't find any free pointers.  Allocate more heap. 
     Round up to next heap increment. */
  i = ((msize + sizeof(long) + M_BRKINC - 1) / M_BRKINC) * M_BRKINC;
  if ((h = getmemblock (i)) == NULL) 
    {
#ifdef DEBUG
      printf ("xil_malloc: Out of dynamic memory.\n");
#endif
      return NULL;
    }

  /* Mark end of block with zero for four bytes so we don't merge next block 
     into free list accidentally. */
  setsize(h, i - sizeof(long));
  *((long*)getend(h)) = 0;

 gotit:
  /* Merge allocated blocks so we can free a bigger part of what is left! */
  coalesce(h);
  if (getsize(h) >= msize + M_BLOCKSIZE) 
    {
      M_HEADERP r;
      int rsize;
      /* add the remainder of this block to the free list. */
      rsize = getsize(h) - msize;
      r = (M_HEADERP) (((DATA_T *)h) + msize);
      setsize (r, rsize);
      setsize (h, msize);
      addtofreelist (r);
    }

  setalloced(h);
  setdbglev(h, mdebuglevel);
  if (mdebuglevel > M_DBG_NORMAL) 
    {
      // Chain into alloc'd list and set sentinel. */
      setsentinel(h, M_ALLOCED);
      setnext(h, alloclist);
      alloclist = h;
    }

#ifdef DEBUG
  if (!m_reenter && m_prtflag) 
    {
      m_reenter = 1;
      printf("%d      malloc\n",h+1);
      fflush(stdout);
      if (m_stopaddr)
        {
          if ((DATA_T *)m_stopaddr == getdata(h))
            {
              if (m_stopcnt == ++m_curcount)
                exit(10);
            }
        }
      m_reenter = 0;
    }
#endif

  totalmalloc += getsize(h);
  if (totalmalloc > highwater)
    highwater = totalmalloc;

  return getdata(h);
}

void 
xil_free(void* ap)
{
  M_HEADERP h;
  numfrees += 1;

  if (ap == NULL) 
   {
#ifdef DEBUG
     if (mdebuglevel != M_DBG_NORMAL && getenv(xil_mem_null_free))
       die ("free: tried to free NULL pointer.");
     else
       return;        /* Let `em do it. */
#else
     return;
#endif
   }

  /* Drop through to here if not a smartheap allocation.  This
     handles free of both xil_malloc and libc malloc. */

  h = (M_HEADERP) (((DATA_T *)ap) - sizeof (struct M_HEADER));

  if (h == zeroblock)
    return;

#ifdef DEBUG
  if (!m_reenter && m_prtflag) {
    m_reenter = 1;
    printf("%d      mfree\n",h+1);
    fflush(stdout);
    m_reenter = 0;
  }
#endif

  if (!isalloced(h)) {
    if (isfree(h))
      die ("free: tried to free pointer twice.");
    else
      die ("free: tried to free a block not allocated by malloc.");
    return;
  }

  if (getdbglev(h) > M_DBG_NORMAL) 
    {
      /* Make sure things look reasonable. */
      checkalloc(h);

      /* Try to find the pointer in the alloc list. */
      if (alloclist == h)
        alloclist = getnext(h);
      else 
	{
          M_HEADERP a = alloclist;
          while (a != NULL && getnext(a) != h)
            a = getnext(a);

          /* If a is NULL, debuglevel must have been reset at some point. */
          if (a != NULL)
            setnext(a, getnext(h));
        }
    }

  totalmalloc -= getsize(h);

  addtofreelist (h);

  if (mdebuglevel == M_DBG_FULL) 
    {
#ifdef DEBUG
      static unsigned do_cnt = ~0;
      static unsigned done_cnt = 0;
      if (do_cnt == ~0) 
	{
          char *x = (char *)getenv(xil_mem_chkcnt);
          do_cnt = 1;
          if (x)
            do_cnt = atoi(x);
        }
      if (do_cnt == 1 || done_cnt % do_cnt == 0)
        xil_malloc_verify();
      done_cnt++;
#else
      xil_malloc_verify();
#endif
    }
}

unsigned 
xil_msize (void* ap)
{
  M_HEADERP h = (M_HEADERP) (((DATA_T *)ap) - sizeof (struct M_HEADER));
  return getdatasize(h);
}

void* 
xil_realloc (void* oldblk, size_t newsize )
{
  M_HEADERP h;
  size_t oldsize;
  void* newblk;

  numreallocs += 1;

  if (oldblk == NULL) 
    {
      if (mdebuglevel != M_DBG_NORMAL)
        die ("realloc: tried to realloc NULL pointer.");
      else
        return xil_malloc(newsize);        /* Don't need to copy anything. */
    }

  /* Make sure this is a valid block. */
  h = (M_HEADERP) (((char*)oldblk) - sizeof (struct M_HEADER));

  /* if old block was zero bytes, just alloc a new one. */
  if (h == zeroblock)
    return xil_malloc(newsize);           /* Source is empty anyway. */

  /* If old block was already freed, error. */
  if (isfree(h))
    die ("realloc: tried to realloc freed pointer.");

  if (!isalloced(h)) 
    {
      long* pdesc = *(long**)h;         /* Get pointer to the block descriptor. */
      long* pnextdesc = (long*)*pdesc;
      if ((pdesc[1] & ~3) != (long)h)   /* Should point back to block. */
        die ("realloc: header clobbered.");

      /* This must be a libc block.  We need to figure out how big it is.
         Length of block is delta between two descriptors - sizeof (void*). */
      
      oldsize = (size_t) ((pnextdesc[1] & ~3) - (pdesc[1] & ~3)-sizeof(void*));

      /* Don't bother to change anything unless there's not enough room. */
      if (oldsize < newsize) 
	{
          /* Alloc a new block with our malloc. */
          if ((newblk = xil_malloc(newsize)) == NULL )
            return NULL ;

          /* Copy the old data to it. */
          memcpy (newblk, oldblk, (newsize < oldsize) ? newsize : oldsize);
          xil_free(oldblk);
          return newblk;
        }
    }

  /* If the new size is bigger than my allocated
     size, or if more than 1/4 of the block would be left free, allocate
     a new block and copy the data.  Otherwise, leave well enough alone. */

  coalesce(h);

  oldsize = getdatasize(h);

  if (oldsize < newsize 
      || (newsize > (2*M_BLOCKSIZE) && (newsize*4) < (oldsize*3))) 
    {
      if (( newblk = xil_malloc( newsize )) == NULL )
        return NULL ;

      memcpy (newblk, oldblk, (newsize < oldsize) ? newsize : oldsize);

      xil_free (oldblk);
      return newblk;
    }
  else
    return oldblk;
}

void* 
xil_calloc (size_t number, size_t size)
{
  long*  longptr ;
  void*  blockptr ;
  size_t temp   = number * size + sizeof (long) - 1;
  temp -= temp % sizeof (long);

  blockptr = xil_malloc( temp );
  if ( blockptr != 0 ) 
    {
      longptr = (long*) blockptr ;
      temp /= sizeof (long);
      while ( temp-- > 0 ) 
	{
          *longptr++ = 0 ;
        }
    }
  return blockptr ;
}

#define M_STAT_NORMAL 0
#define M_STAT_VERBOSE 1
#define M_STAT_REALLYVERBOSE 2

#ifdef DEBUG
void 
xil_mstats(int verbosity)
{  
  unsigned long totalfree = 0;
  int i;
  printf("Memory Statics:\n"
         "---------------\n");
  printf("   Number of calls to malloc:   %ld.\n", nummallocs);
  printf("   Number of calls to free:     %ld.\n", numfrees);
  printf("   Number of calls to realloc:  %ld.\n", numreallocs);
  printf("   Total allocated memory:      %lu (0x%lx)\n",
         totalallocated, totalallocated);
  printf("   Currently malloced memory:   %lu (0x%lx)\n",
         totalmalloc, totalmalloc);
  fflush(stdout);

 
  for (i = 0; i < M_FREESIZE; i += 1) 
    {
      M_HEADERP h = freelist[i];
      unsigned long numblocks = 0;
      while (h != NULL) 
	{
          totalfree += getsize(h);
          numblocks += 1;
          h = getnext(h);
        }
      if (verbosity > M_STAT_NORMAL && numblocks > 0) 
	{
          printf("   There are %d blocks on freelist for size %d\n",
                 numblocks, i * M_BLOCKSIZE);
          fflush(stdout);
        }
    }
  printf("   Currently free memory:       %lu (0x%lx)\n",
         totalfree, totalfree);
  printf("   High water mark:             %lu (0x%lx)\n",
         highwater, highwater);

  printf("\n");
  fflush(stdout);
}
#else
void 
xil_mstats(int verbosity)
{
}
#endif
