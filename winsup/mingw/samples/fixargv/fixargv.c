/*
 * fixargv.c
 *
 * A special function which "fixes" an argv array by replacing arguments
 * that need quoting with quoted versions.
 *
 * NOTE: In order to be reasonably consistent there is some misuse of the
 *       const keyword here-- which leads to compilation warnings. These
 *       should be ok to ignore.
 *
 * This is a sample distributed as part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Colin Peters <colin@bird.fu.is.saga-u.ac.jp>
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRENTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAMED. This includes but is not limited to warrenties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Revision$
 * $Author$
 * $Date$
 *
 */

#include <string.h>
#include "fixargv.h"

/*
 * This takes a single string and fixes it, enclosing it in quotes if it
 * contains any spaces and/or escaping the quotes it contains.
 */
char*
fix_arg (const char* szArg)
{
	int	nQuoteAll;	/* Does the whole arg need quoting? */
	int	nBkSlRun;	/* How may backslashes in a row? */
	char*	sz;
	char*	szNew;
	size_t	sizeLen;

	nQuoteAll = 0;
	nBkSlRun = 0;
	sz = szArg;
	sizeLen = 1;

	/* First we figure out how much bigger the new string has to be
	 * than the old one. */
	while (*sz != '\0')
	{
		/*
		 * Arguments containing whitespace of wildcards will be
		 * quoted to preserve tokenization and/or those special
		 * characters (i.e. wildcarding will NOT be done at the
		 * other end-- they will get the * and ? characters as is).
		 * TODO: Is this the best way? Do we want to enable wildcards?
		 *       If so, when?
		 */
		if (!nQuoteAll &&
		    (*sz == ' ' || *sz == '\t' || *sz == '*' || *sz == '?'))
		{
			nQuoteAll = 1;
		}
		else if (*sz == '\\')
		{
			nBkSlRun++;
		}
		else
		{
			if (*sz == '\"')
			{
				sizeLen += nBkSlRun + 1;
			}
			nBkSlRun = 0;
		}

		sizeLen++;
		sz++;
	}

	if (nQuoteAll)
	{
		sizeLen += 2;
	}

	/*
	 * Make a new string big enough.
	 */
	szNew = (char*) malloc (sizeLen);
	if (!szNew)
	{
		return NULL;
	}
	sz = szNew;

	/* First enclosing quote for fully quoted args. */
	if (nQuoteAll)
	{
		*sz = '\"';
		sz++;
	}

	/*
	 * Go through the string putting backslashes in front of quotes,
	 * and doubling all backslashes immediately in front of quotes.
	 */
	nBkSlRun = 0;
	while (*szArg != '\0')
	{
		if (*szArg == '\\')
		{
			nBkSlRun++;
		}
		else
		{
			if (*szArg == '\"')
			{
				while (nBkSlRun > 0)
				{
					*sz = '\\';
					sz++;
					nBkSlRun--;
				}
				*sz = '\\';
				sz++;
			}
			nBkSlRun = 0;
		}

		*sz = *szArg;
		sz++;
		szArg++;
	}

	/* Closing quote for fully quoted args. */
	if (nQuoteAll)
	{
		*sz = '\"';
		sz++;
	}

	*sz = '\0';
	return szNew;
}

/*
 * Takes argc and argv and returns a new argv with escaped members. Pass
 * this fixed argv (along with the old one) to free_fixed_argv after
 * you finish with it. Pass in an argc of -1 and make sure the argv vector
 * ends with a null pointer to have fix_argv count the arguments for you.
 */
char* const*
fix_argv (int argc, char* const* szaArgv)
{
	char**	szaNew;
	char*	sz;
	int	i;

	if (!szaArgv)
	{
		return NULL;
	}

	/*
	 * Count the arguments if asked.
	 */
	if (argc == -1)
	{
		for (i = 0; szaArgv[i]; i++)
			;

		argc = i;
	}

	/*
	 * If there are no args or only one arg then do no escaping.
	 */
	if (argc < 2)
	{
		return szaArgv;
	}

	for (i = 1, szaNew = NULL; i < argc; i++)
	{
		sz = szaArgv[i];

		/*
		 * If an argument needs fixing, then fix it.
		 */
		if (strpbrk (sz, "\" \t*?"))
		{
			/*
			 * If we haven't created a new argv list already
			 * then make one.
			 */
			if (!szaNew)
			{
				szaNew = (char**) malloc ((argc + 1) *
					sizeof (char*));
				if (!szaNew)
				{
					return NULL;
				}

				/*
				 * Copy previous args from old to new.
				 */
				memcpy (szaNew, szaArgv, sizeof(char*) * i);
			}

			/*
			 * Now do the fixing.
			 */
			szaNew[i] = fix_arg (sz);
			if (!szaNew[i])
			{
				/* Fixing failed, free up and return error. */
				free_fixed_argv (szaNew, szaArgv);
				return NULL;
			}
		}
		else if (szaNew)
		{
			szaNew[i] = sz;
		}
	}

	if (szaNew)
	{
		/* If we have created a new argv list then we might as well
		 * terminate it nicely. (And we depend on it in
		 * free_fixed_argv.) */
		szaNew[argc] = NULL;
	}
	else
	{
		/* If we didn't create a new argv list then return the
		 * original. */
		return szaArgv;
	}

	return szaNew;
}

void
free_fixed_argv (char* const* szaFixed, char* const* szaOld)
{
	char* const*	sza;

	/*
	 * Check for error conditions. Also note that if no corrections
	 * were required the fixed argv will actually be the same as
	 * the old one, and we don't need to do anything.
	 */
	if (!szaFixed || !szaOld || szaFixed == szaOld)
	{
		return;
	}

	/*
	 * Go through all members of the argv list. If any of the
	 * members in the fixed list are different from the old
	 * list we free those members.
	 * NOTE: The first member is never modified, so we don't need to
	 * check.
	 */
	sza = szaFixed + 1;
	szaOld++;
	while (*sza)
	{
		if (*sza != *szaOld)
		{
			free (*sza);
		}
		sza++;
		szaOld++;
	}

	/*
	 * Now we can free the array of char pointers itself.
	 */
	free (szaFixed);
}

