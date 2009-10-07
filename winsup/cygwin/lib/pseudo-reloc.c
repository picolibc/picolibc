/* pseudo-reloc.c

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

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(__CYGWIN__)
#include <wchar.h>
#include <ntdef.h>
#include <stdarg.h>
#include <sys/cygwin.h>
/* copied from winsup.h */
# define NO_COPY __attribute__((nocommon)) __attribute__((section(".data_cygwin_nocopy")))
/* custom status code: */
#define STATUS_ILLEGAL_DLL_PSEUDO_RELOCATION ((NTSTATUS) 0xe0000269)
#else
# define NO_COPY
#endif

extern char __RUNTIME_PSEUDO_RELOC_LIST__;
extern char __RUNTIME_PSEUDO_RELOC_LIST_END__;
extern char _image_base__;

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

#if defined(__CYGWIN__)
#define SHORT_MSG_BUF_SZ 128
/* This function is used to print short error messages
 * to stderr, which may occur during DLL initialization
 * while fixing up 'pseudo' relocations. This early, we
 * may not be able to use cygwin stdio functions, so we
 * use the win32 WriteFile api. This should work with both
 * normal win32 console IO handles, redirected ones, and
 * cygwin ptys.
 */
static BOOL
__print_reloc_error (const char *fmt, ...)
{
  char buf[SHORT_MSG_BUF_SZ];
  wchar_t module[MAX_PATH];
  char * posix_module = NULL;
  BOOL rVal = FALSE;
  static const char * UNKNOWN_MODULE = "<unknown module>: ";
  DWORD len;
  DWORD done;
  va_list args;
  HANDLE errh = GetStdHandle (STD_ERROR_HANDLE);
  ssize_t modulelen = GetModuleFileNameW (NULL, module, sizeof (module));

  if (errh == INVALID_HANDLE_VALUE)
    return FALSE;

  if (modulelen > 0)
    posix_module = cygwin_create_path (CCP_WIN_W_TO_POSIX, module);

  va_start (args, fmt);
  len = (DWORD) vsnprintf (buf, SHORT_MSG_BUF_SZ, fmt, args);
  va_end (args);
  buf[SHORT_MSG_BUF_SZ-1] = '\0'; /* paranoia */

  if (posix_module)
    {
      rVal = WriteFile (errh, (PCVOID)posix_module,
                        strlen(posix_module), &done, NULL) &&
             WriteFile (errh, (PCVOID)": ", 2, &done, NULL) &&
             WriteFile (errh, (PCVOID)buf, len, &done, NULL);
      free (posix_module);
    }
  else
    {
      rVal = WriteFile (errh, (PCVOID)UNKNOWN_MODULE,
                        sizeof(UNKNOWN_MODULE), &done, NULL) &&
             WriteFile (errh, (PCVOID)buf, len, &done, NULL);
    }
  return rVal;
}
#endif /* __CYGWIN__ */

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
__write_memory (void *addr,const void *src,size_t len)
{
  MEMORY_BASIC_INFORMATION b;
  DWORD oldprot;
  SIZE_T memsz;

  if (!len)
    return;

  memsz = VirtualQuery (addr, &b, sizeof(b));

#if defined(__CYGWIN__)
  /* CYGWIN: If error, print error message and die. */
  if (memsz == 0)
    {
      __print_reloc_error (
        "error while loading shared libraries: bad address specified 0x%08x.\n",
        addr);
      cygwin_internal (CW_EXIT_PROCESS,
                       STATUS_ILLEGAL_DLL_PSEUDO_RELOCATION,
                       1);
    }
#else
  /* MINGW: If error, die. assert() may print error message when !NDEBUG */
  assert (memsz);
#endif

  /* Temporarily allow write access to read-only protected memory.  */
  if (b.Protect != PAGE_EXECUTE_READWRITE && b.Protect != PAGE_READWRITE)
    VirtualProtect (b.BaseAddress, b.RegionSize, PAGE_EXECUTE_READWRITE,
		  &oldprot);
  /* write the data. */
  memcpy (addr, src, len);
  /* Restore original protection. */
  if (b.Protect != PAGE_EXECUTE_READWRITE && b.Protect != PAGE_READWRITE)
    VirtualProtect (b.BaseAddress, b.RegionSize, oldprot, &oldprot);
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
   *         { 0, 0, RP_VERSION_V1 }
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
	  __write_memory ((void *) reloc_target, &newval, sizeof(DWORD));
	}
      return;
    }

  /* If we got this far, then we have relocations of version 2 or newer */

  /* Check if this is a known version.  */
  if (v2_hdr->version != RP_VERSION_V2)
    {
#if defined(__CYGWIN__)
      /* CYGWIN: Print error message and die, even when !DEBUGGING */
      __print_reloc_error (
        "error while loading shared libraries: invalid pseudo_reloc version %d.\n",
        (int) v2_hdr->version);
      cygwin_internal (CW_EXIT_PROCESS,
                       STATUS_ILLEGAL_DLL_PSEUDO_RELOCATION,
                       1);
#else
# if defined(DEBUG)
      /* MINGW: Don't die; just return to caller. If DEBUG, print error message. */
      fprintf (stderr, "internal mingw runtime error:"
	       "psuedo_reloc version %d is unknown to this runtime.\n",
	       (int) v2_hdr->version);
# endif
#endif
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
#ifdef _WIN64
	    if ((reldata & 0x80000000) != 0)
	      reldata |= ~((ptrdiff_t) 0xffffffff);
#endif
	    break;
#ifdef _WIN64
	  case 64:
	    reldata = (ptrdiff_t) (*((unsigned long long *)reloc_target));
	    break;
#endif
	  default:
	    reldata=0;
#if defined(__CYGWIN__)
            /* Print error message and die, even when !DEBUGGING */
            __print_reloc_error (
              "error while loading shared libraries: unknown pseudo_reloc bit size %d.\n",
              (int) (r->flags & 0xff));
            cygwin_internal (CW_EXIT_PROCESS,
                             STATUS_ILLEGAL_DLL_PSEUDO_RELOCATION,
                             1);
#else
# ifdef DEBUG
            /* MINGW: If error, don't die; just print message if DEBUG */
	    fprintf(stderr, "internal mingw runtime error: "
		    "unknown pseudo_reloc bit size %d\n",
		    (int) (r->flags & 0xff));
# endif
#endif
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
           __write_memory ((void *) reloc_target, &reldata, 4);
	   break;
#ifdef _WIN64
	 case 64:
           __write_memory ((void *) reloc_target, &reldata, 8);
	   break;
#endif
        }
     }
 }

void
_pei386_runtime_relocator ()
{
  static NO_COPY int was_init = 0;
  if (was_init)
    return;
  ++was_init;
  do_pseudo_reloc (&__RUNTIME_PSEUDO_RELOC_LIST__,
		   &__RUNTIME_PSEUDO_RELOC_LIST_END__,
		   &_image_base__);
}
