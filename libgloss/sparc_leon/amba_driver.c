/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <asm-leon/leon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*#define DEBUG_CONFIG*/

/* Structure containing address to devices found on the Amba Plug&Play bus */
extern amba_confarea_type amba_conf;

/*collect apb slaves*/
int
amba_get_free_apbslv_devices (int vendor, int device, amba_apb_device * dev,
			      int nr)
{
  unsigned int i, conf, iobar, j = 0;
#ifdef DEBUG_CONFIG
  printf ("Apbslv: search for apdslv devices\n");
#endif
  for (i = 0; i < amba_conf.apbslv.devnr && j < nr; i++)
    {
      conf = amba_get_confword (amba_conf.apbslv, i, 0);
#ifdef DEBUG_CONFIG
      printf ("Apbslv: check(%x:%x)==(%x:%x)\n", vendor, device,
	      amba_vendor (conf), amba_device (conf));
#endif
      if ((amba_vendor (conf) == vendor) && (amba_device (conf) == device))
	{
	  if (!(amba_conf.apbslv.allocbits[i / 32] & (1 << (i & (32 - 1)))))
	    {
#ifdef DEBUG_CONFIG
	      printf ("Apbslv: alloc device idx %i (%x:%x)\n",
		      j, vendor, device);
#endif
	      amba_conf.apbslv.allocbits[i / 32] |= (1 << (i & (32 - 1)));
	      dev[j].irq = amba_irq (conf);
	      iobar = amba_apb_get_membar (amba_conf.apbslv, i);
	      dev[j].start =
		amba_iobar_start (amba_conf.apbslv.apbmst[i], iobar);
#ifdef DEBUG_CONFIG
	      printf (" +bar: 0x%x \n", k, dev[j].start);
#endif
	      j++;
	    }
	}
    }
  return j;
}

/*collect ahb slaves*/
int
amba_get_free_ahbslv_devices (int vendor, int device, amba_ahb_device * dev,
			      int nr)
{
  unsigned int addr, i, conf, iobar, j = 0, k;
#ifdef DEBUG_CONFIG
  printf ("Ahbslv: search for ahdslv devices\n");
#endif
  for (i = 0; i < amba_conf.ahbslv.devnr && j < nr; i++)
    {
      conf = amba_get_confword (amba_conf.ahbslv, i, 0);
#ifdef DEBUG_CONFIG
      printf ("Ahbslv: check(%x:%x)==(%x:%x)\n", vendor, device,
	      amba_vendor (conf), amba_device (conf));
#endif
      if ((amba_vendor (conf) == vendor) && (amba_device (conf) == device))
	{
	  if (!(amba_conf.ahbslv.allocbits[i / 32] & (1 << (i & (32 - 1)))))
	    {
#ifdef DEBUG_CONFIG
	      printf ("Ahbslv: alloc device idx %i (%x:%x)\n",
		      j, vendor, device);
#endif
	      amba_conf.ahbslv.allocbits[i / 32] |= (1 << (i & (32 - 1)));
	      dev[j].irq = amba_irq (conf);
	      for (k = 0; k < 4; k++)
		{
		  iobar = amba_ahb_get_membar (amba_conf.ahbslv, i, k);
		  addr = amba_membar_start (iobar);
		  if (amba_membar_type (iobar) == AMBA_TYPE_AHBIO)
		    {
		      addr = AMBA_TYPE_AHBIO_ADDR (addr);
		    }
		  dev[j].start[k] = addr;
#ifdef DEBUG_CONFIG
		  printf (" +%i: 0x%x \n", k, dev[j].start[k]);
#endif
		}
	      j++;
	    }
	}
    }
  return j;
}
