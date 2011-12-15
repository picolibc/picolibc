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


#include <asm-leon/amba.h>
#undef AMBA_TYPE_AHBIO_ADDR
#include <asm-leon/lambapp.h>
#include <string.h>

#define AMBA_CONF_AREA 0xff000
#define AMBA_AHB_SLAVE_CONF_AREA (1 << 11)
#define AMBA_APB_SLAVES 16

#ifdef PDEBUG
#define DPRINTF(p)  printf p
#else
#define DPRINTF(p)
#endif

unsigned int
ambapp_addr_from (struct ambapp_mmap *mmaps, unsigned int address)
{
  /* no translation? */
  if (!mmaps)
    return address;

  while (mmaps->size)
    {
      if ((address >= mmaps->remote_adr)
	  && (address <= (mmaps->remote_adr + (mmaps->size - 1))))
	{
	  return (address - mmaps->remote_adr) + mmaps->local_adr;
	}
      mmaps++;
    }
  return 1;
}


static void
ambapp_ahb_dev_init (unsigned int ioarea,
		     struct ambapp_mmap *mmaps,
		     struct ambapp_pnp_ahb *ahb, struct ambapp_dev_hdr *dev)
{
  int bar;
  struct ambapp_ahb_info *ahb_info;
  unsigned int addr, mask, mbar;

  /* Setup device struct */
  dev->vendor = ambapp_pnp_vendor (ahb->id);
  dev->device = ambapp_pnp_device (ahb->id);
  ahb_info = dev->devinfo;
  ahb_info->ver = ambapp_pnp_ver (ahb->id);
  ahb_info->irq = ambapp_pnp_irq (ahb->id);
  ahb_info->custom[0] = (unsigned int) ahb->custom[0];
  ahb_info->custom[1] = (unsigned int) ahb->custom[1];
  ahb_info->custom[2] = (unsigned int) ahb->custom[2];

  DPRINTF (("+AHB device %d:%d\n", dev->device, dev->vendor));

  /* Memory BARs */
  for (bar = 0; bar < 4; bar++)
    {
      mbar = ahb->mbar[bar];
      if (mbar == 0)
	{
	  addr = 0;
	  mask = 0;
	}
      else
	{
	  addr = ambapp_pnp_start (mbar);
	  if (ambapp_pnp_mbar_type (mbar) == AMBA_TYPE_AHBIO)
	    {
	      /* AHB I/O area is releative IO_AREA */
	      addr = AMBA_TYPE_AHBIO_ADDR (addr, ioarea);
	      mask =
		(((unsigned int) (ambapp_pnp_mbar_mask ((~mbar)) << 8) |
		  0xff)) + 1;
	    }
	  else
	    {
	      /* AHB memory area, absolute address */
	      addr = ambapp_addr_from (mmaps, addr);
	      mask =
		(~((unsigned int) (ambapp_pnp_mbar_mask (mbar) << 20))) + 1;
	    }
	}
      ahb_info->start[bar] = addr;
      ahb_info->mask[bar] = mask;
    }
}

static void
ambapp_apb_dev_init (unsigned int base,
		     struct ambapp_mmap *mmaps,
		     struct ambapp_pnp_apb *apb, struct ambapp_dev_hdr *dev)
{
  struct ambapp_apb_info *apb_info;

  /* Setup device struct */
  dev->vendor = ambapp_pnp_vendor (apb->id);
  dev->device = ambapp_pnp_device (apb->id);
  apb_info = dev->devinfo;
  apb_info->ver = ambapp_pnp_ver (apb->id);
  apb_info->irq = ambapp_pnp_irq (apb->id);
  apb_info->start = ambapp_pnp_apb_start (apb->iobar, base);
  apb_info->mask = ambapp_pnp_apb_mask (apb->iobar);

  DPRINTF (("+APB device %d:%d\n", dev->device, dev->vendor));


}

#define MAX_NUM_BUSES 16
static void
ambapp_add_scanned_bus (unsigned int *ioareas, unsigned int ioarea)
{
  int i;
  for (i = 0; i < MAX_NUM_BUSES; i++)
    {
      if (ioareas[i] == 0)
	{
	  ioareas[i] = ioarea;
	  return;
	}
    }
}

