/* pseudo-reloc.cc

   Contributed by Egor Duda  <deo@logos-m.ru>
   Modified by addition of runtime_pseudo_reloc version 2
   by Kai Tietz  <kai.tietz@onevision.com>

   THIS SOFTWARE IS NOT COPYRIGHTED

   This source code is offered for use in the public domain. You may
   use, modify or distribute it freely.

   This code is distributed in the hope that it will be useful but
   WITHOUT ANY WARRANTY. ALL WARRENTIES, EXPRESS OR IMPLIED ARE HEREBY
   DISCLAMED. This includes but is not limited to warrenties of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef __CYGWIN__
# include "windows.h"
# define NO_COPY
#else
# include "winsup.h"
# include <sys/cygwin.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#ifdef __GNUC__
#define ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
#define ATTRIBUTE_NORETURN
#endif

#ifndef __MINGW_LSYMBOL
#define __MINGW_LSYMBOL(sym) sym
#endif

extern char __RUNTIME_PSEUDO_RELOC_LIST__;
extern char __RUNTIME_PSEUDO_RELOC_LIST_END__;
extern char __MINGW_LSYMBOL(_image_base__);

/* v1 relocation is basically:
 *   *(base + .target) += .addend
 * where (base + .target) is always assumed to point
 * to a DWORD (4 bytes).
 */
typedef struct {
  DWORD addend;
  DWORD target;
} runtime_pseudo_reloc_item_v1;

/* v2 relocation is more complex. In effect, it is
 *    *(base + .target) += *(base + .sym) - (base + .sym)
 * with care taken in both reading, sign extension, and writing
 * because .flags may indicate that (base + .target) may point
 * to a BYTE, WORD, DWORD, or QWORD (w64).
 */
typedef struct {
  DWORD sym;
  DWORD target;
  DWORD flags;
} runtime_pseudo_reloc_item_v2;

typedef struct {
  DWORD magic1;
  DWORD magic2;
  DWORD version;
} runtime_pseudo_reloc_v2;

static void ATTRIBUTE_NORETURN
__report_error (const char *msg, ...)
{
#ifdef __CYGWIN__
  /* This function is used to print short error messages
   * to stderr, which may occur during DLL initialization
   * while fixing up 'pseudo' relocations. This early, we
   * may not be able to use cygwin stdio functions, so we
   * use the win32 WriteFile api. This should work with both
   * normal win32 console IO handles, redirected ones, and
   * cygwin ptys.
   */
  char buf[128];
  char *posix_module = NULL;
  static const char UNKNOWN_MODULE[] = "<unknown module>: ";
  static const char CYGWIN_FAILURE_MSG[] = "Cygwin runtime failure: ";
  HANDLE errh = GetStdHandle (STD_ERROR_HANDLE);
  va_list args;

  /* FIXME: cleanup further to avoid old use of cygwin_internal */
  if (errh == INVALID_HANDLE_VALUE)
    cygwin_internal (CW_EXIT_PROCESS, STATUS_ILLEGAL_DLL_PSEUDO_RELOCATION, 1);

  posix_module = (char *) cygwin_create_path (CCP_WIN_W_TO_POSIX,
					      global_progname);

  va_start (args, msg);
  vsnprintf (buf, sizeof (buf), msg, args);
  va_end (args);
  buf[sizeof (buf) - 1] = '\0'; /* paranoia */

  small_printf ("%s%s: %s\n", CYGWIN_FAILURE_MSG, posix_module ?: UNKNOWN_MODULE, buf);
  if (posix_module)
    free (posix_module);

  cygwin_internal (CW_EXIT_PROCESS, STATUS_ILLEGAL_DLL_PSEUDO_RELOCATION, 1);
  /* not reached, but silences noreturn warning */
  abort ();
#else
  va_list argp;
  va_start (argp, msg);
# ifdef __MINGW64_VERSION_MAJOR
  fprintf (stderr, "Mingw-w64 runtime failure:\n");
# else
  fprintf (stderr, "Mingw runtime failure:\n");
# endif
  vfprintf (stderr, msg, argp);
  va_end (argp);
  abort ();
#endif
}

/*
 * This function automatically sets addr as PAGE_EXECUTE_READWRITE
 * by deciding whether VirtualQuery for the addr is actually needed.
 * And it assumes that it is called in LdrpCallInitRoutine.
 * Hence not thread safe.
 */
