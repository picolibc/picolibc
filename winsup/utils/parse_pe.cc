/* parse_pe.cc

   Copyright 1999 Cygnus Solutions.

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>

#include "dumper.h"

int
exclusion::add ( LPBYTE mem_base, DWORD mem_size )
{
  while ( last >= size ) size += step;
  region = (process_mem_region*) realloc ( region, size * sizeof ( process_mem_region ) );
  if ( region == NULL ) return 0;
  region [ last ].base = mem_base;
  region [ last ].size = mem_size;
  last++;
  return 1;
};

int cmp_regions ( const void* r1, const void* r2 )
{
  if ( ((process_mem_region*) r1)->base < ((process_mem_region*) r2)->base )
    return -1;
  if ( ((process_mem_region*) r1)->base > ((process_mem_region*) r2)->base )
    return 1;
  return 0;
}

int
exclusion::sort_and_check ()
{
  qsort ( region, last, sizeof ( process_mem_region ), &cmp_regions );
  for ( process_mem_region* p = region; p < region + last - 1; p++ )
    {
      process_mem_region* q = p + 1;
      if ( p->base + size > q->base )
        {
          fprintf ( stderr, "region error @ %08x", p->base );
          return 0;
        }
    }
  return 1;
}

static void
select_data_section ( bfd *abfd, asection *sect, PTR obj )
{
  exclusion* excl_list = (exclusion*) obj;

  if ( ( sect->flags & ( SEC_CODE | SEC_DEBUGGING ) ) &&
       sect->vma && sect->_raw_size )
    {
      excl_list->add ( (LPBYTE)sect->vma, (DWORD)sect->_raw_size );
      deb_printf ( "excluding section: %20s %08lx\n", sect->name, sect->_raw_size);
    }
}

int
parse_pe ( const char* file_name, exclusion* excl_list )
{
  if ( file_name == NULL || excl_list == NULL ) return 0;

  bfd* abfd = bfd_openr ( file_name, "pei-i386" );
  if ( abfd == NULL )
    {
      bfd_perror ( "failed to open file" );
      return 0;
    }

  bfd_check_format ( abfd, bfd_object );
  bfd_map_over_sections ( abfd, &select_data_section, (PTR)excl_list );
  excl_list->sort_and_check ();

  bfd_close ( abfd );
  return 1;
}

