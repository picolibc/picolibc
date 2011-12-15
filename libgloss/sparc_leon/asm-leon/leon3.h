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


#ifndef _INCLUDE_LEON3_h
#define _INCLUDE_LEON3_h

#ifndef __ASSEMBLER__

#ifdef __cplusplus
extern "C"
{
#endif

#define ASI_LEON3_CACHEMISS 1
#define ASI_LEON3_SYSCTRL   0x02
#define ASI_LEON3_DFLUSH    0x11

#define ASI_LEON3_SYSCTRL_ICFG		0x08
#define ASI_LEON3_SYSCTRL_DCFG		0x0c
#define ASI_LEON3_SYSCTRL_CFG_SNOOPING (1<<27)
#define ASI_LEON3_SYSCTRL_CFG_SSIZE(c) (1<<((c>>20)&0xf))


  extern __inline__ unsigned long sparc_leon23_get_psr (void)
  {
    unsigned int retval;
    __asm__ __volatile__ ("rd %%psr, %0\n\t":"=r" (retval):);
      return (retval);
  }

  extern __inline__ unsigned long sparc_leon23_get_psr_version (void)
  {
    unsigned int psr = sparc_leon23_get_psr ();
    return (psr >> 24) & 0xf;
  }
#define LEON_ISLEON2 (sparc_leon23_get_psr_version() == 2 || sparc_leon23_get_psr_version() == 0)
#define LEON_ISLEON3 (sparc_leon23_get_psr_version() == 3)

  extern __inline__ unsigned long sparc_leon3_get_dcachecfg (void)
  {
    unsigned int retval;
    __asm__
      __volatile__ ("lda [%1] %2, %0\n\t":"=r" (retval):"r"
		    (ASI_LEON3_SYSCTRL_DCFG), "i" (ASI_LEON3_SYSCTRL));
    return (retval);
  }

  extern __inline__ void sparc_leon3_enable_snooping (void)
  {
    /*enable snooping */
    __asm__ volatile ("lda [%%g0] 2, %%l1\n\t"
		      "set 0x800000, %%l2\n\t"
		      "or  %%l2, %%l1, %%l2\n\t"
		      "sta %%l2, [%%g0] 2\n\t":::"l1", "l2");
  };

  extern __inline__ void sparc_leon3_disable_cache (void)
  {
    /*asi 2 */
    __asm__ volatile ("lda [%%g0] 2, %%l1\n\t"
		      "set 0x00000f, %%l2\n\t"
		      "andn  %%l2, %%l1, %%l2\n\t"
		      "sta %%l2, [%%g0] 2\n\t":::"l1", "l2");
  };



  extern __inline__ void sparc_leon3_dcache_flush (void)
  {
    __asm__ __volatile__ (" flush ");	//iflush 
    __asm__
      __volatile__ ("sta %%g0, [%%g0] %0\n\t"::"i"
		    (ASI_LEON3_DFLUSH):"memory");
  };


#ifdef __cplusplus
}
#endif

#endif /* !ASM */


#endif /* !_INCLUDE_LEON3_h */
/* end of include file */
