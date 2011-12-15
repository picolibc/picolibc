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


#ifndef _LAMBAPP_H
#define _LAMBAPP_H


/* Include VENDOR and DEVICE definitions */
#include "lambapp_devs.h"

#ifdef __cplusplus
extern "C"
{
#endif

  struct ambapp_dev_hdr;
  struct ambapp_apb_info;
  struct ambapp_ahb_info;

  struct ambapp_dev_hdr
  {
    struct ambapp_dev_hdr *next;	/* Next */
    struct ambapp_dev_hdr *prev;	/* Previous Device. If (this == prev->child) prev is bus bridge */
    struct ambapp_dev_hdr *children;	/* Points to first device on sub-bus */
    void *owner;		/* Owner of this AMBA device */
    unsigned char dev_type;	/* AHB MST, AHB SLV or APB SLV */
    unsigned char vendor;	/* Vendor ID */
    unsigned short device;	/* Device ID */
    void *devinfo;		/* Device info (APB or AHB depending on type) */
  };

#define AMBAPP_FLAG_FFACT_DIR	0x100	/* Frequency factor direction, 0=down, 1=up */
#define AMBAPP_FLAG_FFACT	0x0f0	/* Frequency factor against top bus */
#define AMBAPP_FLAG_MBUS	0x00c
#define AMBAPP_FLAG_SBUS	0x003

  struct ambapp_apb_info
  {
    /* COMMON */
    unsigned char irq;
    unsigned char ver;

    /* APB SPECIFIC */
    unsigned int start;
    unsigned int mask;
  };

  struct ambapp_ahb_info
  {
    /* COMMON */
    unsigned char irq;
    unsigned char ver;

    /* AHB SPECIFIC */
    unsigned int start[4];
    unsigned int mask[4];
    char type[4];		/* type[N] Determine type of start[N]-mask[N], 2=AHB Memory Space, 3=AHB I/O Space */
    unsigned int custom[3];
  };

/* Describes a complete AMBA Core. Each device may consist of 3 interfaces */
  struct ambapp_dev_info
  {
    char irq;			/* irq=-1 indicate no IRQ */
    unsigned char vendor;
    unsigned short device;
    int index;			/* Core index if multiple "subcores" in one */
    struct ambapp_ahb_info *ahb_mst;
    struct ambapp_ahb_info *ahb_slv;
    struct ambapp_apb_info *apb_slv;
  };

  struct ambapp_mmap
  {
    unsigned int size;
    unsigned int local_adr;
    unsigned int remote_adr;
  };

/* Complete AMBA PnP information */
  struct ambapp_bus
  {
    struct ambapp_mmap *mmaps;
    struct ambapp_dev_hdr *root;
  };

/* 
 * Return values
 *  0 - continue
 *  1 - stop scanning
 */
  typedef int (*ambapp_func_t) (struct ambapp_dev_hdr * dev, int index,
				int maxdepth, void *arg);

#define DEV_IS_FREE(dev) (dev->owner == NULL)
#define DEV_IS_ALLOCATED(dev) (dev->owner != NULL)

/* Options to ambapp_for_each */
#define OPTIONS_AHB_MSTS	0x00000001
#define OPTIONS_AHB_SLVS	0x00000002
#define OPTIONS_APB_SLVS	0x00000004
#define OPTIONS_ALL_DEVS	(OPTIONS_AHB_MSTS|OPTIONS_AHB_SLVS|OPTIONS_APB_SLVS)

#define OPTIONS_FREE		0x00000010
#define OPTIONS_ALLOCATED	0x00000020
#define OPTIONS_ALL		(OPTIONS_FREE|OPTIONS_ALLOCATED)

/* Depth first search, Defualt is breth first search. */
#define OPTIONS_DEPTH_FIRST	0x00000100

#define DEV_AHB_NONE 0
#define DEV_AHB_MST  1
#define DEV_AHB_SLV  2
#define DEV_APB_SLV 3

/* Structures used to access Plug&Play information directly */
  struct ambapp_pnp_ahb
  {
    const unsigned int id;	/* VENDOR, DEVICE, VER, IRQ, */
    const unsigned int custom[3];
    const unsigned int mbar[4];	/* MASK, ADDRESS, TYPE, CACHABLE/PREFETCHABLE */
  };

  struct ambapp_pnp_apb
  {
    const unsigned int id;	/* VENDOR, DEVICE, VER, IRQ, */
    const unsigned int iobar;	/* MASK, ADDRESS, TYPE, CACHABLE/PREFETCHABLE */
  };

#define ambapp_pnp_vendor(id) (((id) >> 24) & 0xff)
#define ambapp_pnp_device(id) (((id) >> 12) & 0xfff)
#define ambapp_pnp_ver(id) (((id)>>5) & 0x1f)
#define ambapp_pnp_irq(id) ((id) & 0x1f)

#define ambapp_pnp_start(mbar)  (((mbar) & 0xfff00000) & (((mbar) & 0xfff0) << 16))
#define ambapp_pnp_mbar_mask(mbar) (((mbar)>>4) & 0xfff)
#define ambapp_pnp_mbar_type(mbar) ((mbar) & 0xf)

#define ambapp_pnp_apb_start(iobar, base) ((base) | ((((iobar) & 0xfff00000)>>12) & (((iobar) & 0xfff0)<<4)) )
#define ambapp_pnp_apb_mask(iobar) ((~(ambapp_pnp_mbar_mask(iobar)<<8) & 0x000fffff) + 1)

#define AMBA_TYPE_AHBIO_ADDR(addr,base_ioarea) ((unsigned int)(base_ioarea) | ((addr) >> 12))

#define AMBA_TYPE_APBIO 0x1
#define AMBA_TYPE_MEM   0x2
#define AMBA_TYPE_AHBIO 0x3

  extern int find_apbslv (int vendor, int device,
			  struct ambapp_apb_info *dev);

#ifdef __cplusplus
}
#endif

#endif
