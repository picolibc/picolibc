/* Test of external malloc, calloc, realloc and free capability */

#if 1
#include "test.h"
#include "usctest.h"
#else
enum {TPASS, TFAIL, TBROK, TINFO};
#define tst_resm(xxx, yyy...) printf(yyy), printf(" RES %d\n", xxx)
#define tst_brkm(xxx, yyy, zzz...) printf(zzz), printf(" RES %d\n", xxx)
#define tst_exit()
int Tst_count;
#endif

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

const char *TCID = "malloc";    /* Test program identifier. */
int TST_TOTAL = 2;              /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */

/* Main test.
   Verbose mode if argc > 0
   Note that malloc and friends are also called by Cygwin before main,
   and that malloc can call getenv. */

int malloc_error = 0, realloc_error = 0, free_error = 0; 
int calloc_count = 0, malloc_count = 0, realloc_count = 0, free_count = 0;

void
cleanup (void)
{
  tst_exit(); 
}

int
syncwithchild (pid_t pid, int expected_exit_status)
{
  int status;

  if (waitpid (pid, &status, 0) != pid)
    {
      tst_brkm (TBROK, cleanup, "Wait for child: %s", strerror (errno));
      return 1;
    }
  if (!WIFEXITED (status))
    {
      tst_brkm (TBROK, cleanup, "Child had abnormal exit");
      return 1;
    }
  if (WEXITSTATUS (status) != expected_exit_status)
    {
      tst_brkm (TFAIL, cleanup, "Child had exit status %d != %d",
		WEXITSTATUS (status), expected_exit_status);
      return 1;
    }
  return 0;
}

#if 0
void * mallocX (size_t size);
void * callocX (size_t nmemb, size_t size);
void * reallocX (void * ptr, size_t size);
void freeX(void *);

#define malloc mallocX
#define calloc callocX
#define realloc reallocX
#define free freeX
#endif

int main (int argc, char * argv[])
{ 
  void * ptr;
  int error = 0;
  pid_t pid;

  Tst_count = 0;

  tst_resm(TINFO, "Testing if external malloc works. ppid %d", getppid());

  ptr = malloc (16);
  ptr = calloc (1, 16);
  ptr = realloc (ptr, 24);
  free (ptr);

  error = (malloc_count == 0 || calloc_count == 0 || realloc_count == 0 || free_count == 0);

  if (error || argc > 1)
    {
      printf ("malloc_count %d, calloc_count %d, realloc_count %d, free_count %d\n", 
	      malloc_count, calloc_count, realloc_count, free_count);
      printf ("malloc_error %d, realloc_error %d, free_error %d\n",
	      malloc_error, realloc_error, free_error);
    }
  tst_resm (!error ? TPASS : TFAIL, "Running in pid %d", getpid());

  /* If run from Windows, run also from Cygwin */
  if (getppid() == 1)
    {
      tst_resm(TINFO, "Ready to test if malloc works from Cygwin");

      if ((pid = fork()) == 0)
	{
	  tst_resm(TINFO, "Ready to exec with pid %d\n", getpid());
	  error = execl(argv[0], argv[0], argc > 1? argv[1]:NULL, NULL);
	  exit(error);
	}
      else if (pid < 0)
	tst_brkm (TBROK, cleanup, "Fork failed: %s", strerror (errno));
      else
	{
	  error = syncwithchild (pid, 0);
	  tst_resm (!error ? TPASS : TFAIL, "Running in pid %d", pid);
	}
    }

  tst_exit ();
}

/****************************************
Actual malloc & friends implementation 
****************************************/

typedef unsigned long long ull;
 
#define SIZE (1024*1024ULL) /* long long */ 
ull buffer[SIZE]; 
ull * current = buffer;

static int is_valid (void * ptr)
{
  unsigned int iptr = (unsigned int) ptr;
  ull * ullptr = (ull *) ptr;

  iptr = (iptr / sizeof(ull)) * sizeof(ull);
  if (iptr != (int) ptr)
    return 0;
  if (--ullptr < buffer || ullptr[0] > SIZE || ullptr  + ullptr[0]  > current)
    return 0;
  return 1;
}

void * malloc (size_t size)
{
  ull llsize = (size + 2 * sizeof (ull) - 1) / sizeof (ull);
  static char * envptr;
  void * ret;
  
  /* Make sure getenv works */
  if (!envptr)
    envptr = getenv ("PATH");

  malloc_count++;
  if (current + llsize >= buffer + SIZE) 
    {
      malloc_error++;
      errno = ENOMEM;
      return NULL;
    }
  *current = llsize;
  ret = (void *) (current + 1);
  current += llsize;

  return ret;
}

void * calloc (size_t nmemb, size_t size) 
{
  calloc_count++;
  void * ptr = malloc (nmemb * size);
  malloc_count--;
  if (ptr)
    memset(ptr, 0, nmemb * size);
  return ptr;
}

void * realloc (void * ptr, size_t size)
{
  const ull ullsize = (size + 2 * sizeof (ull) - 1) / sizeof (ull);
  ull * const ullptr = (ull *) ptr;
  void * newptr;
  
  realloc_count++;
  
  if (ptr)
    {
      if (!is_valid (ptr))
	{
	  realloc_error++;
	  errno = ENOMEM;
	  return NULL;
	}  
      if (ullptr[-1] >= ullsize)
	return ptr;
    }

  newptr = malloc (size);
  malloc_count--;
  
  if (ptr && newptr)
    memcpy (newptr, ptr, size);
  
  return newptr;
}

void free (void * x)
{
  free_count++;
  if (x && ! is_valid (x))
      free_error++;
}

