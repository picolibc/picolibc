/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include <ieeefp.h>

#ifdef __riscv_flen
static void
fssr(unsigned value)
{
  asm volatile ("fscsr %0" :: "r"(value));
}

static unsigned
frsr()
{
  unsigned value;
  asm volatile ("frcsr %0" : "=r" (value));
  return value;
}

static fp_rnd
frm_fp_rnd (unsigned frm)
{
  switch (frm)
    {
    case 0: return FP_RN;
    case 1: return FP_RZ;
    case 2: return FP_RM;
    case 3: return FP_RP;
    /* 4 ~ 7 is invalid value, so just retun FP_RP.  */
    default:return FP_RP;
    }
}

static fp_except
frm_fp_except (unsigned except)
{
  fp_except fp = 0;
  if (except & (1 << 0))
    fp |= FP_X_IMP;
  if (except & (1 << 1))
    fp |= FP_X_UFL;
  if (except & (1 << 2))
    fp |= FP_X_OFL;
  if (except & (1 << 3))
    fp |= FP_X_DX;
  if (except & (1 << 4))
    fp |= FP_X_INV;
  return fp;
}

static unsigned
frm_except(fp_except fp)
{
  unsigned except = 0;
  if (fp & FP_X_IMP)
    except |= (1 << 0);
  if (fp & FP_X_UFL)
    except |= (1 << 1);
  if (fp & FP_X_OFL)
    except |= (1 << 2);
  if (fp & FP_X_DX)
    except |= (1 << 3);
  if (fp & FP_X_INV)
    except |= (1 << 4);
  return except;
}

#endif /* __riscv_flen */

fp_except
fpgetmask(void)
{
  return 0;
}

fp_rnd
fpgetround(void)
{
#ifdef __riscv_flen
  unsigned rm = (frsr () >> 5) & 0x7;
  return frm_fp_rnd (rm);
#else
  return FP_RZ;
#endif /* __riscv_flen */
}

fp_except
fpgetsticky(void)
{
#ifdef __riscv_flen
  return frm_fp_except(frsr ());
#else
  return 0;
#endif /* __riscv_flen */
}

fp_except
fpsetmask(fp_except mask)
{
  return -1;
}

fp_rnd
fpsetround(fp_rnd rnd_dir)
{
#ifdef __riscv_flen
  unsigned fsr = frsr ();
  unsigned rm = (fsr >> 5) & 0x7;
  unsigned new_rm;
  switch (rnd_dir)
    {
    case FP_RN: new_rm = 0; break;
    case FP_RZ: new_rm = 1; break;
    case FP_RM: new_rm = 2; break;
    case FP_RP: new_rm = 3; break;
    default:    return -1;
    }
  fssr (new_rm << 5 | fsr & 0x1f);
  return frm_fp_rnd (rm);
#else
  return -1;
#endif /* __riscv_flen */
}

fp_except
fpsetsticky(fp_except sticky)
{
#ifdef __riscv_flen
  unsigned fsr = frsr ();
  fssr (frm_except(sticky) | (fsr & ~0x1f));
  return frm_fp_except(fsr);
#else
  return -1;
#endif /* __riscv_flen */
}
