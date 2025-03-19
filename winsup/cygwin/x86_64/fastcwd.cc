/* x86_64/fastcwd.cc: find fast cwd pointer on x86_64 hosts.

  This file is part of Cygwin.

  This software is a copyrighted work licensed under the terms of the
  Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
  details. */

#include "winsup.h"
#include <assert.h>
#include "udis86/types.h"
#include "udis86/extern.h"

class fcwd_access_t;

/* Helper function to get the absolute address of an rip-relative instruction
   by summing the current instruction's pc (rip), the current instruction's
   length, and the signed 32-bit displacement in the operand.  Optionally, an
   additional offset is subtracted to deal with the case where a member of a
   struct is being referenced by the instruction but the address of the struct
   is desired.
*/
static inline const void *
rip_rel_offset (const ud_t *ud_obj, const ud_operand_t *opr, int sub_off=0)
{
  assert ((opr->type == UD_OP_JIMM && opr->size == 32) ||
	  (opr->type == UD_OP_MEM && opr->base == UD_R_RIP &&
	   opr->index == UD_NONE && opr->scale == 0 && opr->offset == 32));

  return (const void *) (ud_insn_off (ud_obj) + ud_insn_len (ud_obj) +
			 opr->lval.sdword - sub_off);
}

/* This function scans the code in ntdll.dll to find the address of the
   global variable used to access the CWD.  While the pointer is global,
   it's not exported from the DLL, unfortunately.  Therefore we have to
   use some knowledge to figure out the address. */

