/* 
 * getopt.c
 *
 * $Id$
 *
 * Implementation of the `getopt', `getopt_long' and `getopt_long_only'
 * APIs, for inclusion in the MinGW runtime library.
 *
 * This file is part of the MinGW32 package set.
 *
 * Contributed by Keith Marshall <keithmarshall@users.sourceforge.net>
 *
 *
 * THIS SOFTWARE IS NOT COPYRIGHTED
 *
 * This source code is offered for use in the public domain. You may
 * use, modify or distribute it freely.
 *
 * This code is distributed in the hope that it will be useful but
 * WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 * DISCLAIMED. This includes but is not limited to warranties of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Revision$
 * $Author$
 * $Date$
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>

/* Identify how to get the calling program name, for use in messages...
 */
#ifdef __CYGWIN__
/*
 * CYGWIN uses this DLL reference...
 */
# define PROGNAME  __progname
extern char __declspec(dllimport) *__progname;
#else
/*
 * ...while elsewhere, we simply use the first argument passed.
 */
# define PROGNAME  *argv
#endif

/* Initialise the public variables. */

int optind = 1;				/* index for first non-option arg     */
int opterr = 1;				/* enable built-in error messages     */

char *optarg = NULL;			/* pointer to current option argument */

#define CHAR  char			/* argument type selector */

#define getopt_switchar         '-'	/* option prefix character in argv    */
#define getopt_pluschar         '+'	/* prefix for POSIX mode in optstring */
#define getopt_takes_argument   ':'	/* marker for optarg in optstring     */
#define getopt_arg_assign       '='     /* longopt argument field separator   */
#define getopt_unknown          '?'	/* return code for unmatched option   */
#define getopt_ordered           1      /* return code for ordered non-option */

#define getopt_all_done         -1	/* return code to indicate completion */

enum
{ /* All `getopt' API functions are implemented via calls to the
   * common static function `getopt_parse()'; these `mode' selectors
   * determine the behaviour of `getopt_parse()', to deliver the
   * appropriate result in each case.
   */
  getopt_mode_standard = 0,	/* getopt()	      */
  getopt_mode_long,		/* getopt_long()      */
  getopt_mode_long_only		/* getopt_long_only() */
};

enum
{ /* When attempting to match a command line argument to a long form option,
   * these indicate the status of the match.
   */
  getopt_no_match = 0,		/* no successful match			     */
  getopt_abbreviated_match,	/* argument is an abbreviation for an option */
  getopt_exact_match		/* argument matches the full option name     */
};

int optopt = getopt_unknown;	/* return value for option being evaluated   */

static __inline__
int getopt_missing_arg( const CHAR *optstring )
{
  /* Helper function to determine the appropriate return value,
   * for the case where a required option argument is missing.
   */
  if( (*optstring == getopt_pluschar) || (*optstring == getopt_switchar) )
    ++optstring;
  return (*optstring == getopt_takes_argument)
    ? getopt_takes_argument
    : getopt_unknown;
}

/* `complain' macro facilitates the generation of simple built-in
 * error messages, displayed on various fault conditions, provided
 * `opterr' is non-zero.
 */
#define	complain( MSG, ARG )  if( opterr ) \
  fprintf( stderr, "%s: "MSG"\n", PROGNAME, ARG )

static __inline__
int getopt_argerror( int mode, char *fmt, CHAR *prog, struct option *opt, int retval )
{
  /* Helper function, to generate more complex built-in error
   * messages, for invalid arguments to long form options ...
   */
  if( opterr )
  {
    /* ... but, displayed only if `opterr' is non-zero.
     */
    char flag[] = "--";
    if( mode != getopt_mode_long )
      /*
       * only display one hyphen, for implicit long form options,
       * improperly resolved by `getopt_long_only()'.
       */
      flag[1] = 0;
    /*
     * always preface the program name ...
     */
    fprintf( stderr, "%s: ", prog );
    /*
     * to the appropriate, option specific message.
     */
    fprintf( stderr, fmt, flag, opt->name );
  }
  /* Whether displaying the message, or not, always set `optopt'
   * to identify the faulty option ...
   */
  optopt = opt->val;
  /*
   * and return the `invalid option' indicator.
   */
  return retval;
}

/* `getopt_conventions' establish behavioural options, to control
 * the operation of `getopt_parse()', e.g. to select between POSIX
 * and GNU style argument parsing behaviour.
 */
#define getopt_set_conventions  0x1000
#define getopt_posixly_correct  0x0010

