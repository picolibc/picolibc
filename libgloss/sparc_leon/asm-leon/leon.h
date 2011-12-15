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


#ifndef _INCLUDE_LEON_h
#define _INCLUDE_LEON_h

#include <asm-leon/leon3.h>
#include <asm-leon/amba.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* psr defines */
#define SPARC_PSR_WIN_MASK  0x0000001f	/* bit   0-4 */
#define SPARC_PSR_ET_MASK   0x00000020	/* bit   5 */
#define SPARC_PSR_PS_MASK   0x00000040	/* bit   6 */
#define SPARC_PSR_S_MASK    0x00000080	/* bit   7 */
#define SPARC_PSR_PIL_MASK  0x00000F00	/* bits  8 - 11 */
#define SPARC_PSR_EF_MASK   0x00001000	/* bit  12 */
#define SPARC_PSR_EC_MASK   0x00002000	/* bit  13 */
#define SPARC_PSR_ICC_MASK  0x00F00000	/* bits 20 - 23 */
#define SPARC_PSR_VER_MASK  0x0F000000	/* bits 24 - 27 */
#define SPARC_PSR_IMPL_MASK 0xF0000000	/* bits 28 - 31 */
#define SPARC_PSR_PIL_SHIFT 8

#define SPARC_NUM_REGWIN    _nwindows

#ifndef __ASSEMBLER__
  extern int _nwindows;
  extern int _leon_version;
#endif

#define LEON_VERSION _leon_version

/*
 *  Interrupt Sources
 *
 *  The interrupt source numbers directly map to the trap type and to 
 *  the bits used in the Interrupt Clear, Interrupt Force, Interrupt Mask,
 *  and the Interrupt Pending Registers.
 */

#define LEON_INTERRUPT_CORRECTABLE_MEMORY_ERROR  1
#define LEON2_INTERRUPT_UART_2_RX_TX             2
#define LEON2_INTERRUPT_UART_1_RX_TX             3
#define LEON23_INTERRUPT_UART_2_RX_TX            leon23_irqs[1]	/*console.c */
#define LEON23_INTERRUPT_UART_1_RX_TX            leon23_irqs[0]	/*console.c */
#define LEON_INTERRUPT_EXTERNAL_0                4
#define LEON_INTERRUPT_EXTERNAL_1                5
#define LEON_INTERRUPT_EXTERNAL_2                6
#define LEON_INTERRUPT_EXTERNAL_3                7
#define LEON2_INTERRUPT_TIMER1                   8
#define LEON2_INTERRUPT_TIMER2                   9
#define LEON23_INTERRUPT_TIMER1                  leon23_timerirqs[0]	/* timer.c */
#define LEON23_INTERRUPT_TIMER2                  leon23_timerirqs[1]	/* timer.c */
#define LEON_INTERRUPT_EMPTY1                    10
#define LEON_INTERRUPT_EMPTY2                    11
#define LEON_INTERRUPT_EMPTY3                    12
#define LEON_INTERRUPT_EMPTY4                    13
#define LEON_INTERRUPT_EMPTY5                    14
#define LEON_INTERRUPT_EMPTY6                    15

#ifndef  __ASSEMBLER__

/*
 *  Trap Types for on-chip peripherals
 *
 *  Source: Table 8 - Interrupt Trap Type and Default Priority Assignments
 *
 *  NOTE: The priority level for each source corresponds to the least 
 *        significant nibble of the trap type.
 */

#define LEON_TRAP_TYPE( _source ) SPARC_ASYNCHRONOUS_TRAP((_source) + 0x10)

#define LEON_TRAP_SOURCE( _trap ) ((_trap) - 0x10)

#define LEON_INT_TRAP( _trap ) \
  ( (_trap) >= LEON_TRAP_TYPE( LEON_INTERRUPT_CORRECTABLE_MEMORY_ERROR ) && \
    (_trap) <= LEON_TRAP_TYPE( LEON_INTERRUPT_EMPTY6 ) )


#endif


/*
 *  The following defines the bits in Memory Configuration Register 1.
 */

#define LEON_MEMORY_CONFIGURATION_PROM_SIZE_MASK  0x0003C000

/*
 *  The following defines the bits in Memory Configuration Register 1.
 */

#define LEON_MEMORY_CONFIGURATION_RAM_SIZE_MASK  0x00001E00


