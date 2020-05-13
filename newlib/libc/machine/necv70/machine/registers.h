/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
by the University of California, Berkeley.  The name of the
University may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


typedef struct 
{
  	int fp_rounding:2;	/* fp rounding control 		*/
  	int integer_rounding:1;	/* integer rounding 		*/
  	int rfu:1;	        /* reserved			*/
	int fp_trap:5;   	/* floating point trap bits 	*/
	int otm:1;	
	int rfu2:3;
	int att:3;
	int rfu3:16;
} v60_tkcw_type;

