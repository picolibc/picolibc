/* Mach-O support for BFD.
   Copyright 2011
   Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _MACH_O_EXTERNAL_H
#define _MACH_O_EXTERNAL_H

struct mach_o_header_external
{
  unsigned char magic[4];	/* Magic number.  */
  unsigned char cputype[4];	/* CPU that this object is for.  */
  unsigned char cpusubtype[4];	/* CPU subtype.  */
  unsigned char filetype[4];	/* Type of file.  */
  unsigned char ncmds[4];	/* Number of load commands.  */
  unsigned char sizeofcmds[4];	/* Total size of load commands.  */
  unsigned char flags[4];	/* Flags.  */
  unsigned char reserved[4];	/* Reserved (on 64-bit version only).  */
};

#define BFD_MACH_O_HEADER_SIZE 28
#define BFD_MACH_O_HEADER_64_SIZE 32

/* 32-bit section header.  */

struct mach_o_section_32_external
{
  unsigned char sectname[16];   /* Section name.  */
  unsigned char segname[16];    /* Segment that the section belongs to.  */
  unsigned char addr[4];        /* Address of this section in memory.  */
  unsigned char size[4];        /* Size in bytes of this section.  */
  unsigned char offset[4];      /* File offset of this section.  */
  unsigned char align[4];       /* log2 of this section's alignment.  */
  unsigned char reloff[4];      /* File offset of this section's relocs.  */
  unsigned char nreloc[4];      /* Number of relocs for this section.  */
  unsigned char flags[4];       /* Section flags/attributes.  */
  unsigned char reserved1[4];
  unsigned char reserved2[4];
};
#define BFD_MACH_O_SECTION_SIZE 68

/* 64-bit section header.  */

struct mach_o_section_64_external
{
  unsigned char sectname[16];   /* Section name.  */
  unsigned char segname[16];    /* Segment that the section belongs to.  */
  unsigned char addr[8];        /* Address of this section in memory.  */
  unsigned char size[8];        /* Size in bytes of this section.  */
  unsigned char offset[4];      /* File offset of this section.  */
  unsigned char align[4];       /* log2 of this section's alignment.  */
  unsigned char reloff[4];      /* File offset of this section's relocs.  */
  unsigned char nreloc[4];      /* Number of relocs for this section.  */
  unsigned char flags[4];       /* Section flags/attributes.  */
  unsigned char reserved1[4];
  unsigned char reserved2[4];
  unsigned char reserved3[4];
};
#define BFD_MACH_O_SECTION_64_SIZE 80

struct mach_o_load_command_external
{
  unsigned char cmd[4];         /* The type of load command.  */
  unsigned char cmdsize[4];     /* Size in bytes of entire command.  */
};
#define BFD_MACH_O_LC_SIZE 8

struct mach_o_segment_command_32_external
{
  unsigned char segname[16];    /* Name of this segment.  */
  unsigned char vmaddr[4];      /* Virtual memory address of this segment.  */
  unsigned char vmsize[4];      /* Size there, in bytes.  */
  unsigned char fileoff[4];     /* Offset in bytes of the data to be mapped.  */
  unsigned char filesize[4];    /* Size in bytes on disk.  */
  unsigned char maxprot[4];     /* Maximum permitted vm protection.  */
  unsigned char initprot[4];    /* Initial vm protection.  */
  unsigned char nsects[4];      /* Number of sections in this segment.  */
  unsigned char flags[4];       /* Flags that affect the loading.  */
};
#define BFD_MACH_O_LC_SEGMENT_SIZE 56 /* Include the header.  */

struct mach_o_segment_command_64_external
{
  unsigned char segname[16];    /* Name of this segment.  */
  unsigned char vmaddr[8];      /* Virtual memory address of this segment.  */
  unsigned char vmsize[8];      /* Size there, in bytes.  */
  unsigned char fileoff[8];     /* Offset in bytes of the data to be mapped.  */
  unsigned char filesize[8];    /* Size in bytes on disk.  */
  unsigned char maxprot[4];     /* Maximum permitted vm protection.  */
  unsigned char initprot[4];    /* Initial vm protection.  */
  unsigned char nsects[4];      /* Number of sections in this segment.  */
  unsigned char flags[4];       /* Flags that affect the loading.  */
};
#define BFD_MACH_O_LC_SEGMENT_64_SIZE 72 /* Include the header.  */

struct mach_o_reloc_info_external
{
  unsigned char r_address[4];
  unsigned char r_symbolnum[4];
};
#define BFD_MACH_O_RELENT_SIZE 8

