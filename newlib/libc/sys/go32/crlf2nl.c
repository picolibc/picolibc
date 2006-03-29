/* This is file CRLF2NL.C */
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

unsigned crlf2nl(char *buf, unsigned len)
{
  char *bp = buf;
  int i=0;
  while (len--)
  {
    if (*bp != 13)
    {
      *buf++ = *bp;
      i++;
    }
    bp++;
  }
  return i;
}

unsigned readcr(int fd, char *buf, unsigned len)
{
  unsigned i;
  i = read(fd, buf, len);
  if (i <= 0)
    return i;
  return crlf2nl(buf, i);
}

static char *sbuf = 0;
#define BUFSIZE 4096

unsigned writecr(int fd, char *buf, unsigned len)
{
  unsigned bufp=0, sbufp=0, crcnt=0, rlen=0;
  int rv;
  if (sbuf == 0)
    sbuf = (char *)malloc(BUFSIZE+1);
  while (len--)
  {
    if (buf[bufp] == 10)
    {
      crcnt++;
      sbuf[sbufp++] = 13;
    }
    sbuf[sbufp++] = buf[bufp++];
    if ((sbufp >= BUFSIZE) || (len == 0))
    {
      rv = write(fd, sbuf, sbufp);
      if (rv < 0)
        return rv;
      rlen += rv - crcnt;
      crcnt = 0;
      sbufp = 0;
    }
  }
  return rlen;
}

