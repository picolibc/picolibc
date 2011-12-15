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

#define DPRINTF(p)  printf p

/* Allocate */
struct ambapp_dev_hdr *
ambapp_alloc_dev_struct (int dev_type)
{
  int size = sizeof (struct ambapp_dev_hdr);
  struct ambapp_dev_hdr *dev;

  if (dev_type == DEV_APB_SLV)
    {
      size += sizeof (struct ambapp_apb_info);
    }
  else
    {
      /* AHB */
      size += sizeof (struct ambapp_ahb_info);
    }
  dev = malloc (size);
  if (dev == NULL)
    return NULL;
  memset (dev, 0, size);
  dev->devinfo = (void *) (dev + 1);
  dev->dev_type = dev_type;
  return dev;
}

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

void
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

void
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
void
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

int
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

int
ambapp_scan (unsigned int ioarea,
	     struct ambapp_dev_hdr *parent,
	     struct ambapp_mmap *mmaps,
	     void *(*memfunc) (void *dest, const void *src, int n),
	     struct ambapp_dev_hdr **root, void *internal)
{
  struct ambapp_pnp_ahb *ahb, ahb_buf;
  struct ambapp_pnp_apb *apb, apb_buf;
  struct ambapp_dev_hdr *dev, *prev, *prevapb, *apbdev;
  struct ambapp_ahb_info *ahb_info;
  int maxloops = 64;
  unsigned int apbbase, bridge_address;
  int i, j;

  DPRINTF (("Scan at 0x%08x\n", ioarea));

  /* Default to memcpy() */
  if (!memfunc)
    memfunc = (void *(*)(void *dest, const void *src, int n)) memcpy;

  *root = NULL;

  if (parent)
    {
      /* scan first bus for 64 devices, rest for 16 devices */
      maxloops = 16;
    }
  else
    {
      DPRINTF (("+(malloc:"));
      internal = malloc (sizeof (unsigned int) * MAX_NUM_BUSES);
      DPRINTF (("0x%x)\n", internal));

      if (!internal)
	return -1;
      memset (internal, 0, sizeof (unsigned int) * MAX_NUM_BUSES);

      ambapp_add_scanned_bus (internal, ioarea);
    }

  prev = parent;

  /* AHB MASTERS */
  ahb = (struct ambapp_pnp_ahb *) (ioarea | AMBA_CONF_AREA);
  for (i = 0; i < maxloops; i++)
    {
      memfunc (&ahb_buf, ahb, sizeof (struct ambapp_pnp_ahb));
      if (ahb_buf.id != 0)
	{
	  /* A AHB device present here */
	  dev = ambapp_alloc_dev_struct (DEV_AHB_MST);
	  if (!dev)
	    return -1;

	  ambapp_ahb_dev_init (ioarea, mmaps, &ahb_buf, dev);

	  if (*root == NULL)
	    *root = dev;

	  if (prev != parent)
	    prev->next = dev;
	  dev->prev = prev;
	  prev = dev;
	}
      ahb++;
    }

  /* AHB SLAVES */
  ahb =
    (struct ambapp_pnp_ahb *) (ioarea | AMBA_CONF_AREA |
			       AMBA_AHB_SLAVE_CONF_AREA);
  for (i = 0; i < maxloops; i++)
    {
      memfunc (&ahb_buf, ahb, sizeof (struct ambapp_pnp_ahb));
      if (ahb_buf.id != 0)
	{
	  /* A AHB device present here */
	  dev = ambapp_alloc_dev_struct (DEV_AHB_SLV);
	  if (!dev)
	    return -1;

	  ambapp_ahb_dev_init (ioarea, mmaps, &ahb_buf, dev);

	  if (prev != parent)
	    prev->next = dev;
	  dev->prev = prev;
	  prev = dev;

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
		  if (ambapp_has_been_scanned (internal, bridge_address) ==
		      NULL)
		    {
		      ambapp_add_scanned_bus (internal, bridge_address);
		      if (ambapp_scan
			  (bridge_address, dev, mmaps, memfunc,
			   &dev->children, internal))
			return -1;
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
		  memfunc (&apb_buf, apb, sizeof (struct ambapp_pnp_apb));
		  if (apb_buf.id)
		    {
		      apbdev = ambapp_alloc_dev_struct (DEV_APB_SLV);
		      if (!dev)
			return -1;

		      ambapp_apb_dev_init (apbbase, mmaps, &apb_buf, apbdev);

		      if (prevapb != dev)
			prevapb->next = apbdev;
		      else
			dev->children = apbdev;
		      apbdev->prev = prevapb;
		      prevapb = apbdev;
		    }
		  apb++;
		}
	    }
	}
      ahb++;
    }

  if (parent == NULL)
    {
      free (internal);
    }

  return 0;
}

