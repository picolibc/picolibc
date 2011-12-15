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


#ifndef _LEON3_AMBA_H__
#define _LEON3_AMBA_H__

#define LEON3_IO_AREA 0xfff00000
#define LEON3_CONF_AREA 0xff000
#define LEON3_AHB_SLAVE_CONF_AREA (1 << 11)

#define LEON3_AHB_CONF_WORDS 8
#define LEON3_APB_CONF_WORDS 2
#define LEON3_AHB_MASTERS 8
#define LEON3_AHB_SLAVES 8
#define LEON3_APB_SLAVES 16
#define LEON3_APBUARTS 8

/* Vendor codes */
#define VENDOR_GAISLER   1
#define VENDOR_PENDER    2
#define VENDOR_ESA       4
#define VENDOR_OPENCORES 8

/* Gaisler Research device id's */
#define GAISLER_LEON3    0x003
#define GAISLER_LEON3DSU 0x004
#define GAISLER_ETHAHB   0x005
#define GAISLER_APBMST   0x006
#define GAISLER_AHBUART  0x007
#define GAISLER_SRCTRL   0x008
#define GAISLER_SDCTRL   0x009
#define GAISLER_APBUART  0x00c
#define GAISLER_IRQMP    0x00d
#define GAISLER_AHBRAM   0x00e
#define GAISLER_GPTIMER  0x011
#define GAISLER_PCITRG   0x012
#define GAISLER_PCISBRG  0x013
#define GAISLER_PCIFBRG  0x014
#define GAISLER_PCITRACE 0x015
#define GAISLER_PCIDMA   0x016
#define GAISLER_AHBTRACE 0x017
#define GAISLER_ETHDSU   0x018
#define GAISLER_PIOPORT  0x01A
#define GAISLER_SPACEWIRE 0x01f

#define GAISLER_ETHMAC       0x01d
#define GAISLER_EHCI         0x026
#define GAISLER_UHCI         0x027

#define GAISLER_L2TIME   0xffd	/* internal device: leon2 timer */
#define GAISLER_L2C      0xffe	/* internal device: leon2compat */
#define GAISLER_PLUGPLAY 0xfff	/* internal device: plug & play configarea */

#ifndef __ASSEMBLER__

extern inline char *
gaisler_device_str (int id)
{
  switch (id)
    {
    case GAISLER_LEON3:
      return "GAISLER_LEON3";
    case GAISLER_LEON3DSU:
      return "GAISLER_LEON3DSU";
    case GAISLER_ETHAHB:
      return "GAISLER_ETHAHB";
    case GAISLER_APBMST:
      return "GAISLER_APBMST";
    case GAISLER_AHBUART:
      return "GAISLER_AHBUART";
    case GAISLER_SRCTRL:
      return "GAISLER_SRCTRL";
    case GAISLER_SDCTRL:
      return "GAISLER_SDCTRL";
    case GAISLER_APBUART:
      return "GAISLER_APBUART";
    case GAISLER_IRQMP:
      return "GAISLER_IRQMP";
    case GAISLER_AHBRAM:
      return "GAISLER_AHBRAM";
    case GAISLER_GPTIMER:
      return "GAISLER_GPTIMER";
    case GAISLER_PCITRG:
      return "GAISLER_PCITRG";
    case GAISLER_PCISBRG:
      return "GAISLER_PCISBRG";
    case GAISLER_PCIFBRG:
      return "GAISLER_PCIFBRG";
    case GAISLER_PCITRACE:
      return "GAISLER_PCITRACE";
    case GAISLER_AHBTRACE:
      return "GAISLER_AHBTRACE";
    case GAISLER_ETHDSU:
      return "GAISLER_ETHDSU";
    case GAISLER_PIOPORT:
      return "GAISLER_PIOPORT";
    case GAISLER_SPACEWIRE:
      return "GAISLER_SPACEWIRE";


    case GAISLER_L2TIME:
      return "GAISLER_L2TIME";
    case GAISLER_L2C:
      return "GAISLER_L2C";
    case GAISLER_PLUGPLAY:
      return "GAISLER_PLUGPLAY";

    default:
      break;
    }
  return 0;
}

#endif

/* European Space Agency device id's */
#define ESA_LEON2        0x002
#define ESA_MCTRL        0x00f

#ifndef __ASSEMBLER__

extern inline char *
esa_device_str (int id)
{
  switch (id)
    {
    case ESA_LEON2:
      return "ESA_LEON2";
    case ESA_MCTRL:
      return "ESA_MCTRL";
    default:
      break;
    }
  return 0;
}

#endif

/* Opencores device id's */
#define OPENCORES_PCIBR  0x4
#define OPENCORES_ETHMAC 0x5

