/* aarch64/fastcwd.cc: find the fast cwd pointer on aarch64 hosts.

  This file is part of Cygwin.

  This software is a copyrighted work licensed under the terms of the
  Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
  details. */

/* You might well wonder why this file is included in x86_64 target files
   in Makefile.am.  It turns out that this code works when built for i686,
   x86_64, or aarch64 with just the small #if/#elif block in
   GetArm64ProcAddress below caring which. */

#include "winsup.h"
#include <assert.h>

class fcwd_access_t;

static LPCVOID
GetArm64ProcAddress (HMODULE hModule, LPCSTR procname)
{
  const BYTE *proc = (const BYTE *) GetProcAddress (hModule, procname);
#if defined (__aarch64__)
  return proc;
#else
#if defined (__i386__)
  static const BYTE thunk[] = "\x8b\xff\x55\x8b\xec\x5d\x90\xe9";
  static const BYTE thunk2[0];
#elif defined (__x86_64__)
  /* see
     https://learn.microsoft.com/en-us/windows/arm/arm64ec-abi#fast-forward-sequences */
  static const BYTE thunk[] = "\x48\x8b\xc4\x48\x89\x58\x20\x55\x5d\xe9";
  /* on windows 11 22000 the thunk is different than documented on that page */
  static const BYTE thunk2[] = "\x48\x8b\xff\x55\x48\x8b\xec\x5d\x90\xe9";
#else
#error "Unhandled architecture for thunk detection"
#endif
  if (memcmp (proc, thunk, sizeof (thunk) - 1) == 0 ||
     (sizeof(thunk2) && memcmp (proc, thunk2, sizeof (thunk2) - 1) == 0))
    {
      proc += sizeof (thunk) - 1;
      proc += 4 + *(const int32_t *) proc;
    }
  return proc;
#endif
}

/* these ids and masks, as well as the names of the various other parts of
   instructions used in this file, came from
   https://developer.arm.com/documentation/ddi0602/2024-09/Index-by-Encoding
   (Arm A-profile A64 Instruction Set Architecture)
*/
#define IS_INSN(pc, name) ((*(pc) & name##_mask) == name##_id)
static const uint32_t add_id = 0x11000000;
static const uint32_t add_mask = 0x7fc00000;
static const uint32_t adrp_id = 0x90000000;
static const uint32_t adrp_mask = 0x9f000000;
static const uint32_t b_id = 0x14000000;
static const uint32_t b_mask = 0xfc000000;
static const uint32_t bl_id = 0x94000000;
static const uint32_t bl_mask = 0xfc000000;
/* matches both cbz and cbnz */
static const uint32_t cbz_id = 0x34000000;
static const uint32_t cbz_mask = 0x7e000000;
static const uint32_t ldr_id = 0xb9400000;
static const uint32_t ldr_mask = 0xbfc00000;
/* matches both ret and br (which are the same except ret is a 'hint' that
   it's  a subroutine return */
static const uint32_t ret_id = 0xd61f0000;
static const uint32_t ret_mask = 0xffbffc1f;

/* this would work for either bl or b, but we only use it for bl */
static inline LPCVOID
extract_bl_target (const uint32_t *pc)
{
  assert (IS_INSN (pc, bl) || IS_INSN (pc, b));
  int32_t offset = *pc & ~bl_mask;
  /* sign extend */
  if (offset & (1 << 25))
    offset |= bl_mask;
  /* Note uint32_t * artithmatic will implicitly multiply the offset by 4 */
  return pc + offset;
}

static inline uint64_t
extract_adrp_address (const uint32_t *pc)
{
  assert (IS_INSN (pc, adrp));
  uint64_t adrp_base = (uint64_t) pc & ~0xFFF;
  int64_t  adrp_imm = (*pc >> (5+19+5)) & 0x3;
  adrp_imm |= ((*pc >> 5) & 0x7FFFF) << 2;
  /* sign extend */
  if (adrp_imm & (1 << 20))
    adrp_imm |= ~((1 << 21) - 1);
  adrp_imm <<= 12;
  return adrp_base + adrp_imm;
}

