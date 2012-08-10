/* e_printf.c

   Copyright (c) 2011, Adapteva, Inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Adapteva nor the names of its contributors may be
      used to endorse or promote products derived from this software without
      specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.                                               */
#include <stdarg.h>
#include <string.h>




int trap_7( char *s , int _bsize, int _argsize) {
	volatile  register void* buff_adrr asm("r0") = s;
	volatile register unsigned bsize asm("r1") = _bsize;
	volatile register unsigned argsize asm("r2") = _argsize;
	volatile register int result asm("r0");
	__asm__ __volatile__ ("trap 7" : "=r" (result) : "r" (buff_adrr), "r" (bsize) , "r" (argsize));
	return result;

}

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>

volatile static int*fp_int =0;
volatile int getIntFromFloat(float *f) {
	fp_int= (int*)f;

	return *fp_int;
}


int e_printf(const char *fmt, ...) {

	char buf[256],*p,*fmt_p;
	va_list args;
	int percentMet=0;
	int pos = 0;

	char *v_arg_s;
	unsigned v_arg_int;
	float fl_f[1];

	unsigned fmt_len =  strnlen(fmt,128);

	fmt_p = (char*)fmt;

	//printf("---- 111 +++  %d  \n", strlen(fmt));

	va_start(args, fmt);

	p = (char*)buf;

	strcpy(buf,fmt);
	pos =fmt_len;
	p[pos] = '\0';
	pos++;

	va_start(args, fmt);

	while (*fmt_p) {

//		putchar(*p);
//		puts("");

		if(*fmt_p == '%') {
			percentMet=1;
		}
		if(*fmt_p == 's'  && percentMet == 1 ) {
			percentMet=0;
			v_arg_s = va_arg(args, char *);
			if (!v_arg_s) v_arg_s = "<NULL>";

			strcpy(p + pos , v_arg_s );
			pos+=strlen(v_arg_s);

			p[pos] = '\0';
			pos++;
		}

		if((*fmt_p == 'i' || *fmt_p == 'd' || *fmt_p == 'u'   || *fmt_p == 'x' || *fmt_p == 'f' )  && percentMet == 1 ) {
			percentMet=0;
			if(*fmt_p == 'f' ) {
				fl_f[0] = (float)va_arg(args, double);
				//printf("v_arg_ float  --- %f \n", fl_f[0]);
				//printf("v_arg_ p  --- %x \n", *( (unsigned *)fl_f));

				v_arg_int = getIntFromFloat (fl_f);

				//v_arg_int = 0;

			} else {
				v_arg_int =  (int)va_arg(args, int);
			}

//			if(*fmt_p == 'd') {
//				printf("v_arg_int --- %d \n", v_arg_int);
//			}
//			if(*fmt_p == 'x') {
//				printf("v_arg_int --- %x \n", v_arg_int);
//			}
//			if(*fmt_p == 'f') {
//				printf("fff++v_arg_int --- %x \n", v_arg_int);
//				fl = (float*)&v_arg_int;
//				printf("fff++ v_arg_float --- %f \n", *fl);
//			}

			*(p + pos) = ((v_arg_int>>24) & 0xff);
			pos++;
			*(p + pos) = ((v_arg_int>>16) & 0xff);
			pos++;
			*(p + pos) = ((v_arg_int>>8) & 0xff);
			pos++;
			*(p + pos) = ((v_arg_int>>0) & 0xff);
			pos++;
		}

		fmt_p++;
	}

	va_end(args);

	//printf(" +++  %d %d \n" , strlen(fmt), pos);

	return trap_7((char*)buf ,fmt_len, pos) ;
	//return 1;
}