static __inline__
int getopt_conventions( int flags )
{
  static int conventions = 0;

  if( (conventions == 0) && ((flags & getopt_set_conventions) == 0) )
  {
    /* default conventions have not yet been established;
     * initialise them now!
     */
    conventions = getopt_set_conventions;
    if( (flags == getopt_pluschar) || (getenv( "POSIXLY_CORRECT" ) != NULL) )
      conventions |= getopt_posixly_correct;
  }

  else if( flags & getopt_set_conventions )
    /*
     * default conventions may have already been established,
     * but this is a specific request to augment them.
     */
    conventions |= flags;

  /* in any event, return the currently established conventions.
   */
  return conventions;
}

static __inline__
int is_switchar( CHAR flag )
{
  /* A simple helper function, used to identify the switch character
   * introducing an optional command line argument.
   */
  return flag == getopt_switchar;
}

static __inline__
const CHAR *getopt_match( CHAR lookup, const CHAR *opt_string )
{
  /* Helper function, used to identify short form options.
   */
  if( (*opt_string == getopt_pluschar) || (*opt_string == getopt_switchar) )
    ++opt_string;
  if( *opt_string == getopt_takes_argument )
    ++opt_string;
  do if( lookup == *opt_string ) return opt_string;
     while( *++opt_string );
  return NULL;
}

static __inline__
int getopt_match_long( const CHAR *nextchar, const CHAR *optname )
{
  /* Helper function, used to identify potential matches for
   * long form options.
   */
  CHAR matchchar;
  while( (matchchar = *nextchar++) && (matchchar == *optname) )
    /*
     * skip over initial substring which DOES match.
     */
    ++optname;

  if( matchchar )
  {
    /* did NOT match the entire argument to an initial substring
     * of a defined option name ...
     */
    if( matchchar != getopt_arg_assign )
      /*
       * ... and didn't stop at an `=' internal field separator,
       * so this is NOT a possible match.
       */
      return getopt_no_match;

    /* DID stop at an `=' internal field separator,
     * so this IS a possible match, and what follows is an
     * argument to the possibly matched option.
     */
    optarg = (char *)(nextchar);
  }
  return *optname
    /*
     * if we DIDN'T match the ENTIRE text of the option name,
     * then it's a possible abbreviated match ...
     */
    ? getopt_abbreviated_match
    /*
     * but if we DID match the entire option name,
     * then it's a DEFINITE EXACT match.
     */
    : getopt_exact_match;
}

static __inline__
int getopt_resolved( int mode, int argc, CHAR *const *argv, int *argind,
struct option *opt, int index, int *retindex, const CHAR *optstring )
{
  /* Helper function to establish appropriate return conditions,
   * on resolution of a long form option.
   */
  if( retindex != NULL )
    *retindex = index;

  if( optarg && (opt[index].has_arg == no_argument) )
    /*
     * it is an error for the user to specify an option specific argument
     * with an option which doesn't expect one!
     */
    return getopt_argerror( mode, "option `%s%s' doesn't accept an argument\n",
	PROGNAME, opt + index, getopt_unknown );

  else if( (optarg == NULL) && (opt[index].has_arg == required_argument) )
  {
    /* similarly, it is an error if no argument is specified
     * with an option which requires one ...
     */
    if( (*argind + 1) < argc )
      /*
       * ... except that the requirement may be satisfied from
       * the following comand line argument, if any ...
       */
      optarg = argv[++*argind];

    else
      /* so fail this case, only if no such argument exists!
       */
      return getopt_argerror( mode, "option `%s%s' requires an argument\n",
	  PROGNAME, opt + index, getopt_missing_arg( optstring ) );
  }

  /* when the caller has provided a return buffer ...
   */
  if( retindex != NULL )
  {
    /* ... then we place the proper return value there,
     * and return a status code of zero ...
     */
    *retindex = opt[index].val;
    return 0;
  }
  /* ... otherwise, the return value becomes the status code.
   */
  return opt[index].val;
}

