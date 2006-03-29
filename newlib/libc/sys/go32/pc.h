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
**
** Modified by J. Alan Eldridge, Liberty Brokerage, 77 Water St, NYC 10005
**
**    added getxkey(), which can read extended keystrokes that start with
**    0xE0, as well as those which start with 0x00
**    
**    added global char ScreenAttrib, the attribute used by ScreenClear():
**    it defaults to 0x07 so as not to break existing code.
**
**    added ScreenMode(), to return the current video mode
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
int getkey(void); /* ALT's have 0x100 set */
int getxkey(void); /* ALT's have 0x100 set, 0xe0 sets 0x200 */

void sound(int frequency);
#define nosound() sound(0)

extern unsigned char ScreenAttrib;
extern short *ScreenPrimary;
extern short *ScreenSecondary;

/* For the primary screen: */
int ScreenMode(void);
int ScreenRows(void);
int ScreenCols(void);
void ScreenPutChar(int ch, int attr, int x, int y);
void ScreenSetCursor(int row, int col);
void ScreenGetCursor(int *row, int *col);
void ScreenClear(void);
void ScreenUpdate(void *virtual_screen);
void ScreenUpdateLine(void *virtual_screen_line, int row);
void ScreenRetrieve(void *virtual_screen);

#ifdef __cplusplus
}
#endif

#endif