#ifndef __ASSEMBLER__

extern inline char *
opencores_device_str (int id)
{
  switch (id)
    {
    case OPENCORES_PCIBR:
      return "OPENCORES_PCIBR";
    case OPENCORES_ETHMAC:
      return "OPENCORES_ETHMAC";
    default:
      break;
    }
  return 0;
}

extern inline char *
device_id2str (int vendor, int id)
{
  switch (vendor)
    {
    case VENDOR_GAISLER:
      return gaisler_device_str (id);
    case VENDOR_ESA:
      return esa_device_str (id);
    case VENDOR_OPENCORES:
      return opencores_device_str (id);
    case VENDOR_PENDER:
    default:
      break;
    }
  return 0;
}

extern inline char *
vendor_id2str (int vendor)
{
  switch (vendor)
    {
    case VENDOR_GAISLER:
      return "VENDOR_GAISLER";
    case VENDOR_ESA:
      return "VENDOR_ESA";
    case VENDOR_OPENCORES:
      return "VENDOR_OPENCORES";
    case VENDOR_PENDER:
      return "VENDOR_PENDER";
    default:
      break;
    }
  return 0;
}

#endif

/* Vendor codes */

/* 
 *
 * Macros for manipulating Configuration registers  
 *
 */

#define LEON3_BYPASS_LOAD_PA(x) (*((unsigned long*)x))
#define LEON3_BYPASS_STORE_PA(x,v) (*((unsigned long*)x) = (v))

#define amba_get_confword(tab, index, word) (*((tab).addr[(index)]+(word)))

#define amba_vendor(x) (((x) >> 24) & 0xff)

#define amba_device(x) (((x) >> 12) & 0xfff)

#define amba_ahb_get_membar(tab, index, nr) (*((tab).addr[(index)]+4+(nr)))

#define amba_apb_get_membar(tab, index) (*((tab).addr[(index)]+1))

#define amba_membar_start(mbar) (((mbar) & 0xfff00000) & (((mbar) & 0xfff0) << 16))

#define amba_iobar_start(base, iobar) ((base) | ((((iobar) & 0xfff00000)>>12) & (((iobar) & 0xfff0)<<4)) )

#define amba_irq(conf) ((conf) & 0xf)

#define amba_membar_type(mbar) ((mbar) & 0xf)

#define AMBA_TYPE_APBIO 0x1
#define AMBA_TYPE_MEM   0x2
#define AMBA_TYPE_AHBIO 0x3

#define AMBA_TYPE_AHBIO_ADDR(addr) (LEON3_IO_AREA | ((addr) >> 12))






#ifndef __ASSEMBLER__

/*
 *  The following defines the bits in the LEON UART Status Registers.
 */

#define LEON_REG_UART_STATUS_DR   0x00000001	/* Data Ready */
#define LEON_REG_UART_STATUS_TSE  0x00000002	/* TX Send Register Empty */
#define LEON_REG_UART_STATUS_THE  0x00000004	/* TX Hold Register Empty */
#define LEON_REG_UART_STATUS_BR   0x00000008	/* Break Error */
#define LEON_REG_UART_STATUS_OE   0x00000010	/* RX Overrun Error */
#define LEON_REG_UART_STATUS_PE   0x00000020	/* RX Parity Error */
#define LEON_REG_UART_STATUS_FE   0x00000040	/* RX Framing Error */
#define LEON_REG_UART_STATUS_ERR  0x00000078	/* Error Mask */

/*
 *  The following defines the bits in the LEON UART Ctrl Registers.
 */

#define LEON_REG_UART_CTRL_RE     0x00000001	/* Receiver enable */
#define LEON_REG_UART_CTRL_TE     0x00000002	/* Transmitter enable */
#define LEON_REG_UART_CTRL_RI     0x00000004	/* Receiver interrupt enable */
#define LEON_REG_UART_CTRL_TI     0x00000008	/* Transmitter interrupt enable */
#define LEON_REG_UART_CTRL_PS     0x00000010	/* Parity select */
#define LEON_REG_UART_CTRL_PE     0x00000020	/* Parity enable */
#define LEON_REG_UART_CTRL_FL     0x00000040	/* Flow control enable */
#define LEON_REG_UART_CTRL_LB     0x00000080	/* Loop Back enable */

#define LEON3_GPTIMER_EN 1
#define LEON3_GPTIMER_RL 2
#define LEON3_GPTIMER_LD 4
#define LEON3_GPTIMER_IRQEN 8
#define LEON3_GPTIMER_IP 0x10

#define LEON3_GPTIMER_CONFIG_TIMERMASK 0x7
#define LEON3_GPTIMER_CONFIG_SEPERATE (1<<8)

