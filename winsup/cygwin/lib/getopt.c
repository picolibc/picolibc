/*
 * Copyright (c) 1987, 1993, 1994, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"

int	  opterr = 1;	/* if error message should be printed */
int	  optind = 1;	/* index into parent argv vector */
int	  optopt;	/* character checked for validity */
int	  optreset;	/* reset getopt */
char 	  *optarg;	/* argument associated with option */

static char * __progname (char *);
int getopt_internal (int, char * const *, const char *);

static char * __progname(nargv0)
    char * nargv0;
{
    char * tmp = strrchr(nargv0, '/');
    if (tmp) tmp++; else tmp = nargv0;
    return(tmp);
}

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int
getopt_internal(nargc, nargv, ostr)
	int nargc;
	char * const *nargv;
	const char *ostr;
{
	static const char *place = EMSG;	/* option letter processing */
	char *oli;				/* option letter list index */

	if (optreset || !*place) {		/* update scanning pointer */
		optreset = 0;
		if (optind >= nargc || *(place = nargv[optind]) != '-') {
			place = EMSG;
			return (-1);
		}
		if (place[1] && *++place == '-') {	/* found "--" */
			/* ++optind; */
			place = EMSG;
			return (-2);
		}
	}					/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
	    !(oli = strchr(ostr, optopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means -1.
		 */
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++optind;
		if (opterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", __progname(nargv[0]), optopt);
		return (BADCH);
	}
	if (*++oli != ':') {			/* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	} else {				/* need an argument */
		if (*place)			/* no white space */
			optarg = (char *)place;
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			if ((opterr) && (*ostr != ':'))
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    __progname(nargv[0]), optopt);
			return (BADARG);
		} else				/* white space */
			optarg = nargv[optind];
		place = EMSG;
		++optind;
	}
	return (optopt);			/* dump back option letter */
}

/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int
getopt(nargc, nargv, ostr)
	int nargc;
	char * const *nargv;
	const char *ostr;
{
	int retval;

	if ((retval = getopt_internal(nargc, nargv, ostr)) == -2) {
		retval = -1;
		++optind;
	}
	return(retval);
}

/*
 * getopt_long --
 *	Parse argc/argv argument vector.
 */
int
getopt_long(nargc, nargv, options, long_options, index)
     int nargc;
     char ** nargv;
     char * options;
     struct option * long_options;
     int * index;
{
	int retval;

	if ((retval = getopt_internal(nargc, nargv, options)) == -2) {
		char *current_argv = nargv[optind++] + 2, *has_equal;
		int i, match = -1;
		size_t current_argv_len;

		if (*current_argv == '\0') {
			return(-1);
		}
		if ((has_equal = strchr(current_argv, '='))) {
			current_argv_len = has_equal - current_argv;
			has_equal++;
		} else
			current_argv_len = strlen(current_argv);

		for (i = 0; long_options[i].name; i++) {
			if (strncmp(current_argv, long_options[i].name, current_argv_len))
				continue;

			if (strlen(long_options[i].name) == current_argv_len) {
				match = i;
				break;
			}
			if (match == -1)
				match = i;
		}
		if (match != -1) {
			if (long_options[match].has_arg) {
				if (has_equal)
					optarg = has_equal;
				else
					optarg = nargv[optind++];
			}
			if ((long_options[match].has_arg == 1) && (optarg == NULL)) {
				/* Missing option, leading : indecates no error */
				if ((opterr) && (*options != ':'))
					(void)fprintf(stderr,
				      "%s: option requires an argument -- %s\n",
				      __progname(nargv[0]), current_argv);
				return (BADARG);
			}
		} else { /* No matching argument */
			if ((opterr) && (*options != ':'))
				(void)fprintf(stderr,
				    "%s: illegal option -- %s\n", __progname(nargv[0]), current_argv);
			return (BADCH);
		}
		if (long_options[match].flag) {
			*long_options[match].flag = long_options[match].val;
			retval = 0;
		} else
			retval = long_options[match].val;
		if (index)
			*index = match;
	}
	return(retval);
}
/*****************************************************************/






#include <stdio.h>
#include "getopt.h"