static void
auto_protect_for (void* addr)
{
  static MEMORY_BASIC_INFORMATION mbi;
  static bool state = false;
  static DWORD oldprot;

  if (!addr)
    {
      /* Restore original protection. */
      if (!(mbi.Protect & (PAGE_EXECUTE_READWRITE | PAGE_READWRITE)))
        VirtualProtect (mbi.BaseAddress, mbi.RegionSize, oldprot, &oldprot);
      state = false;
      return;
    }
  if (state)
    {
      /* We have valid region information.  Are we still within this region?
         If so, just leave. */
      void *ptr = ((void*) ((ptrdiff_t) mbi.BaseAddress + mbi.RegionSize));
      if (addr >= mbi.BaseAddress && addr < ptr)
	return;
      /* Otherwise, restore original protection and fall through to querying
         and potentially changing next region. */
      if (!(mbi.Protect & (PAGE_EXECUTE_READWRITE | PAGE_READWRITE)))
	VirtualProtect (mbi.BaseAddress, mbi.RegionSize, oldprot, &oldprot);
    }
  else
    state = true;
  /* Query region and temporarily allow write access to read-only protected
     memory.  */
  VirtualQuery (addr, &mbi, sizeof mbi);
  if (!(mbi.Protect & (PAGE_EXECUTE_READWRITE | PAGE_READWRITE)))
    VirtualProtect (mbi.BaseAddress, mbi.RegionSize,
	PAGE_EXECUTE_READWRITE, &oldprot);
}

/* This function temporarily marks the page containing addr
 * writable, before copying len bytes from *src to *addr, and
 * then restores the original protection settings to the page.
 *
 * Using this function eliminates the requirement with older
 * pseudo-reloc implementations, that sections containing
 * pseudo-relocs (such as .text and .rdata) be permanently
 * marked writable. This older behavior sabotaged any memory
 * savings achieved by shared libraries on win32 -- and was
 * slower, too.  However, on cygwin as of binutils 2.20 the
 * .text section is still marked writable, and the .rdata section
 * is folded into the (writable) .data when --enable-auto-import.
 */
static void
__write_memory (void *addr, const void *src, size_t len)
{
  if (!len)
    return;
  /* Fix page protection for writing. */
  auto_protect_for (addr);
  /* write the data. */
  memcpy (addr, src, len);
}

#define RP_VERSION_V1 0
#define RP_VERSION_V2 1