struct mach_o_symtab_command_external
{
  unsigned char symoff[4];
  unsigned char nsyms[4];
  unsigned char stroff[4];
  unsigned char strsize[4];
};

struct mach_o_nlist_external
{
  unsigned char n_strx[4];
  unsigned char n_type[1];
  unsigned char n_sect[1];
  unsigned char n_desc[2];
  unsigned char n_value[4];
};
#define BFD_MACH_O_NLIST_SIZE 12

struct mach_o_nlist_64_external
{
  unsigned char n_strx[4];
  unsigned char n_type[1];
  unsigned char n_sect[1];
  unsigned char n_desc[2];
  unsigned char n_value[8];
};
#define BFD_MACH_O_NLIST_64_SIZE 16

struct mach_o_thread_command_external
{
  unsigned char flavour[4];
  unsigned char count[4];
};

/* For commands that just have a string or a path.  */
struct mach_o_str_command_external
{
  unsigned char str[4];
};

struct mach_o_dylib_command_external
{
  unsigned char name[4];
  unsigned char timestamp[4];
  unsigned char current_version[4];
  unsigned char compatibility_version[4];
};

struct mach_o_dysymtab_command_external
{
  unsigned char ilocalsym[4];	/* Index of.  */
  unsigned char nlocalsym[4];	/* Number of.  */
  unsigned char iextdefsym[4];
  unsigned char nextdefsym[4];
  unsigned char iundefsym[4];
  unsigned char nundefsym[4];
  unsigned char tocoff[4];
  unsigned char ntoc[4];
  unsigned char modtaboff[4];
  unsigned char nmodtab[4];
  unsigned char extrefsymoff[4];
  unsigned char nextrefsyms[4];
  unsigned char indirectsymoff[4];
  unsigned char nindirectsyms[4];
  unsigned char extreloff[4];
  unsigned char nextrel[4];
  unsigned char locreloff[4];
  unsigned char nlocrel[4];
};

struct mach_o_dylib_module_external
{
  unsigned char module_name[4];
  unsigned char iextdefsym[4];
  unsigned char nextdefsym[4];
  unsigned char irefsym[4];
  unsigned char nrefsym[4];
  unsigned char ilocalsym[4];
  unsigned char nlocalsym[4];
  unsigned char iextrel[4];
  unsigned char nextrel[4];
  unsigned char iinit_iterm[4];
  unsigned char ninit_nterm[4];
  unsigned char objc_module_info_addr[4];
  unsigned char objc_module_info_size[4];
};
#define BFD_MACH_O_DYLIB_MODULE_SIZE 52

struct mach_o_dylib_module_64_external
{
  unsigned char module_name[4];
  unsigned char iextdefsym[4];
  unsigned char nextdefsym[4];
  unsigned char irefsym[4];
  unsigned char nrefsym[4];
  unsigned char ilocalsym[4];
  unsigned char nlocalsym[4];
  unsigned char iextrel[4];
  unsigned char nextrel[4];
  unsigned char iinit_iterm[4];
  unsigned char ninit_nterm[4];
  unsigned char objc_module_info_size[4];
  unsigned char objc_module_info_addr[8];
};
#define BFD_MACH_O_DYLIB_MODULE_64_SIZE 56

struct mach_o_dylib_table_of_contents_external
{
  unsigned char symbol_index[4];
  unsigned char module_index[4];
};
#define BFD_MACH_O_TABLE_OF_CONTENT_SIZE 8

struct mach_o_linkedit_data_command_external
{
  unsigned char dataoff[4];
  unsigned char datasize[4];
};

struct mach_o_dyld_info_command_external
{
  unsigned char rebase_off[4];
  unsigned char rebase_size[4];
  unsigned char bind_off[4];
  unsigned char bind_size[4];
  unsigned char weak_bind_off[4];
  unsigned char weak_bind_size[4];
  unsigned char lazy_bind_off[4];
  unsigned char lazy_bind_size[4];
  unsigned char export_off[4];
  unsigned char export_size[4];
};

struct mach_o_version_min_command_external
{
  unsigned char version[4];
  unsigned char reserved[4];
};

struct mach_o_fat_header_external
{
  unsigned char magic[4];
  unsigned char nfat_arch[4];	/* Number of components.  */
};

struct mach_o_fat_arch_external
{
  unsigned char cputype[4];
  unsigned char cpusubtype[4];
  unsigned char offset[4];	/* File offset of the member.  */
  unsigned char size[4];	/* Size of the member.  */
  unsigned char align[4];	/* Power of 2.  */
};

#endif /* _MACH_O_EXTERNAL_H */