/* Stuff for getopt */
static struct option long_options[] = {
	{ (char *)"simple", 0, NULL, 's' },
	{ (char *)"t", 0, NULL, 't' },
	{ (char *)"u", 1, NULL, 'u' },
	{ (char *)"v", 0, NULL, 'v' },
	/* Do not reorder the following */
	{ (char *)"yy", 0, NULL, 'Y' },
	{  (char *)"y", 0, NULL, 'y' },
	{ (char *)"zz", 0, NULL, 'z' },
	{ (char *)"zzz", 0, NULL, 'Z' },
	{ NULL, 0, NULL, 0 }
};
extern char * optarg;
extern int optreset;
extern int optind;

int test_getopt_long(args, expected_result)
    char ** args, * expected_result;
{
	char actual_result[256];
	int count, pass, i;

	pass = 0;
	optind = 1;
	optreset = 1;
	for (count = 0; args[count]; count++);
	while ((i = getopt_long(count, args, (char *)"ab:", long_options, NULL)) != EOF) {
		switch(i) {
		case 'u':
			if (strcmp(optarg, "bogus")) {
				printf("--u option does not have bogus optarg.\n");
				return(1);
			}
		case 'Y':
		case 's':
		case 't':
		case 'v':
		case 'y':
		case 'z':
			actual_result[pass++] = i;
			break;
		default:
			actual_result[pass++] = '?';
			break;
		}
	}

	actual_result[pass] = '\0';
	return(strcmp(actual_result, expected_result));
	
}

#if 0
int usage(value)
	int value;
{
	printf("test_getopt [-d]\n");
	exit(value);
}
#endif

#if 0

/*
 * Static arglists for individual tests
 * This is ugly and maybe I should just use a variable arglist
 */
const char *argv1[] = { "Test simple", "--s", NULL };
const char *argv2[] = { "Test multiple", "--s", "--t", NULL };
const char *argv3[] = { "Test optarg with space", "--u", "bogus", NULL };
const char *argv4[] = { "Test optarg with equal", "--u=bogus", NULL };
const char *argv5[] = { "Test complex", "--s", "--t", "--u", "bogus", "--v", NULL };
const char *argv6[] = { "Test exact", "--y", NULL };
const char *argv7[] = { "Test abbr", "--z", NULL };
const char *argv8[] = { "Test simple termination", "--z", "foo", "--z", NULL };
const char *argv9[] = { "Test -- termination", "--z", "--", "--z", NULL };

int debug = 0;
int main(argc, argv)
    int argc;
    char ** argv;
{
	int i;

	/* Of course if getopt() has a bug this won't work */
	while ((i = getopt(argc, argv, "d")) != EOF) {
		switch(i) {
		case 'd':
			debug++;
			break;
		default:
			usage(1);
			break;
		}
	}

	/* Test getopt_long() */
	{
		if (test_getopt_long(argv1, "s")) {
			printf("Test simple failed.\n");
			exit(1);
		}
	}

	/* Test multiple arguments */
	{
		if (test_getopt_long(argv2, "st")) {
			printf("Test multiple failed.\n");
			exit(1);
		}
	}

	/* Test optarg with space */
	{
		if (test_getopt_long(argv3, "u")) {
			printf("Test optarg with space failed.\n");
			exit(1);
		}
	}

	/* Test optarg with equal */
	{
		if (test_getopt_long(argv4, "u")) {
			printf("Test optarg with equal failed.\n");
			exit(1);
		}
	}

	/* Test complex */
	{
		if (test_getopt_long(argv5, "stuv")) {
			printf("Test complex failed.\n");
			exit(1);
		}
	}

	/* Test that exact matches override abbr matches */
	{
		if (test_getopt_long(argv6, "y")) {
			printf("Test exact failed.\n");
			exit(1);
		}
	}

	/* Test that abbr matches are first match. */
	{
		if (test_getopt_long(argv7, "z")) {
			printf("Test abbr failed.\n");
			exit(1);
		}
	}

	/* Test that option termination succeeds */
	{
		if (test_getopt_long(argv8, "z")) {
			printf("Test simple termination failed.\n");
			exit(1);
		}
	}

	/* Test that "--" termination succeeds */
	{
		if (test_getopt_long(argv9, "z")) {
			printf("Test -- termination failed.\n");
			exit(1);
		}
	}
	exit(0);
}
#endif