static int
ambapp_has_been_scanned (unsigned int *ioareas, unsigned int ioarea)
{
  int i;
  if (!ioareas)
    return 0;

  for (i = 0; i < MAX_NUM_BUSES; i++)
    {
      if (ioareas[i] == 0)
	{
	  break;
	}
      else if (ioareas[i] == ioarea)
	{
	  return 1;
	}
    }
  return 0;
}

static int
ambapp_find (unsigned int ioarea,
	     struct ambapp_dev_hdr *parent,
	     struct ambapp_mmap *mmaps,
	     void *internal,
	     int (*find_match) (struct ambapp_dev_hdr * dev, void *arg),
	     void *arg, int vendor, int device)
{
  struct ambapp_pnp_ahb *ahb, ahb_buf;
  struct ambapp_pnp_apb *apb, apb_buf;
  struct ambapp_dev_hdr *dev, *prev, *prevapb, *apbdev;
  struct ambapp_ahb_info *ahb_info;
  int maxloops = 64;
  unsigned int apbbase, bridge_address;
  int i, j;

  DPRINTF (("Scan at 0x%08x\n", ioarea));

  if (parent)
    {
      /* scan first bus for 64 devices, rest for 16 devices */
      maxloops = 16;
    }
  else
    {
      if (internal)
	{
	  ambapp_add_scanned_bus (internal, ioarea);
	}
    }

  prev = parent;

  /* AHB MASTERS */
  ahb = (struct ambapp_pnp_ahb *) (ioarea | AMBA_CONF_AREA);
  for (i = 0; i < maxloops; i++)
    {
      memcpy (&ahb_buf, ahb, sizeof (struct ambapp_pnp_ahb));
      if (ahb_buf.id != 0)
	{
	  struct ambapp_dev_hdr _dev;
	  struct ambapp_ahb_info _ahb;
	  memset (&_dev, 0, sizeof (_dev));
	  memset (&_ahb, 0, sizeof (_ahb));
	  _dev.devinfo = &_ahb;
	  _dev.dev_type = DEV_AHB_MST;
	  dev = &_dev;

	  ambapp_ahb_dev_init (ioarea, mmaps, &ahb_buf, dev);

	  DPRINTF ((" = test %d:%d == %d:%d\n", vendor, device, dev->vendor,
		    dev->device));

	  if (vendor == dev->vendor &&
	      device == dev->device && find_match (dev, arg))
	    {
	      return 1;
	    }
	}
      ahb++;
    }


  /* AHB SLAVES */
  ahb =
    (struct ambapp_pnp_ahb *) (ioarea | AMBA_CONF_AREA |
			       AMBA_AHB_SLAVE_CONF_AREA);
  for (i = 0; i < maxloops; i++)
    {
      memcpy (&ahb_buf, ahb, sizeof (struct ambapp_pnp_ahb));
      if (ahb_buf.id != 0)
	{
	  struct ambapp_dev_hdr _dev;
	  struct ambapp_ahb_info _ahb;
	  memset (&_dev, 0, sizeof (_dev));
	  memset (&_ahb, 0, sizeof (_ahb));
	  _dev.devinfo = &_ahb;
	  _dev.dev_type = DEV_AHB_MST;
	  dev = &_dev;

	  ambapp_ahb_dev_init (ioarea, mmaps, &ahb_buf, dev);

	  DPRINTF ((" = test %d:%d == %d:%d\n", vendor, device, dev->vendor,
		    dev->device));

	  if (vendor == dev->vendor &&
	      device == dev->device && find_match (dev, arg))
	    {
	      return 1;
	    }

	  /* Is it a AHB/AHB Bridge ? */
	  if ((dev->device == GAISLER_AHB2AHB)
	      && (dev->vendor == VENDOR_GAISLER))
	    {
	      /* AHB/AHB Bridge Found, recurse down the Bridge */
	      ahb_info = dev->devinfo;
	      if (ahb_info->ver)
		{
		  bridge_address =
		    ambapp_addr_from (mmaps, ahb_info->custom[1]);

		  DPRINTF (("+(AHBAHB:0x%x)\n", bridge_address));

		  /* Makes sure bus only scanned once */
		  if (internal == 0
		      || ambapp_has_been_scanned (internal,
						  bridge_address) == NULL)
		    {
		      if (internal)
			ambapp_add_scanned_bus (internal, bridge_address);

		      if (ambapp_find (bridge_address, dev, mmaps, internal,
				       find_match, arg, vendor, device))
			return 1;
		    }
		}
	    }
	  else if ((dev->device == GAISLER_APBMST)
		   && (dev->vendor == VENDOR_GAISLER))
	    {
	      /* AHB/APB Bridge Found, add the APB devices to this AHB Slave's children */
	      prevapb = dev;
	      ahb_info = dev->devinfo;
	      apbbase = ahb_info->start[0];
	      apb = (struct ambapp_pnp_apb *) (apbbase | AMBA_CONF_AREA);
	      for (j = 0; j < AMBA_APB_SLAVES; j++)
		{
		  memcpy (&apb_buf, apb, sizeof (struct ambapp_pnp_apb));
		  if (apb_buf.id)
		    {
		      struct ambapp_dev_hdr _apbdev;
		      struct ambapp_apb_info _apb;
		      memset (&_apbdev, 0, sizeof (_apbdev));
		      memset (&_apb, 0, sizeof (_apb));
		      _apbdev.devinfo = &_apb;
		      _apbdev.dev_type = DEV_APB_SLV;
		      apbdev = &_apbdev;

		      ambapp_apb_dev_init (apbbase, mmaps, &apb_buf, apbdev);

		      DPRINTF ((" = test %d:%d == %d:%d\n", vendor, device,
				apbdev->vendor, apbdev->device));

		      if (vendor == apbdev->vendor &&
			  device == apbdev->device &&
			  find_match (apbdev, arg))
			{

			  return 1;
			}
		    }
		  apb++;
		}
	    }
	}
      ahb++;
    }

  if (parent == NULL)
    {
      /*free(internal); */
    }

  return 0;
}