/*
 *  The following defines the bits in the Timer Control Register.
 */

#define LEON_REG_TIMER_CONTROL_EN    0x00000001	/* 1 = enable counting */
  /* 0 = hold scalar and counter */
#define LEON_REG_TIMER_CONTROL_RL    0x00000002	/* 1 = reload at 0 */
  /* 0 = stop at 0 */
#define LEON_REG_TIMER_CONTROL_LD    0x00000004	/* 1 = load counter */
  /* 0 = no function */

/*
 *  The following defines the bits in the UART Control Registers.
 *
 */

#define LEON_REG_UART_CONTROL_RTD  0x000000FF	/* RX/TX data */

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
 *  The following defines the bits in the LEON UART Status Registers.
 */

#define LEON_REG_UART_CTRL_RE     0x00000001	/* Receiver enable */
#define LEON_REG_UART_CTRL_TE     0x00000002	/* Transmitter enable */
#define LEON_REG_UART_CTRL_RI     0x00000004	/* Receiver interrupt enable */
#define LEON_REG_UART_CTRL_TI     0x00000008	/* Transmitter interrupt enable */
#define LEON_REG_UART_CTRL_PS     0x00000010	/* Parity select */
#define LEON_REG_UART_CTRL_PE     0x00000020	/* Parity enable */
#define LEON_REG_UART_CTRL_FL     0x00000040	/* Flow control enable */
#define LEON_REG_UART_CTRL_LB     0x00000080	/* Loop Back enable */

/* leon2 asis */
#define ASI_LEON2_IFLUSH		0x05
#define ASI_LEON2_DFLUSH		0x06
#define ASI_LEON2_CACHEMISS             1

/* leon3 asis */
#define ASI_LEON3_IFLUSH		0x10
#define ASI_LEON3_DFLUSH		0x11
#define ASI_LEON3_CACHEMISS             1
#define ASI_LEON3_SYSCTRL               0x02

#define ASI_LEON23_ITAG		0x0c
#define ASI_LEON23_DTAG		0x0e


#ifndef  __ASSEMBLER__

  unsigned int leonbare_leon23_loadnocache (unsigned int addr);
  unsigned int leonbare_leon23_loadnocache16 (unsigned int addr);
  unsigned int leonbare_leon23_loadnocache8 (unsigned int addr);
  unsigned int leonbare_leon23_storenocache (unsigned int addr,
					     unsigned int value);
  unsigned int leonbare_leon23_storenocache16 (unsigned int addr,
					       unsigned int value);
  unsigned int leonbare_leon23_storenocache8 (unsigned int addr,
					      unsigned int value);

  unsigned int leonbare_leon3_loadnocache (unsigned int addr);
  unsigned int leonbare_leon3_loadnocache16 (unsigned int addr);
  unsigned int leonbare_leon3_loadnocache8 (unsigned int addr);


/*
 *  This is used to manipulate the on-chip registers.
 *
 *  The following symbol must be defined in the linkcmds file and point
 *  to the correct location.
 */

  extern unsigned long *LEON23_IRQ_mask_addr;	/* in peripherals.h */
  extern unsigned long *LEON23_IRQ_force_addr;	/* in peripherals.h */
  extern unsigned long *LEON23_IRQ_pending_addr;	/* in peripherals.h */
  extern unsigned long *LEON23_IRQ_clear_addr;	/* in peripherals.h */

/*
 *  Macros to manipulate the Interrupt Clear, Interrupt Force, Interrupt Mask,
 *  and the Interrupt Pending Registers.
 *
 *  NOTE: For operations which are not atomic, this code disables interrupts
 *        to guarantee there are no intervening accesses to the same register.
 *        The operations which read the register, modify the value and then
 *        store the result back are vulnerable.
 */

#define LEON_Clear_interrupt( _source ) \
  do { \
    (*LEON23_IRQ_clear_addr) = (1 << (_source)); \
  } while (0)

#define LEON_Force_interrupt( _source ) \
  do { \
    (*LEON23_IRQ_force_addr) = (1 << (_source)); \
  } while (0)

#define LEON_Is_interrupt_masked( _source ) \
  ((*LEON23_IRQ_mask_addr) & (1 << (_source)))

#define LEON_Mask_interrupt( _source ) \
  do { \
    unsigned32 _level; \
    \
    _level = sparc_disable_interrupts(); \
      (*LEON23_IRQ_mask_addr) &= ~(1 << (_source)); \
    sparc_enable_interrupts( _level ); \
  } while (0)

