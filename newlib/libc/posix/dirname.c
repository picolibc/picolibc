/* Copyright 2005 Shaun Jackman
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <libgen.h>
#include <string.h>

#if defined(__GNUC__) && !defined(clang) && __OPTIMIZE_SIZE__
/*
 * GCC 12.x has a bug in -Os mode on (at least) arm v8.1-m which
 * mis-compiles this function. Work around that by switching
 * optimization mode
 */
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC optimize("O2")
#endif

char *
dirname (char *path)
{
	char *p;
	if( path == NULL || *path == '\0' )
		return ".";
	p = path + strlen(path) - 1;
	while( *p == '/' ) {
		if( p == path )
			return path;
		*p-- = '\0';
	}
	while( p >= path && *p != '/' )
		p--;
	while( p > path && p[-1] == '/' )
                p--;
	return
		p < path ? "." :
		p == path ? "/" :
		(*p = '\0', path);
}