/* Match search options againt device */
int
ambapp_dev_match_options (struct ambapp_dev_hdr *dev, unsigned int options,
			  int vendor, int device)
{
  if ((((options & (OPTIONS_ALL_DEVS)) == OPTIONS_ALL_DEVS) ||	/* Match TYPE */
       ((options & OPTIONS_AHB_MSTS) && (dev->dev_type == DEV_AHB_MST)) || ((options & OPTIONS_AHB_SLVS) && (dev->dev_type == DEV_AHB_SLV)) || ((options & OPTIONS_APB_SLVS) && (dev->dev_type == DEV_APB_SLV))) && ((vendor == -1) || (vendor == dev->vendor)) &&	/* Match ID */
      ((device == -1) || (device == dev->device)) && (((options & OPTIONS_ALL) == OPTIONS_ALL) ||	/* Match Allocated State */
						      ((options &
							OPTIONS_FREE)
						       && DEV_IS_FREE (dev))
						      ||
						      ((options &
							OPTIONS_ALLOCATED)
						       &&
						       DEV_IS_ALLOCATED
						       (dev))))
    {
      return 1;
    }
  return 0;
}

/* If device is an APB bridge all devices on the APB bridge is processed */
static int
ambapp_for_each_apb (struct ambapp_dev_hdr *dev,
		     unsigned int options,
		     int vendor,
		     int device, int maxdepth, ambapp_func_t func, void *arg)
{
  int index;
  struct ambapp_dev_hdr *apbslv;

  if (maxdepth < 0)
    return 0;

  if (dev->children && (dev->children->dev_type == DEV_APB_SLV))
    {
      /* Found a APB Bridge */
      index = 0;
      apbslv = dev->children;
      while (apbslv)
	{
	  if (ambapp_dev_match_options (apbslv, options, vendor, device) == 1)
	    {
	      if (func (apbslv, index, maxdepth, arg) == 1)
		return 1;	/* Signalled stopped */
	    }
	  index++;
	  apbslv = apbslv->next;
	}
    }
  return 0;
}

/* Traverse the prescanned device information */
int
ambapp_for_each (struct ambapp_dev_hdr *root,
		 unsigned int options,
		 int vendor,
		 int device, int maxdepth, ambapp_func_t func, void *arg)
{
  struct ambapp_dev_hdr *dev;
  int ahb_slave = 0;
  int index;

  if (maxdepth < 0)
    return 0;

  /* Start at device 'root' and process downwards.
   *
   * Breadth first search, search order
   * 1. AHB MSTS
   * 2. AHB SLVS
   * 3. APB SLVS on primary bus
   * 4. AHB/AHB secondary... -> step to 1.
   */

  /* AHB MST / AHB SLV */
  if (options & (OPTIONS_AHB_MSTS | OPTIONS_AHB_SLVS | OPTIONS_DEPTH_FIRST))
    {
      index = 0;
      dev = root;
      while (dev)
	{
	  if ((dev->dev_type == DEV_AHB_SLV) && !ahb_slave)
	    {
	      /* First AHB Slave */
	      ahb_slave = 1;
	      index = 0;
	    }

	  /* Conditions must be fullfilled for function to be called */
	  if (ambapp_dev_match_options (dev, options, vendor, device) == 1)
	    {
	      /* Correct device and vendor ID */
	      if (func (dev, index, maxdepth, arg) == 1)
		return 1;	/* Signalled stopped */
	    }

	  if ((options & OPTIONS_DEPTH_FIRST) && (options & OPTIONS_APB_SLVS))
	    {
	      /* Check is APB bridge, and process all APB Slaves in that case */
	      if (ambapp_for_each_apb
		  (dev, options, vendor, device, (maxdepth - 1), func,
		   arg) == 1)
		return 1;	/* Signalled stopped */
	    }

	  if (options & OPTIONS_DEPTH_FIRST)
	    {
	      if (dev->children && (dev->children->dev_type != DEV_APB_SLV))
		{
		  /* Found AHB Bridge, recurse */
		  if (ambapp_for_each
		      (dev->children, options, vendor, device, (maxdepth - 1),
		       func, arg) == 1)
		    return 1;
		}
	    }

	  index++;
	  dev = dev->next;
	}
    }

  /* Find APB Bridges */
  if ((options & OPTIONS_APB_SLVS) && !(options & OPTIONS_DEPTH_FIRST))
    {
      dev = root;
      while (dev)
	{
	  /* Check is APB bridge, and process all APB Slaves in that case */
	  if (ambapp_for_each_apb
	      (dev, options, vendor, device, (maxdepth - 1), func, arg) == 1)
	    return 1;		/* Signalled stopped */
	  dev = dev->next;
	}
    }

  /* Find AHB Bridges */
  if (!(options & OPTIONS_DEPTH_FIRST))
    {
      dev = root;
      while (dev)
	{
	  if (dev->children && (dev->children->dev_type != DEV_APB_SLV))
	    {
	      /* Found AHB Bridge, recurse */
	      if (ambapp_for_each
		  (dev->children, options, vendor, device, (maxdepth - 1),
		   func, arg) == 1)
		return 1;
	    }
	  dev = dev->next;
	}
    }

  return 0;
}

