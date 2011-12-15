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

unsigned int
leon3_ahbslv_scan (register unsigned int vendor, register unsigned int driver)
{
  register unsigned int conf, i, *confp;
  register unsigned int cfg_area =
    (unsigned int) (LEON3_IO_AREA | LEON3_CONF_AREA |
		    LEON3_AHB_SLAVE_CONF_AREA);
  for (i = 0; i < LEON3_AHB_SLAVES; i++)
    {
      confp = (unsigned int *) (cfg_area + (i * LEON3_AHB_CONF_WORDS * 4));
      conf = *confp;
      if ((amba_vendor (conf) == vendor) && (amba_device (conf) == driver))
	{
	  return (unsigned int) confp;
	}
    }
  return 0;
}

unsigned int
leon3_getbase (register unsigned int *mbar, register unsigned int iobase,
	       int *irq)
{
  register unsigned int conf = mbar[1];
  return (unsigned int) (((iobase & 0xfff00000) |
			  ((conf & 0xfff00000) >> 12)) & (((conf & 0x0000fff0)
							   << 4) |
							  0xfff00000));
}

unsigned int
leon3_apbslv_scan (register unsigned int base,
		   register unsigned int vendor,
		   register unsigned int driver,
		   amba_apb_device * apbdevs, int c)
{
  register unsigned int conf, i, *confp;
  int j = 0;
  for (i = 0; i < LEON3_APB_SLAVES; i++)
    {
      confp = (unsigned int *) (base + (i * LEON3_APB_CONF_WORDS * 4));
      conf = *confp;
      if ((amba_vendor (conf) == vendor) && (amba_device (conf) == driver))
	{
	  if (j < c)
	    {
	      apbdevs[j].start = leon3_getbase (confp, base, 0);
	      apbdevs[j].irq = amba_irq (conf);
	      j++;
	    }
	}
    }
  return j;
}


unsigned int
leon3_getapbbase (register unsigned int vendor,
		  register unsigned int driver,
		  amba_apb_device * apbdevs, int c)
{
  unsigned int apb = leon3_ahbslv_scan (VENDOR_GAISLER, GAISLER_APBMST);
  apb = (*(unsigned int *) (apb + 16)) & LEON3_IO_AREA;
  apb |= LEON3_CONF_AREA;
  return leon3_apbslv_scan (apb, vendor, driver, apbdevs, c);
}
