/*
** Copyright (C) 1991 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _PC_H_
#define _PC_H_

#ifdef __cplusplus
extern "C" {
#endif

unsigned char inportb(unsigned short port);
unsigned short inportw(unsigned short port);
unsigned long inportl(unsigned short port);
unsigned char inportsb(unsigned short port, unsigned char *buf, unsigned len);
unsigned short inportsw(unsigned short port, unsigned short *buf, unsigned len);
unsigned long inportsl(unsigned short port, unsigned long *buf, unsigned len);
void outportb(unsigned short port, unsigned char data);
void outportw(unsigned short port, unsigned short data);
void outportl(unsigned short port, unsigned long data);
void outportsb(unsigned short port, unsigned char *buf, unsigned len);
void outportsw(unsigned short port, unsigned short *buf, unsigned len);
void outportsl(unsigned short port, unsigned long *buf, unsigned len);

int kbhit(void);
int getkey(void);

void sound(int frequency);

extern short ScreenPrimary[];
extern short ScreenSecondary[];

/* For the primary screen: */
int ScreenRows();
int ScreenCols();
void ScreenPutChar(int ch, int attr, int x, int y);
void ScreenSetCursor(int row, int col);
void ScreenGetCursor(int *row, int *col);
void ScreenClear();


#ifdef __cplusplus
}
#endif

#endif