#define LEON_Unmask_interrupt( _source ) \
  do { \
    unsigned32 _level; \
    \
    _level = sparc_disable_interrupts(); \
      (*LEON23_IRQ_mask_addr) |= (1 << (_source)); \
    sparc_enable_interrupts( _level ); \
  } while (0)

#define LEON_Disable_interrupt( _source, _previous ) \
  do { \
    unsigned32 _level; \
    unsigned32 _mask = 1 << (_source); \
    \
    _level = sparc_disable_interrupts(); \
      (_previous) = (*LEON23_IRQ_mask_addr); \
      (*LEON23_IRQ_mask_addr) = _previous & ~_mask; \
    sparc_enable_interrupts( _level ); \
    (_previous) &= _mask; \
  } while (0)

#define LEON_Restore_interrupt( _source, _previous ) \
  do { \
    unsigned32 _level; \
    unsigned32 _mask = 1 << (_source); \
    \
    _level = sparc_disable_interrupts(); \
      (*LEON23_IRQ_mask_addr) = \
        ((*LEON23_IRQ_mask_addr) & ~_mask) | (_previous); \
    sparc_enable_interrupts( _level ); \
  } while (0)

/*
 *  Each timer control register is organized as follows:
 *
 *    D0 - Enable
 *          1 = enable counting
 *          0 = hold scaler and counter
 *
 *    D1 - Counter Reload
 *          1 = reload counter at zero and restart
 *          0 = stop counter at zero
 *
 *    D2 - Counter Load
 *          1 = load counter with preset value 
 *          0 = no function
 *
 */

#define LEON_REG_TIMER_COUNTER_IRQEN              0x00000008

#define LEON_REG_TIMER_COUNTER_RELOAD_AT_ZERO     0x00000002
#define LEON_REG_TIMER_COUNTER_STOP_AT_ZERO       0x00000000

#define LEON_REG_TIMER_COUNTER_LOAD_COUNTER       0x00000004

#define LEON_REG_TIMER_COUNTER_ENABLE_COUNTING    0x00000001
#define LEON_REG_TIMER_COUNTER_DISABLE_COUNTING   0x00000000

#define LEON_REG_TIMER_COUNTER_RELOAD_MASK        0x00000002
#define LEON_REG_TIMER_COUNTER_ENABLE_MASK        0x00000001

#define LEON_REG_TIMER_COUNTER_DEFINED_MASK       0x00000003
#define LEON_REG_TIMER_COUNTER_CURRENT_MODE_MASK  0x00000003

/* console.c */
  int lo_sprintf (char *buf, const char *fmt, ...);

/* do a virtual address read without cache */
  static __inline__ unsigned long leon23_getpsr ()
  {
    unsigned long retval;
    __asm__ __volatile__ ("mov %%psr, %0\n\t":"=r" (retval):);
      return retval;
  }

  extern __inline__ void sparc_leon2_dcache_flush (void)
  {
    __asm__
      __volatile__ ("sta %%g0, [%%g0] %0\n\t"::"i"
		    (ASI_LEON2_IFLUSH):"memory");
    __asm__
      __volatile__ ("sta %%g0, [%%g0] %0\n\t"::"i"
		    (ASI_LEON2_DFLUSH):"memory");
  };


  extern __inline__ void sparc_leon_dcache_flush (void)
  {
    switch (sparc_leon23_get_psr_version ())
      {
      case 0:
      case 2:
	sparc_leon2_dcache_flush ();
	break;
      default:
	sparc_leon3_dcache_flush ();
	break;
      }
  }

  extern int lolevelirqinstall (int irqnr, void (*handler) ());
  extern unsigned long locore_readtbr ();
  extern void _leonbase_Stop ();

  extern void uninstall_winoverflow_hook ();
  extern int install_winoverflow_hook (void (*func) (void));

  extern void sparc_leon23_icache_flush ();
  extern void sparc_leon23_dcache_flush ();

#endif /* ! __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#define TACODE_IRQCALL    2
#define TACODE_IRQCALL_FLUSH 6

#define TACODE_FLUSH      3
#define TACODE_IRQCALLDIS 5



#endif /* !_INCLUDE_LEON_h */
/* end of include file */
