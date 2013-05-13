/*

Copyright (c) 2013  Red Hat, Inc. All rights reserved.

This copyrighted material is made available to anyone wishing to use, modify,
copy, or redistribute it subject to the terms and conditions of the BSD
License.   This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
are incorporated in the source code or documentation are not subject to the BSD
License and may only be used or replicated with the express permission of
Red Hat, Inc.

*/

#ifndef CIO_BUF_SIZE
#define CIO_BUF_SIZE 64
#endif

typedef struct {
  unsigned char length[2];
  unsigned char parms[9];
  unsigned char buf[CIO_BUF_SIZE];
} __CIOBUF__TYPE__;

extern __CIOBUF__TYPE__ __CIOBUF__;

extern void _libgloss_cio_hook (void);

#define CIO_OPEN    (0xF0)
#define CIO_CLOSE   (0xF1)
#define CIO_READ    (0xF2)
#define CIO_WRITE   (0xF3)
#define CIO_LSEEK   (0xF4)
#define CIO_UNLINK  (0xF5)
#define CIO_GETENV  (0xF6)
#define CIO_RENAME  (0xF7)
#define CIO_GETTIME (0xF8)
#define CIO_GETCLK  (0xF9)
#define CIO_SYNC    (0xFF)

