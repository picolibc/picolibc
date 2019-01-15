/*
 * Support file for amdgcn in newlib.
 * Copyright (c) 2014, 2017 Mentor Graphics.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* This struct must match the one used by gcn-run and libgomp.
   It holds all the data output from a kernel (besides mapping data).
 
   The base address pointer can be found at kernargs+16.
 
   The next_output counter must be atomically incremented for each
   print output.  Only when the print data is fully written can the
   "written" flag be set.  */
struct output {
  int return_value;
  int next_output;
  struct printf_data {
    int written;
    char msg[128];
    int type;
    union {
      int64_t ivalue;
      double dvalue;
      char text[128];
    };
  } queue[1000];
};

_READ_WRITE_RETURN_TYPE write (int fd, const void *buf, size_t count)
{
  if (fd != 1 && fd != 2)
    {
      errno = EBADF;
      return -1;
    }

  /* The output data is at ((void*)kernargs)[2].  */
  register void **kernargs asm("s8");
  struct output *data = (struct output *)kernargs[2];

  /* Each output slot allows 256 bytes, so reserve as many as we need. */
  int slot_count = ((count+1)/256)+1;
  int index = __atomic_fetch_add (&data->next_output, slot_count,
				  __ATOMIC_ACQUIRE);
  for (int c = count;
       c >= 0 && index < 1000;
       buf += 256, c -= 256, index++)
    {
      if (c < 128)
	{
	  memcpy (data->queue[index].msg, buf, c);
	  data->queue[index].msg[c] = '\0';
	  data->queue[index].text[0] = '\0';
	}
      else if (c < 256)
	{
	  memcpy (data->queue[index].msg, buf, 128);
	  memcpy (data->queue[index].text, buf+128, c-128);
	  data->queue[index].text[c-128] = '\0';
	}
      else
	{
	  memcpy (data->queue[index].msg, buf, 128);
	  memcpy (data->queue[index].text, buf+128, 128);
	}

      data->queue[index].type = 3; /* Raw.  */
      __atomic_store_n (&data->queue[index].written, 1, __ATOMIC_RELEASE);
    }

  return count;
}
