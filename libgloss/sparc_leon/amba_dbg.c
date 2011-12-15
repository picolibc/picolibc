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

#ifdef DEBUG_CONFIG
#define printk(fmt,arg...) \
{ char c[1024]; \
  sprintf(c,fmt,##arg); \
  DEBUG_puts(c); \
}
#else
#define printk(fmt,arg...)
#endif

static void
vendor_dev_string (unsigned long conf, char *vendorbuf, char *devbuf)
{
  int vendor = amba_vendor (conf);
  int dev = amba_device (conf);
  char *devstr;
  char *vendorstr;
#ifdef DEBUG_CONFIG
  sprintf (vendorbuf, "Unknown vendor %2x", vendor);
  sprintf (devbuf, "Unknown device %2x", dev);
  vendorstr = vendor_id2str (vendor);
  if (vendorstr)
    {
      sprintf (vendorbuf, "%s", vendorstr);
    }
  devstr = device_id2str (vendor, dev);
  if (devstr)
    {
      sprintf (devbuf, "%s", devstr);
    }
#else
  vendorbuf[0] = 0;
  devbuf[0] = 0;
#endif
}

void
amba_prinf_config (void)
{
  char devbuf[128];
  char vendorbuf[128];
  unsigned int conf;
  int i = 0;
  int j = 0;
  unsigned int addr;
  unsigned int m;
  printk ("             Vendors         Slaves\n");
  printk ("Ahb masters:\n");
  i = 0;
  while (i < amba_conf.ahbmst.devnr)
    {
      conf = amba_get_confword (amba_conf.ahbmst, i, 0);
      vendor_dev_string (conf, vendorbuf, devbuf);
      printk ("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor (conf),
	      amba_device (conf), amba_irq (conf), vendorbuf, devbuf);
      for (j = 0; j < 4; j++)
	{
	  m = amba_ahb_get_membar (amba_conf.ahbmst, i, j);
	  if (m)
	    {
	      addr = amba_membar_start (m);
	      printk (" +%i: 0x%x \n", j, addr);
	    }
	}
      i++;
    }
  printk ("Ahb slaves:\n");
  i = 0;
  while (i < amba_conf.ahbslv.devnr)
    {
      conf = amba_get_confword (amba_conf.ahbslv, i, 0);
      vendor_dev_string (conf, vendorbuf, devbuf);
      printk ("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor (conf),
	      amba_device (conf), amba_irq (conf), vendorbuf, devbuf);
      for (j = 0; j < 4; j++)
	{
	  m = amba_ahb_get_membar (amba_conf.ahbslv, i, j);
	  if (m)
	    {
	      addr = amba_membar_start (m);
	      if (amba_membar_type (m) == AMBA_TYPE_AHBIO)
		{
		  addr = AMBA_TYPE_AHBIO_ADDR (addr);
		}
	      else if (amba_membar_type (m) == AMBA_TYPE_APBIO)
		{
		  printk ("Warning: apbio membar\n");
		}
	      printk (" +%i: 0x%x (raw:0x%x)\n", j, addr, m);
	    }
	}
      i++;
    }
  printk ("Apb slaves:\n");
  i = 0;
  while (i < amba_conf.apbslv.devnr)
    {

      conf = amba_get_confword (amba_conf.apbslv, i, 0);
      vendor_dev_string (conf, vendorbuf, devbuf);
      printk ("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor (conf),
	      amba_device (conf), amba_irq (conf), vendorbuf, devbuf);

      m = amba_apb_get_membar (amba_conf.apbslv, i);
      addr = amba_iobar_start (amba_conf.apbslv.apbmst[i], m);
      printk (" +%2i: 0x%x (raw:0x%x) \n", 0, addr, m);

      i++;

    }

}
