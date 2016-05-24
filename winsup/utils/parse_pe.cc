/* parse_pe.cc

   Written by Egor Duda <deo@logos-m.ru>

   This file is part of Cygwin.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License (file COPYING.dumper) for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#define PACKAGE
#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>

#include "dumper.h"

int
exclusion::add (LPBYTE mem_base, SIZE_T mem_size)
{
  while (last >= size)
    size += step;
  region = (process_mem_region *) realloc (region, size * sizeof (process_mem_region));
  if (region == NULL)
    return 0;
  region[last].base = mem_base;
  region[last].size = mem_size;
  last++;
  return 1;
};

int
cmp_regions (const void *r1, const void *r2)
{
  if (((process_mem_region *) r1)->base < ((process_mem_region *) r2)->base)
    return -1;
  if (((process_mem_region *) r1)->base > ((process_mem_region *) r2)->base)
    return 1;
  return 0;
}

int
exclusion::sort_and_check ()
{
  qsort (region, last, sizeof (process_mem_region), &cmp_regions);
  for (process_mem_region * p = region; p < region + last - 1; p++)
    {
      process_mem_region *q = p + 1;
      if (q == p + 1)
	continue;
      if (p->base + size > q->base)
	{
	  fprintf (stderr, "region error @ (%p + %zd) > %p\n", p->base, size, q->base);
	  return 0;
	}
    }
  return 1;
}

static void
select_data_section (bfd * abfd, asection * sect, PTR obj)
{
  exclusion *excl_list = (exclusion *) obj;

  if ((sect->flags & (SEC_CODE | SEC_DEBUGGING)) &&
      sect->vma && bfd_get_section_size (sect))
    {
      excl_list->add ((LPBYTE) sect->vma, (SIZE_T) bfd_get_section_size (sect));
      deb_printf ("excluding section: %20s %08lx\n", sect->name,
		  bfd_get_section_size (sect));
    }
}

int
parse_pe (const char *file_name, exclusion * excl_list)
{
  if (file_name == NULL || excl_list == NULL)
    return 0;

  bfd *abfd = bfd_openr (file_name, "pei-i386");
  if (abfd == NULL)
    {
      bfd_perror ("failed to open file");
      return 0;
    }

  bfd_check_format (abfd, bfd_object);
  bfd_map_over_sections (abfd, &select_data_section, (PTR) excl_list);
  excl_list->sort_and_check ();

  bfd_close (abfd);
  return 1;
}