/* This function scans the code in ntdll.dll to find the address of the
   global variable used to access the CWD.  While the pointer is global,
   it's not exported from the DLL, unfortunately.  Therefore we have to
   use some knowledge to figure out the address. */

fcwd_access_t **
find_fast_cwd_pointer_aarch64 ()
{
  /* Fetch entry points of relevant functions in ntdll.dll. */
  HMODULE ntdll = GetModuleHandle ("ntdll.dll");
  if (!ntdll)
    return NULL;
  LPCVOID get_dir = GetArm64ProcAddress (ntdll, "RtlGetCurrentDirectory_U");
  LPCVOID ent_crit = GetArm64ProcAddress (ntdll, "RtlEnterCriticalSection");
  if (!get_dir || !ent_crit)
    return NULL;

  LPCVOID use_cwd = NULL;
  const uint32_t *start = (const uint32_t *) get_dir;
  const uint32_t *pc = start;
  /* find the call to RtlpReferenceCurrentDirectory, and get its address */
  for (; pc < start + 20 && !IS_INSN (pc, ret) && !IS_INSN (pc, b); pc++)
    {
      if (IS_INSN (pc, bl))
	{
	  use_cwd = extract_bl_target (pc);
	  break;
	}
    }
  if (!use_cwd)
    return NULL;

  start = pc = (const uint32_t *) use_cwd;

  const uint32_t *ldrpc = NULL;
  uint32_t ldroffset, ldrsz;
  uint32_t ldrrn, ldrrd;

  /* find the ldr (immediate unsigned offset) for RtlpCurDirRef */
  for (; pc < start + 20 && !IS_INSN (pc, ret) && !IS_INSN (pc, b); pc++)
    {
      if (IS_INSN (pc, ldr))
	{
	  ldrpc = pc;
	  ldrsz = (*pc & 0x40000000);
	  ldroffset = (*pc >> (5+5)) & 0xFFF;
	  ldroffset <<= ldrsz ? 3 : 2;
	  ldrrn = (*pc >> 5) & 0x1F;
	  ldrrd = *pc & 0x1F;
	  break;
	}
    }
  if (ldrpc == NULL)
    return NULL;

  /* the next instruction after the ldr should be checking if it was NULL:
     either a compare and branch if zero or not zero (hence why cbz_mask is 7e
     instead of 7f) */
  if (!IS_INSN (pc + 1, cbz) || (*(pc + 1) & 0x1F) != ldrrd
      || (*(pc + 1) & 0x80000000) != (ldrsz << 1))
    return NULL;

  /* work backwards, find a bl to RtlEnterCriticalSection whose argument
     is the fast peb lock */

  for (pc = ldrpc; pc >= start; pc--)
    {
      if (IS_INSN (pc, bl) && extract_bl_target (pc) == ent_crit)
	break;
    }
  uint32_t addoffset;
  uint32_t addrn;
  for (; pc >= start; pc--)
    {
      if (IS_INSN (pc, add) && (*pc & 0x1F) == 0)
	{
	  addoffset = (*pc >> (5+5)) & 0xFFF;
	  addrn = (*pc >> 5) & 0x1F;
	  break;
	}
    }
  PRTL_CRITICAL_SECTION lockaddr = NULL;
  for (; pc >= start; pc--)
    {
      if (IS_INSN (pc, adrp) && (*pc & 0x1F) == addrn)
	{
	  lockaddr = (PRTL_CRITICAL_SECTION) (extract_adrp_address (pc) +
					      addoffset);
	  break;
	}
    }
  if (lockaddr != NtCurrentTeb ()->Peb->FastPebLock)
    return NULL;

  /* work backwards from the ldr to find the corresponding adrp */
  fcwd_access_t **RtlpCurDirRef = NULL;
  for (pc = ldrpc; pc >= start; pc--)
    {
      if (IS_INSN (pc, adrp) && (*pc & 0x1F) == ldrrn)
	{
	  RtlpCurDirRef = (fcwd_access_t **) (extract_adrp_address (pc) +
					      ldroffset);
	  break;
	}
    }

  return RtlpCurDirRef;
}