struct ambapp_dev_find_match_arg
{
  int index;
  int count;
  int type;
  void *dev;
};

/* AMBA PP find routines */
static int
ambapp_dev_find_match (struct ambapp_dev_hdr *dev, void *arg)
{
  struct ambapp_dev_find_match_arg *p = arg;

  if (p->index == 0)
    {
      /* Found controller, stop */
      if (p->type == DEV_APB_SLV)
	{
	  *(struct ambapp_apb_info *) p->dev =
	    *(struct ambapp_apb_info *) dev->devinfo;
	  p->dev = ((struct ambapp_apb_info *) p->dev) + 1;
	}
      else
	{
	  *(struct ambapp_ahb_info *) p->dev =
	    *(struct ambapp_ahb_info *) dev->devinfo;
	  p->dev = ((struct ambapp_ahb_info *) p->dev) + 1;
	}
      p->count--;
      if (p->count < 1)
	return 1;
    }
  else
    {
      p->index--;
    }
  return 0;
}

static int
find_apbslvs_next (int vendor, int device, struct ambapp_apb_info *dev,
		   int index, int maxno)
{
  struct ambapp_dev_find_match_arg arg;
  unsigned int busses[MAX_NUM_BUSES];
  memset (busses, 0, sizeof (busses));

  arg.index = index;
  arg.count = maxno;
  arg.type = DEV_APB_SLV;	/* APB */
  arg.dev = dev;

  ambapp_find (LEON3_IO_AREA, NULL, NULL, &busses,
	       ambapp_dev_find_match, &arg, vendor, device);

  return maxno - arg.count;
}

int
find_apbslv (int vendor, int device, struct ambapp_apb_info *dev)
{
  return find_apbslvs_next (vendor, device, dev, 0, 1);
}

struct ambapp_dev_hdr *ambapp_root = NULL;
unsigned int busses[MAX_NUM_BUSES];
extern unsigned int console;
extern unsigned int rtc;
extern unsigned int irqmp;

void
pnpinit (void)
{
  struct ambapp_apb_info dev;
  int n;
  if ((n = find_apbslv (VENDOR_GAISLER, GAISLER_APBUART, &dev)) == 1)
    {
      console = dev.start;
      DPRINTF (("Found abuart at 0x%x\n", console));
    }
  if ((n = find_apbslv (VENDOR_GAISLER, GAISLER_GPTIMER, &dev)) == 1)
    {
      rtc = dev.start + 0x10;
      DPRINTF (("Found rtc at 0x%x\n", rtc));
    }
  if ((n = find_apbslv (VENDOR_GAISLER, GAISLER_IRQMP, &dev)) == 1)
    {
      irqmp = dev.start;
      DPRINTF (("Found irqmp at 0x%x\n", rtc));
    }
}