fcwd_access_t **
find_fast_cwd_pointer_x86_64 ()
{
  /* Fetch entry points of relevant functions in ntdll.dll. */
  HMODULE ntdll = GetModuleHandle ("ntdll.dll");
  if (!ntdll)
    return NULL;
  const uint8_t *get_dir = (const uint8_t *)
			   GetProcAddress (ntdll, "RtlGetCurrentDirectory_U");
  const uint8_t *ent_crit = (const uint8_t *)
			    GetProcAddress (ntdll, "RtlEnterCriticalSection");
  if (!get_dir || !ent_crit)
    return NULL;
  /* Initialize udis86 */
  ud_t ud_obj;
  ud_init (&ud_obj);
  /* Set 64-bit mode */
  ud_set_mode (&ud_obj, 64);
  ud_set_input_buffer (&ud_obj, get_dir, 80);
  /* Set pc (rip) so that subsequent calls to ud_insn_off will return the pc of
     the instruction, saving us the hassle of tracking it ourselves */
  ud_set_pc (&ud_obj, (uint64_t) get_dir);
  const ud_operand_t *opr, *opr0;
  ud_mnemonic_code_t insn;
  ud_type_t reg = UD_NONE;
  /* Search first relative call instruction in RtlGetCurrentDirectory_U. */
  const uint8_t *use_cwd = NULL;
  while (ud_disassemble (&ud_obj) &&
      (insn = ud_insn_mnemonic (&ud_obj)) != UD_Iret &&
      insn != UD_Ijmp)
    {
      if (insn == UD_Icall)
	{
	  opr = ud_insn_opr (&ud_obj, 0);
	  if (opr->type == UD_OP_JIMM && opr->size == 32)
	    {
	      /* Fetch offset from instruction and compute address of called
		 function.  This function actually fetches the current FAST_CWD
		 instance and performs some other actions, not important to us.
	       */
	      use_cwd = (const uint8_t *) rip_rel_offset (&ud_obj, opr);
	      break;
	    }
	}
    }
  if (!use_cwd)
    return NULL;
  ud_set_input_buffer (&ud_obj, use_cwd, 120);
  ud_set_pc (&ud_obj, (uint64_t) use_cwd);

  /* Next we search for the locking mechanism and perform a sanity check.
     we basically look for the RtlEnterCriticalSection call and test if the
     code uses the FastPebLock. */
  PRTL_CRITICAL_SECTION lockaddr = NULL;

  while (ud_disassemble (&ud_obj) &&
      (insn = ud_insn_mnemonic (&ud_obj)) != UD_Iret &&
      insn != UD_Ijmp)
    {
      if (insn == UD_Ilea)
	{
	  /* udis86 seems to follow intel syntax, in that operand 0 is the
	     dest and 1 is the src */
	  opr0 = ud_insn_opr (&ud_obj, 0);
	  opr = ud_insn_opr (&ud_obj, 1);
	  if (opr->type == UD_OP_MEM && opr->base == UD_R_RIP &&
	      opr->index == UD_NONE && opr->scale == 0 && opr->offset == 32 &&
	      opr0->type == UD_OP_REG && opr0->size == 64)
	    {
	      lockaddr = (PRTL_CRITICAL_SECTION) rip_rel_offset (&ud_obj, opr);
	      reg = opr0->base;
	      break;
	    }
	}
    }

  /* Test if lock address is FastPebLock. */
  if (lockaddr != NtCurrentTeb ()->Peb->FastPebLock)
    return NULL;

  /* Find where the lock address is loaded into rcx as the first parameter of
     a function call */
  bool found = false;
  if (reg != UD_R_RCX)
    {
      while (ud_disassemble (&ud_obj) &&
	  (insn = ud_insn_mnemonic (&ud_obj)) != UD_Iret &&
	  insn != UD_Ijmp)
	{
	  if (insn == UD_Imov)
	    {
	      opr0 = ud_insn_opr (&ud_obj, 0);
	      opr = ud_insn_opr (&ud_obj, 1);
	      if (opr->type == UD_OP_REG && opr->size == 64 &&
		  opr->base == reg && opr0->type == UD_OP_REG &&
		  opr0->size == 64 && opr0->base == UD_R_RCX)
		{
		  found = true;
		  break;
		}
	    }
	}
      if (!found)
	return NULL;
    }

  /* Next is the `callq RtlEnterCriticalSection' */
  found = false;
  while (ud_disassemble (&ud_obj) &&
      (insn = ud_insn_mnemonic (&ud_obj)) != UD_Iret &&
      insn != UD_Ijmp)
    {
      if (insn == UD_Icall)
	{
	  opr = ud_insn_opr (&ud_obj, 0);
	  if (opr->type == UD_OP_JIMM && opr->size == 32)
	    {
	      if (ent_crit != rip_rel_offset (&ud_obj, opr))
		return NULL;
	      found = true;
	      break;
	    }
	}
    }
  if (!found)
    return NULL;

  fcwd_access_t **f_cwd_ptr = NULL;
  /* now we're looking for a mov rel(%rip), %<reg64> */
  while (ud_disassemble (&ud_obj) &&
      (insn = ud_insn_mnemonic (&ud_obj)) != UD_Iret &&
      insn != UD_Ijmp)
    {
      if (insn == UD_Imov)
	{
	  opr0 = ud_insn_opr (&ud_obj, 0);
	  opr = ud_insn_opr (&ud_obj, 1);
	  if (opr->type == UD_OP_MEM && opr->size == 64 &&
	      opr->base == UD_R_RIP && opr->index == UD_NONE &&
	      opr->scale == 0 && opr->offset == 32 &&
	      opr0->type == UD_OP_REG && opr0->size == 64)
	    {
	      f_cwd_ptr = (fcwd_access_t **) rip_rel_offset (&ud_obj, opr);
	      reg = opr0->base;
	      break;
	    }
	}
    }
  /* Check that the next instruction is a test. */
  if (!f_cwd_ptr || !ud_disassemble (&ud_obj) ||
      ud_insn_mnemonic (&ud_obj) != UD_Itest)
    return NULL;

  /* ... and that it's testing the same register that the mov above loaded the
     f_cwd_ptr into against itself */
  opr0 = ud_insn_opr (&ud_obj, 0);
  opr = ud_insn_opr (&ud_obj, 1);
  if (opr->type != UD_OP_REG || opr->size != 64 || opr->base != reg ||
      opr0->type != opr->type || opr0->size != 64 || opr0->base != opr->base)
    return NULL;
  return f_cwd_ptr;
}
