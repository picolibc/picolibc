/* IBM RS/6000 "XCOFF" file definitions for BFD.
   Copyright 1990, 1991, 1993, 1994, 1995 Free Software Foundation, Inc.
   FIXME: Can someone provide a transliteration of this name into ASCII?
   Using the following chars caused a compiler warning on HIUX (so I replaced
   them with octal escapes), and isn't useful without an understanding of what
   character set it is.
   Written by Mimi Ph\373\364ng-Th\345o V\365 of IBM
   and John Gilmore of Cygnus Support.  */

#define DO_NOT_DEFINE_AOUTHDR
#define DO_NOT_DEFINE_AUXENT
#define L_LNNO_SIZE 2
#include "coff/external.h"

        /* IBM RS/6000 */
#define U802WRMAGIC     0730    /* writeable text segments **chh**      */
#define U802ROMAGIC     0735    /* readonly sharable text segments      */
#define U802TOCMAGIC    0737    /* readonly text segments and TOC       */

#define BADMAG(x)	\
	((x).f_magic != U802ROMAGIC && (x).f_magic != U802WRMAGIC && \
	 (x).f_magic != U802TOCMAGIC)

/********************** AOUT "OPTIONAL HEADER" **********************/

typedef struct 
{
  unsigned char	magic[2];	/* type of file			*/
  unsigned char	vstamp[2];	/* version stamp		*/
  unsigned char	tsize[4];	/* text size in bytes, padded to FW bdry */
  unsigned char	dsize[4];	/* initialized data "  "	*/
  unsigned char	bsize[4];	/* uninitialized data "   "	*/
  unsigned char	entry[4];	/* entry pt.			*/
  unsigned char	text_start[4];	/* base of text used for this file */
  unsigned char	data_start[4];	/* base of data used for this file */
  unsigned char	o_toc[4];	/* address of TOC */
  unsigned char	o_snentry[2];	/* section number of entry point */
  unsigned char	o_sntext[2];	/* section number of .text section */
  unsigned char	o_sndata[2];	/* section number of .data section */
  unsigned char	o_sntoc[2];	/* section number of TOC */
  unsigned char	o_snloader[2];	/* section number of .loader section */
  unsigned char	o_snbss[2];	/* section number of .bss section */
  unsigned char	o_algntext[2];	/* .text alignment */
  unsigned char	o_algndata[2];	/* .data alignment */
  unsigned char	o_modtype[2];	/* module type (??) */
  unsigned char o_cputype[2];	/* cpu type */
  unsigned char	o_maxstack[4];	/* max stack size (??) */
  unsigned char o_maxdata[4];	/* max data size (??) */
  unsigned char	o_resv2[12];	/* reserved */
}
AOUTHDR;

#define AOUTSZ 72
#define SMALL_AOUTSZ (28)
#define AOUTHDRSZ 72

#define	RS6K_AOUTHDR_OMAGIC	0x0107	/* old: text & data writeable */
#define	RS6K_AOUTHDR_NMAGIC	0x0108	/* new: text r/o, data r/w */
#define	RS6K_AOUTHDR_ZMAGIC	0x010B	/* paged: text r/o, both page-aligned */

/* More names of "special" sections.  */
#define _PAD	".pad"
#define _LOADER	".loader"

/* XCOFF uses a special .loader section with type STYP_LOADER.  */
#define STYP_LOADER 0x1000

/* XCOFF uses a special .debug section with type STYP_DEBUG.  */
#define STYP_DEBUG 0x2000

/* XCOFF handles line number or relocation overflow by creating
   another section header with STYP_OVRFLO set.  */
#define STYP_OVRFLO 0x8000

union external_auxent
{
  struct
  {
    char x_tagndx[4];	/* str, un, or enum tag indx */

    union
    {
      struct
      {
	char  x_lnno[2]; /* declaration line number */
	char  x_size[2]; /* str/union/array size */
      } x_lnsz;

      char x_fsize[4];	/* size of function */

    } x_misc;

    union
    {
      struct 		/* if ISFCN, tag, or .bb */
      {
	char x_lnnoptr[4];	/* ptr to fcn line # */
	char x_endndx[4];	/* entry ndx past block end */
      } x_fcn;

      struct 		/* if ISARY, up to 4 dimen. */
      {
	char x_dimen[E_DIMNUM][2];
      } x_ary;

    } x_fcnary;

    char x_tvndx[2];		/* tv index */

  } x_sym;

  union
  {
    char x_fname[E_FILNMLEN];

    struct
    {
      char x_zeroes[4];
      char x_offset[4];
    } x_n;

  } x_file;

  struct
  {
    char x_scnlen[4];			/* section length */
    char x_nreloc[2];	/* # relocation entries */
    char x_nlinno[2];	/* # line numbers */
  } x_scn;

  struct
  {
    char x_tvfill[4];	/* tv fill value */
    char x_tvlen[2];	/* length of .tv */
    char x_tvran[2][2];	/* tv range */
  } x_tv;		/* info about .tv section (in auxent of symbol .tv)) */

  struct
  {
    unsigned char x_scnlen[4];
    unsigned char x_parmhash[4];
    unsigned char x_snhash[2];
    unsigned char x_smtyp[1];
    unsigned char x_smclas[1];
    unsigned char x_stab[4];
    unsigned char x_snstab[2];
  } x_csect;
};

#define	AUXENT	union external_auxent
#define	AUXESZ	18

#define DBXMASK 0x80		/* for dbx storage mask */
#define SYMNAME_IN_DEBUG(symptr) ((symptr)->n_sclass & DBXMASK)

/********************** RELOCATION DIRECTIVES **********************/

struct external_reloc
{
  char r_vaddr[4];
  char r_symndx[4];
  char r_size[1];
  char r_type[1];
};

#define RELOC struct external_reloc
#define RELSZ 10

#define DEFAULT_DATA_SECTION_ALIGNMENT 4
#define DEFAULT_BSS_SECTION_ALIGNMENT 4
#define DEFAULT_TEXT_SECTION_ALIGNMENT 4
/* For new sections we havn't heard of before */
#define DEFAULT_SECTION_ALIGNMENT 4