int
ambapp_alloc_dev (struct ambapp_dev_hdr *dev, void *owner)
{
  if (dev->owner)
    return -1;
  dev->owner = owner;
  return 0;
}

void
ambapp_free_dev (struct ambapp_dev_hdr *dev)
{
  dev->owner = NULL;
}

struct ambapp_dev_find_match_arg
{
  int index;
  int count;
  int type;
  void *dev;
};

/* AMBA PP find routines */
int
ambapp_dev_find_match (struct ambapp_dev_hdr *dev, int index, int maxdepth,
		       void *arg)
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

int
ambapp_find_apbslvs_next (struct ambapp_dev_hdr *root, int vendor, int device,
			  struct ambapp_apb_info *dev, int index, int maxno)
{
  struct ambapp_dev_find_match_arg arg;

  arg.index = index;
  arg.count = maxno;
  arg.type = DEV_APB_SLV;	/* APB */
  arg.dev = dev;

  ambapp_for_each (root, (OPTIONS_ALL | OPTIONS_APB_SLVS), vendor, device, 10,
		   ambapp_dev_find_match, &arg);

  return maxno - arg.count;
}

int
ambapp_find_apbslv (struct ambapp_dev_hdr *root, int vendor, int device,
		    struct ambapp_apb_info *dev)
{
  return ambapp_find_apbslvs_next (root, vendor, device, dev, 0, 1);
}

int
ambapp_find_apbslv_next (struct ambapp_dev_hdr *root, int vendor, int device,
			 struct ambapp_apb_info *dev, int index)
{
  return ambapp_find_apbslvs_next (root, vendor, device, dev, index, 1);
}

int
ambapp_find_apbslvs (struct ambapp_dev_hdr *root, int vendor, int device,
		     struct ambapp_apb_info *dev, int maxno)
{
  return ambapp_find_apbslvs_next (root, vendor, device, dev, 0, maxno);
}

int
ambapp_find_ahbslvs_next (struct ambapp_dev_hdr *root, int vendor, int device,
			  struct ambapp_ahb_info *dev, int index, int maxno)
{
  struct ambapp_dev_find_match_arg arg;

  arg.index = index;
  arg.count = maxno;
  arg.type = DEV_AHB_SLV;	/* AHB SLV */
  arg.dev = dev;

  ambapp_for_each (root, (OPTIONS_ALL | OPTIONS_AHB_SLVS), vendor, device, 10,
		   ambapp_dev_find_match, &arg);

  return maxno - arg.count;
}

int
ambapp_find_ahbslv_next (struct ambapp_dev_hdr *root, int vendor, int device,
			 struct ambapp_ahb_info *dev, int index)
{
  return ambapp_find_ahbslvs_next (root, vendor, device, dev, index, 1);
}

int
ambapp_find_ahbslv (struct ambapp_dev_hdr *root, int vendor, int device,
		    struct ambapp_ahb_info *dev)
{
  return ambapp_find_ahbslvs_next (root, vendor, device, dev, 0, 1);
}

int
ambapp_find_ahbslvs (struct ambapp_dev_hdr *root, int vendor, int device,
		     struct ambapp_ahb_info *dev, int maxno)
{
  return ambapp_find_ahbslvs_next (root, vendor, device, dev, 0, maxno);
}

struct ambapp_dev_hdr *
ambapp_find_parent (struct ambapp_dev_hdr *dev)
{
  while (dev->prev)
    {
      if (dev == dev->prev->children)
	{
	  return dev->prev;
	}
      dev = dev->prev;
    }
  return NULL;
}


struct ambapp_dev_hdr *ambapp_root = NULL;
extern unsigned int console;
extern unsigned int rtc;

void
pnpinit (void)
{
  struct ambapp_apb_info dev;
  int n;
  ambapp_scan (LEON3_IO_AREA, NULL, NULL, NULL, &ambapp_root, NULL);
  if ((n =
       ambapp_find_apbslv (ambapp_root, VENDOR_GAISLER, GAISLER_APBUART,
			   &dev)) == 1)
    {
      console = dev.start;
      DPRINTF (("Found abuart at 0x%x\n", console));
    }
  if ((n =
       ambapp_find_apbslv (ambapp_root, VENDOR_GAISLER, GAISLER_GPTIMER,
			   &dev)) == 1)
    {
      rtc = dev.start + 0x10;
      DPRINTF (("Found rtc at 0x%x\n", rtc));
    }
}
