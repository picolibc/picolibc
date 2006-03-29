/* Copyright (c) 1995, 1996 Cygnus Support.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Provide a definition of `environ' if crt0.o doesn't.  */

static char *initial_env[] = { 0 };

/* Posix says `environ' is a pointer to a null terminated list of pointers.
   Hence `environ' itself is never NULL.  */
char **environ = &initial_env[0];