static void
do_pseudo_reloc (void * start, void * end, void * base)
{
  ptrdiff_t addr_imp, reldata;
  ptrdiff_t reloc_target = (ptrdiff_t) ((char *)end - (char*)start);
  runtime_pseudo_reloc_v2 *v2_hdr = (runtime_pseudo_reloc_v2 *) start;
  runtime_pseudo_reloc_item_v2 *r;

  /* A valid relocation list will contain at least one entry, and
   * one v1 data structure (the smallest one) requires two DWORDs.
   * So, if the relocation list is smaller than 8 bytes, bail.
   */
  if (reloc_target < 8)
    return;

  /* Check if this is the old pseudo relocation version.  */
  /* There are two kinds of v1 relocation lists:
   *   1) With a (v2-style) version header. In this case, the
   *      first entry in the list is a 3-DWORD structure, with
   *      value:
   *	  { 0, 0, RP_VERSION_V1 }
   *      In this case, we skip to the next entry in the list,
   *      knowing that all elements after the head item can
   *      be cast to runtime_pseudo_reloc_item_v1.
   *   2) Without a (v2-style) version header. In this case, the
   *      first element in the list IS an actual v1 relocation
   *      record, which is two DWORDs.  Because there will never
   *      be a case where a v1 relocation record has both
   *      addend == 0 and target == 0, this case will not be
   *      confused with the prior one.
   * All current binutils, when generating a v1 relocation list,
   * use the second (e.g. original) form -- that is, without the
   * v2-style version header.
   */
  if (reloc_target >= 12
      && v2_hdr->magic1 == 0 && v2_hdr->magic2 == 0
      && v2_hdr->version == RP_VERSION_V1)
    {
      /* We have a list header item indicating that the rest
       * of the list contains v1 entries.  Move the pointer to
       * the first true v1 relocation record.  By definition,
       * that v1 element will not have both addend == 0 and
       * target == 0 (and thus, when interpreted as a
       * runtime_pseudo_reloc_v2, it will not have both
       * magic1 == 0 and magic2 == 0).
       */
      v2_hdr++;
    }

  if (v2_hdr->magic1 != 0 || v2_hdr->magic2 != 0)
    {
      /*************************
       * Handle v1 relocations *
       *************************/
      runtime_pseudo_reloc_item_v1 * o;
      for (o = (runtime_pseudo_reloc_item_v1 *) v2_hdr;
	   o < (runtime_pseudo_reloc_item_v1 *)end;
	   o++)
	{
	  DWORD newval;
	  reloc_target = (ptrdiff_t) base + o->target;
	  newval = (*((DWORD*) reloc_target)) + o->addend;
	  __write_memory ((void *) reloc_target, &newval, sizeof (DWORD));
	}
      /* Restore original protection. */
      auto_protect_for (NULL);
      return;
    }

  /* If we got this far, then we have relocations of version 2 or newer */

  /* Check if this is a known version.  */
  if (v2_hdr->version != RP_VERSION_V2)
    {
      __report_error ("  Unknown pseudo relocation protocol version %d.\n",
		      (int) v2_hdr->version);
      return;
    }

  /*************************
   * Handle v2 relocations *
   *************************/

  /* Walk over header. */
  r = (runtime_pseudo_reloc_item_v2 *) &v2_hdr[1];

  for (; r < (runtime_pseudo_reloc_item_v2 *) end; r++)
    {
      /* location where new address will be written */
      reloc_target = (ptrdiff_t) base + r->target;

      /* get sym pointer. It points either to the iat entry
       * of the referenced element, or to the stub function.
       */
      addr_imp = (ptrdiff_t) base + r->sym;
      addr_imp = *((ptrdiff_t *) addr_imp);

      /* read existing relocation value from image, casting to the
       * bitsize indicated by the 8 LSBs of flags. If the value is
       * negative, manually sign-extend to ptrdiff_t width. Raise an
       * error if the bitsize indicated by the 8 LSBs of flags is not
       * supported.
       */
      switch ((r->flags & 0xff))
	{
	case 8:
	  reldata = (ptrdiff_t) (*((unsigned char *)reloc_target));
	  if ((reldata & 0x80) != 0)
	    reldata |= ~((ptrdiff_t) 0xff);
	  break;
	case 16:
	  reldata = (ptrdiff_t) (*((unsigned short *)reloc_target));
	  if ((reldata & 0x8000) != 0)
	    reldata |= ~((ptrdiff_t) 0xffff);
	  break;
	case 32:
	  reldata = (ptrdiff_t) (*((unsigned int *)reloc_target));
#if defined (__x86_64__) || defined (_WIN64)
	  if ((reldata & 0x80000000) != 0)
	    reldata |= ~((ptrdiff_t) 0xffffffff);
#endif
	  break;
#if defined (__x86_64__) || defined (_WIN64)
	case 64:
	  reldata = (ptrdiff_t) (*((unsigned long long *)reloc_target));
	  break;
#endif
	default:
	  reldata=0;
	  __report_error ("  Unknown pseudo relocation bit size %d.\n",
		  (int) (r->flags & 0xff));
	  break;
	}

      /* Adjust the relocation value */
      reldata -= ((ptrdiff_t) base + r->sym);
      reldata += addr_imp;

      /* Write the new relocation value back to *reloc_target */
      switch ((r->flags & 0xff))
	{
	case 8:
	  __write_memory ((void *) reloc_target, &reldata, 1);
	  break;
	case 16:
	  __write_memory ((void *) reloc_target, &reldata, 2);
	  break;
	case 32:
#if defined (__CYGWIN__) && defined (__x86_64__)
	  if (reldata > (ptrdiff_t) __INT32_MAX__
	      || reldata < -((ptrdiff_t) __INT32_MAX__) - 1)
	    __report_error ("Invalid relocation.  Offset %p at address %p "
			    "doesn't fit into 32 bits", reldata, reloc_target);
#endif
	  __write_memory ((void *) reloc_target, &reldata, 4);
	  break;
#if defined (__x86_64__) || defined (_WIN64)
	case 64:
	  __write_memory ((void *) reloc_target, &reldata, 8);
	  break;
#endif
	}
    }
  /* Restore original protection. */
  auto_protect_for (NULL);
}

#ifdef __CYGWIN__
extern "C" void
_pei386_runtime_relocator (per_process *u)
{
  if (u)
    do_pseudo_reloc (u->pseudo_reloc_start, u->pseudo_reloc_end, u->image_base);
}
#else
extern "C" void
_pei386_runtime_relocator (void)
{
  static NO_COPY int was_init = 0;
  if (was_init)
    return;
  ++was_init;
  do_pseudo_reloc (&__RUNTIME_PSEUDO_RELOC_LIST__,
		   &__RUNTIME_PSEUDO_RELOC_LIST_END__,
		   &__MINGW_LSYMBOL(_image_base__));
}
#endif
