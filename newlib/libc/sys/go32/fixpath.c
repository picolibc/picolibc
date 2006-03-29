/*
  (c) Copyright 1992 Eric Backus

  This software may be used freely so long as this copyright notice is
  left intact.  There is no warrantee on this software.
*/

#include "dos.h"		/* For intdos() */
#include <errno.h>		/* For errno */
#include <string.h>		/* For strlen() */

int
_get_default_drive(void)
{
    union REGS	regs;

    regs.h.ah = 0x19;		/* DOS Get Default Drive call */
    regs.h.al = 0;
    (void) intdos(&regs, &regs);
    return regs.h.al;
}

static char *
get_current_directory(char *out, int drive_number)
{
    union REGS	regs;

    regs.h.ah = 0x47;
    regs.h.dl = drive_number + 1;
    regs.x.si = (unsigned long) (out + 1);
    (void) intdos(&regs, &regs);
    if (regs.x.cflag != 0)
    {
	errno = regs.x.ax;
	return out;
    }
    else
    {
	/* Root path, don't insert "/", it'll be added later */
	if (*(out + 1) != '\0')
	    *out = '/';
	else
	    *out = '\0';
	return out + strlen(out);
    }
}

static int
is_slash(int c)
{
    return c == '/' || c == '\\';
}

static int
is_term(int c)
{
    return c == '/' || c == '\\' || c == '\0';
}

/* Takes as input an arbitrary path.  Fixes up the path by:
   1. Removing consecutive slashes
   2. Removing trailing slashes
   3. Making the path absolute if it wasn't already
   4. Removing "." in the path
   5. Removing ".." entries in the path (and the directory above them)
   6. Adding a drive specification if one wasn't there
   7. Converting all slashes to '/'
 */
void
_fixpath(const char *in, char *out)
{
    int		drive_number;
    const char	*ip = in;
    char	*op = out;

    /* Add drive specification to output string */
    if (*(ip + 1) == ':' && ((*ip >= 'a' && *ip <= 'z') ||
			     (*ip >= 'A' && *ip <= 'Z')))
    {
	if (*ip >= 'a' && *ip <= 'z')
	    drive_number = *ip - 'a';
	else
	    drive_number = *ip - 'A';
	*op++ = *ip++;
	*op++ = *ip++;
    }
    else
    {
	drive_number = _get_default_drive();
	*op++ = drive_number + 'a';
	*op++ = ':';
    }

    /* Convert relative path to absolute */
    if (!is_slash(*ip))
	op = get_current_directory(op, drive_number);

    /* Step through the input path */
    while (*ip)
    {
	/* Skip input slashes */
	if (is_slash(*ip))
	{
	    ip++;
	    continue;
	}

	/* Skip "." and output nothing */
	if (*ip == '.' && is_term(*(ip + 1)))
	{
	    ip++;
	    continue;
	}

	/* Skip ".." and remove previous output directory */
	if (*ip == '.' && *(ip + 1) == '.' && is_term(*(ip + 2)))
	{
	    ip += 2;
	    /* Don't back up over drive spec */
	    if (op > out + 2)
		/* This requires "/" to follow drive spec */
		while (!is_slash(*--op));
	    continue;
	}

	/* Copy path component from in to out */
	*op++ = '/';
	while (!is_term(*ip)) *op++ = *ip++;
    }

    /* If root directory, insert trailing slash */
    if (op == out + 2) *op++ = '/';

    /* Null terminate the output */
    *op = '\0';
}

#ifdef	TEST
#include <stdio.h>

int
main(int argc, char **argv)
{
    char	path[90];
    int		i;

    for (i = 1; i < argc; i++)
    {
	_fixpath(argv[i], path);
	(void) printf("'%s' -> '%s'\n", argv[i], path);
    }

    return 0;
}
#endif