static
#define getopt_std_args int argc, CHAR *const argv[], const CHAR *optstring
int getopt_parse( int mode, getopt_std_args, ... )
{
  /* Common core implementation for ALL `getopt' functions.
   */
  static int argind = 0;
  static const CHAR *nextchar = NULL;
  static int optmark = 0;

  if( (argind == 0) || (optind == 0) )
  {
    /* POSIX wants `optind' to have an initial value of one, but we want
     * it to be initialised to zero, when we are called for the first time,
     * (as indicated by `argind' having a value of zero).  We also want to
     * allow the caller to reset the `getopt' parser, causing it to scan
     * the arguments again, (or to scan a new set of arguments); this
     * may be achieved by the caller resetting `optind' to zero.
     */
    optmark = optind = argind = 0;
    nextchar = NULL;
  }

  if( nextchar && *nextchar )
  {
    /* we are parsing a standard, or short format, option argument ...
     */
    const CHAR *optchar;
    if( (optchar = getopt_match( optopt = *nextchar++, optstring )) != NULL )
    {
      /* we have identified it as valid ...
       */
      if( optchar[1] == getopt_takes_argument )
      {
	/* and determined that it requires an associated argument ...
	 */
	if( ! *(optarg = (char *)(nextchar)) )
	{
	  /* the argument is NOT attached ...
	   */
	  if( optchar[2] == getopt_takes_argument )
	    /*
	     * but this GNU extension marks it as optional,
	     * so we don't provide one on this occasion.
	     */
	    optarg = NULL;

	  /* otherwise this option takes a mandatory argument,
	   * so, provided there is one available ...
	   */
	  else if( (argc - argind) > 1 )
	    /*
	     * we take the following command line argument,
	     * as the appropriate option argument.
	     */
	    optarg = argv[++argind];

	  /* but if no further argument is available,
	   * then there is nothing we can do, except for
	   * issuing the requisite diagnostic message.
	   */
	  else
	  {
	    complain( "option requires an argument -- %c", optopt );
	    return getopt_missing_arg( optstring );
	  }
	}
	nextchar = NULL;
      }
      else
	optarg = NULL;
      return optopt;
    }
    /* if we didn't find a valid match for the specified option character,
     * then we fall through to here, so take appropriate diagnostic action.
     */
    if( mode == getopt_mode_long_only )
    {
      complain( "unrecognised option `-%s'", --nextchar );
      nextchar = NULL;
      optopt = 0;
    }
    else complain( "invalid option -- %c", optopt );
    return getopt_unknown;
  }

  if( optmark > optind )
  {
    /* This can happen, in GNU parsing mode ONLY, when we have
     * skipped over non-option arguments, and found a subsequent
     * option argument; in this case we permute the arguments.
     */
    int index;
    /*
     * `optspan' specifies the number of contiguous arguments
     * which are spanned by the current option, and so must be
     * moved together during permutation.
     */
    int optspan = argind - optmark + 1;
    /*
     * we use `this_arg' to store these temporarily.
     */
    CHAR *this_arg[optspan];
    /*
     * we cannot manipulate `argv' directly, since the `getopt'
     * API prototypes it as `read-only'; this cast to `arglist'
     * allows us to work around that restriction.
     */
    CHAR **arglist = (char **)(argv);

    /* save temporary copies of the arguments which are associated
     * with the current option ...
     */
    for( index = 0; index < optspan; ++index )
      this_arg[index] = arglist[optmark + index];

    /* move all preceding non-option arguments to the right,
     * overwriting these saved arguments, while making space
     * to replace them in their permuted location.
     */
    for( --optmark; optmark >= optind; --optmark )
      arglist[optmark + optspan] = arglist[optmark];

    /* restore the temporarily saved option arguments to
     * their permuted location.
     */
    for( index = 0; index < optspan; ++index )
      arglist[optind + index] = this_arg[index];

    /* adjust `optind', to account for the relocated option.
     */
    optind += optspan;
  }

  else
    /* no permutation occurred ...
     * simply adjust `optind' for all options parsed so far.
     */
    optind = argind + 1;

  /* enter main parsing loop ...
   */
  while( argc > ++argind )
  {
    /* inspect each argument in turn, identifying possible options ...
     */
    if( is_switchar( *(nextchar = argv[optmark = argind]) ) && *++nextchar )
    {
      /* we've found a candidate option argument ... */

      if( is_switchar( *nextchar ) )
      {
	/* it's a double hyphen argument ... */

	const CHAR *refchar = nextchar;
	if( *++refchar )
	{
	  /* and it looks like a long format option ...
	   * `getopt_long' mode must be active to accept it as such,
	   * `getopt_long_only' also qualifies, but we must downgrade
	   * it to force explicit handling as a long format option.
	   */
	  if( mode >= getopt_mode_long )
	  {
	    nextchar = refchar;
	    mode = getopt_mode_long;
	  }
	}
	else
	{
	  /* this is an explicit `--' end of options marker, so wrap up now!
	   */
	  if( optmark > optind )
	  {
	    /* permuting the argument list as necessary ...
	     * (note use of `this_arg' and `arglist', as above).
	     */
	    CHAR *this_arg = argv[optmark];
	    CHAR **arglist = (CHAR **)(argv);

	    /* move all preceding non-option arguments to the right ...
	     */
	    do arglist[optmark] = arglist[optmark - 1];
	       while( optmark-- > optind );

	    /* reinstate the `--' marker, in its permuted location.
	     */
	    arglist[optind] = this_arg;
	  }
	  /* ... before finally bumping `optind' past the `--' marker,
	   * and returning the `all done' completion indicator.
	   */
	  ++optind;
	  return getopt_all_done;
	}
      }
      else if( mode < getopt_mode_long_only )
      {
	/* it's not an explicit long option, and `getopt_long_only' isn't active,
	 * so we must explicitly try to match it as a short option.
	 */
	mode = getopt_mode_standard;
      }

      if( mode >= getopt_mode_long )
      {
	/* the current argument is a long form option, (either explicitly,
	 * introduced by a double hyphen, or implicitly because we were called
	 * by `getopt_long_only'); this is where we parse it.
	 */
	int lookup;
	int matched = -1;

	/* we need to fetch the `extra' function arguments, which are
	 * specified for the `getopt_long' APIs.
	 */
	va_list refptr;
	va_start( refptr, optstring );
	struct option *longopts = va_arg( refptr, struct option * );
	int *optindex = va_arg( refptr, int * );
	va_end( refptr );

	/* ensuring that `optarg' does not inherit any junk, from parsing
	 * preceding arguments ...
	 */
	optarg = NULL;
	for( lookup = 0; longopts && longopts[lookup].name; ++lookup )
	{
	  /* scan the list of defined long form options ...
	   */
          switch( getopt_match_long( nextchar, longopts[lookup].name ) )
          {
	    /* looking for possible matches for the current argument.
	     */
            case getopt_exact_match:
	      /*
	       * when an exact match is found,
	       * return it immediately, setting `nextchar' to NULL,
	       * to ensure we don't mistakenly try to match any
	       * subsequent characters as short form options.
	       */
	      nextchar = NULL;
	      return getopt_resolved( mode, argc, argv, &argind,
		  longopts, lookup, optindex, optstring );

	    case getopt_abbreviated_match:
	      /*
	       * but, for a partial (initial substring) match ...
	       */
	      if( matched >= 0 )
	      {
		/* if this is not the first, then we have an ambiguity ...
		 */
		complain( "option `%s' is ambiguous", argv[argind] );
		nextchar = NULL;
		optopt = 0;
		return getopt_unknown;
	      }
	      /* otherwise just note that we've found a possible match ...
	       */
	      matched = lookup;
          }
	}
	if( matched >= 0 )
	{
	  /* if we get to here, then we found exactly one partial match,
	   * so return it, as for an exact match.
	   */
	  nextchar = NULL;
	  return getopt_resolved( mode, argc, argv, &argind,
	      longopts, matched, optindex, optstring );
	}
	if( mode < getopt_mode_long_only )
	{
	  /* if here, then we had what SHOULD have been a long form option,
	   * but it is unmatched; (perversely, `mode == getopt_mode_long_only'
	   * allows us to still try to match it as a short form option).
	   */
	  optopt = 0;
	  nextchar = NULL;
	  complain( "unrecognised option `%s'", argv[argind] );
	  return getopt_unknown;
	}
      }
      /* fall through to handle standard short form options...
       * when the option argument format is neither explictly identified
       * as long, nor implicitly matched as such, and the argument isn't
       * just a bare hyphen, (which isn't an option), then we make one
       * recursive call to explicitly interpret it as short format.
       */
      if( *nextchar )
	return getopt_parse( mode, argc, argv, optstring );
    }
    /* if we get to here, then we've parsed a non-option argument ...
     * in GNU compatibility mode, we step over it, so we can permute
     * any subsequent option arguments, but ...
     */
    if( *optstring == getopt_switchar )
    {
      /* if `optstring' begins with a `-' character, this special
       * GNU specific behaviour requires us to return the non-option
       * arguments in strict order, as pseudo-arguments to a special
       * option, with return value defined as `getopt_ordered'.
       */
      nextchar = NULL;
      optarg = argv[argind];
      return getopt_ordered;
    }
    if( getopt_conventions( *optstring ) & getopt_posixly_correct )
      /*
       * otherwise ...
       * for POSIXLY_CORRECT behaviour, or if `optstring' begins with
       * a `+' character, then we break out of the parsing loop, so that
       * the scan ends at the current argument, with no permutation.
       */
      break;
  }
  /* fall through when all arguments have been evaluated,
   */
  return getopt_all_done;
}

/* All three public API entry points are trivially defined,
 * in terms of the internal `getopt_parse' function.
 */
int getopt( getopt_std_args )
{
  return getopt_parse( getopt_mode_standard, argc, argv, optstring );
}

int getopt_long( getopt_std_args, const struct option *opts, int *index )
{
  return getopt_parse( getopt_mode_long, argc, argv, optstring, opts, index );
}

int getopt_long_only( getopt_std_args, const struct option *opts, int *index )
{
  return getopt_parse( getopt_mode_long_only, argc, argv, optstring, opts, index );
}

#ifdef __weak_alias
/*
 * These Microsnot style uglified aliases are provided for compatibility
 * with the previous MinGW implementation of the getopt API.
 */
__weak_alias( getopt, _getopt )
__weak_alias( getopt_long, _getopt_long )
__weak_alias( getopt_long_only, _getopt_long_only )
#endif

/* $RCSfile$Revision$: end of file */