typedef struct
{
  volatile unsigned int ilevel;
  volatile unsigned int ipend;
  volatile unsigned int iforce;
  volatile unsigned int iclear;
  volatile unsigned int notused00;
  volatile unsigned int notused01;
  volatile unsigned int notused02;
  volatile unsigned int notused03;
  volatile unsigned int notused10;
  volatile unsigned int notused11;
  volatile unsigned int notused12;
  volatile unsigned int notused13;
  volatile unsigned int notused20;
  volatile unsigned int notused21;
  volatile unsigned int notused22;
  volatile unsigned int notused23;
  volatile unsigned int mask[16];
} LEON3_IrqCtrl_Regs_Map;
extern volatile LEON3_IrqCtrl_Regs_Map *LEON3_IrqCtrl_Regs;	/* in amba.c */

typedef struct
{
  volatile unsigned int data;
  volatile unsigned int status;
  volatile unsigned int ctrl;
  volatile unsigned int scaler;
} LEON23_APBUART_Regs_Map;
extern volatile LEON23_APBUART_Regs_Map *leon23_uarts[2];	/* in console.c */
extern unsigned int leon23_irqs[2];	/* in console.c */

typedef struct
{
  volatile unsigned int val;
  volatile unsigned int rld;
  volatile unsigned int ctrl;
  volatile unsigned int unused;
} LEON3_GpTimerElem_Regs_Map;


typedef struct
{
  volatile unsigned int scalar;
  volatile unsigned int scalar_reload;
  volatile unsigned int config;
  volatile unsigned int unused;
  volatile LEON3_GpTimerElem_Regs_Map e[8];
} LEON3_GpTimer_Regs_Map;
#define LEON3_GPTIMER_CONFIG_NRTIMERS(c) ((c)->config & 0x7)
int Timer_getTimer1 (unsigned int **count, unsigned int **reload, unsigned int **ctrl);	/* in timer.c */
int Timer_getTimer2 (unsigned int **count, unsigned int **reload, unsigned int **ctrl);	/* in timer.c */
extern volatile LEON3_GpTimer_Regs_Map *LEON3_GpTimer_Regs;
extern unsigned long LEON3_GpTimer_Irq;

typedef struct
{
  volatile unsigned int iodata;
  volatile unsigned int ioout;
  volatile unsigned int iodir;
  volatile unsigned int irqmask;
  volatile unsigned int irqpol;
  volatile unsigned int irqedge;
} LEON3_IOPORT_Regs_Map;


/*
 *  Types and structure used for AMBA Plug & Play bus scanning 
 */
extern int amba_init_done;

#define AMBA_MAXAPB_DEVS 64
#define AMBA_MAXAPB_DEVS_PERBUS 16

typedef struct amba_device_table
{
  int devnr;			/* numbrer of devices on AHB or APB bus */
  unsigned int *addr[16];	/* addresses to the devices configuration tables */
  unsigned int allocbits[1];	/* 0=unallocated, 1=allocated driver */
} amba_device_table;

typedef struct amba_apbslv_device_table
{
  int devnr;			/* number of devices on AHB or APB bus */
  unsigned int *addr[AMBA_MAXAPB_DEVS];	/* addresses to the devices configuration tables */
  unsigned int apbmst[AMBA_MAXAPB_DEVS];	/* apb master if a entry is a apb slave */
  unsigned int apbmstidx[AMBA_MAXAPB_DEVS];	/* apb master idx if a entry is a apb slave */
  unsigned int allocbits[4];	/* 0=unallocated, 1=allocated driver */
} amba_apbslv_device_table;

typedef struct amba_confarea_type
{
  amba_device_table ahbmst;
  amba_device_table ahbslv;
  amba_apbslv_device_table apbslv;
  /*unsigned int apbmst; */
} amba_confarea_type;


extern unsigned long amba_find_apbslv_addr (unsigned long vendor,
					    unsigned long device,
					    unsigned long *irq);

// collect apb slaves
typedef struct amba_apb_device
{
  unsigned int start, irq;
} amba_apb_device;
extern int amba_get_free_apbslv_devices (int vendor, int device,
					 amba_apb_device * dev, int nr);

// collect ahb slaves
typedef struct amba_ahb_device
{
  unsigned int start[4], irq;
} amba_ahb_device;
extern int amba_get_free_ahbslv_devices (int vendor, int device,
					 amba_ahb_device * dev, int nr);


/*amba_scan.c*/
unsigned int leon3_getapbbase (register unsigned int vendor,
			       register unsigned int driver,
			       amba_apb_device * apbdevs, int c);

#endif //!__ASSEMBLER__





#endif
