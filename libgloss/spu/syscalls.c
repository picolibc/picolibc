/*
(C) Copyright IBM Corp. 2005, 2006

All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.
    * Neither the name of IBM nor the names of its contributors may be 
used to endorse or promote products derived from this software without 
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.

Author: Andreas Neukoetter (ti95neuk@de.ibm.com)
*/

#include <spu_intrinsics.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "../syscall.h"

#include "jsre.h"

static void
send_to_ppe_0x2101 (int opcode, void *data)
{

	unsigned int	combined = ( ( opcode<<24 )&0xff000000 ) | ( ( unsigned int )data & 0x00ffffff );

        vector unsigned int stopfunc = {
                0x00002101,     /* stop 0x2101 */
                (unsigned int) combined,
                0x4020007f,     /* nop */
                0x35000000      /* bi $0 */
        };

        void (*f) (void) = (void *) &stopfunc;
        asm ("sync");
        return (f ());
}


int
isatty (int fd)
{
	return (0);
}

int
getpid ()
{
	return (1);
}

int
kill (int pid, int sig)
{
	if (pid == 1)
	  {
		  _exit (sig);
	  }
}

int
read (int file, void *ptr, size_t len)
{
        syscall_write_t sys;
	syscall_out_t	*psys_out = ( syscall_out_t* )&sys;

	sys.file = file;
	sys.ptr = ( unsigned int )ptr;
	sys.len = len;

	send_to_ppe_0x2101 (JSRE_READ, &sys);

        errno = psys_out->err;
        return ( psys_out->rc);
}

off_t
lseek (int file, off_t offset, int whence)
{
        syscall_lseek_t sys;
	syscall_out_t	*psys_out = ( syscall_out_t* )&sys;

	sys.file = file;
	sys.offset = offset;

	switch( whence ){
		case SEEK_SET:
			sys.whence = JSRE_SEEK_SET;
			break;
		case SEEK_CUR:
			sys.whence = JSRE_SEEK_CUR;
			break;
		case SEEK_END:
			sys.whence = JSRE_SEEK_END;
			break;
	}

	send_to_ppe_0x2101 (JSRE_LSEEK, &sys);

        errno = psys_out->err;
        return ( psys_out->rc);
}

int
write (int file, const void *ptr, size_t len)
{
        syscall_write_t sys;
	syscall_out_t	*psys_out = ( syscall_out_t* )&sys;

	sys.file = file;
	sys.ptr = ( unsigned int )ptr;
	sys.len = len;

	send_to_ppe_0x2101 (JSRE_WRITE, &sys);

        errno = psys_out->err;
        return ( psys_out->rc);
}

int
open (const char *filename, int flags, ...)
{
        int rc;
        int len;

        syscall_open_t sys ;
	syscall_out_t	*psys_out = ( syscall_out_t* )&sys;

        sys.pathname = ( unsigned int )filename;

	sys.flags = 0;

	sys.flags |= ( ( flags & O_CREAT ) ? JSRE_O_CREAT : 0 );
	sys.flags |= ( ( flags & O_EXCL ) ? JSRE_O_EXCL : 0 );
	sys.flags |= ( ( flags & O_NOCTTY ) ? JSRE_O_NOCTTY : 0 );
	sys.flags |= ( ( flags & O_TRUNC ) ? JSRE_O_TRUNC : 0 );
	sys.flags |= ( ( flags & O_APPEND ) ? JSRE_O_APPEND : 0 );
//	sys.flags |= ( ( flags & O_NOBLOCK ) ? JSRE_O_NOBLOCK : 0 );
//	sys.flags |= ( ( flags & O_NDELAY ) ? JSRE_O_NDELAY : 0 );
	sys.flags |= ( ( flags & O_SYNC ) ? JSRE_O_SYNC : 0 );
//	sys.flags |= ( ( flags & O_NOFOLLOW ) ? JSRE_O_NOFOLLOW : 0 );
//	sys.flags |= ( ( flags & O_DIRECTORY ) ? JSRE_O_DIRECTORY : 0 );
//	sys.flags |= ( ( flags & O_DIRECT ) ? JSRE_O_DIRECT : 0 );
//	sys.flags |= ( ( flags & O_ASYNC ) ? JSRE_O_ASYNC : 0 );
//	sys.flags |= ( ( flags & O_LARGEFILE ) ? JSRE_O_LARGEFILE : 0 );


	sys.flags |= ( ( flags & O_RDONLY ) ? JSRE_O_RDONLY : 0 );
	sys.flags |= ( ( flags & O_WRONLY ) ? JSRE_O_WRONLY : 0 );
	sys.flags |= ( ( flags & O_RDWR )  ? JSRE_O_RDWR  : 0 );


	/* FIXME: we have to check/map all flags */

        if ((sys.flags & O_CREAT))
          {
                  va_list ap;

                  va_start (ap, flags);
                  sys.mode = va_arg (ap, int);
                  va_end (ap);

          }
        else
          {
                  sys.mode = 0;
          }

        send_to_ppe_0x2101 ( JSRE_OPEN, &sys);

        errno = psys_out->err;
        return ( psys_out->rc);
}

int
close (int file)
{
        int rc;

        syscall_close_t sys ;
	syscall_out_t	*psys_out = ( syscall_out_t* )&sys;

	sys.file = file;

        send_to_ppe_0x2101 (JSRE_CLOSE, &sys);

        errno = psys_out->err;
        return ( psys_out->rc);
}

int
fstat (int file, struct stat *pstat)
{
        syscall_fstat_t sys;
        syscall_out_t   *psys_out = ( syscall_out_t* )&sys;
        jsre_stat_t pjstat;

        sys.file = file;
        sys.ptr = ( unsigned int )&pjstat;

        send_to_ppe_0x2101 (JSRE_FSTAT, &sys);

        pstat->st_dev = pjstat.dev;
        pstat->st_ino = pjstat.ino;
        pstat->st_mode = pjstat.mode;
        pstat->st_nlink = pjstat.nlink;
        pstat->st_uid = pjstat.uid;
        pstat->st_gid = pjstat.gid;
        pstat->st_rdev = pjstat.rdev;
        pstat->st_size = pjstat.size;
        pstat->st_blksize = pjstat.blksize;
        pstat->st_blocks = pjstat.blocks;
        pstat->st_atime = pjstat.atime;
        pstat->st_mtime = pjstat.mtime;
        pstat->st_ctime = pjstat.ctime;


        errno = psys_out->err;
        return( psys_out->rc );
}

int
stat (const char *pathname, struct stat *pstat)
{
        syscall_stat_t sys;
        syscall_out_t   *psys_out = ( syscall_out_t* )&sys;
        jsre_stat_t pjstat;

        sys.pathname = pathname;
        sys.ptr = ( unsigned int )&pjstat;

        send_to_ppe_0x2101 (JSRE_STAT, &sys);

        pstat->st_dev = pjstat.dev;
        pstat->st_ino = pjstat.ino;
        pstat->st_mode = pjstat.mode;
        pstat->st_nlink = pjstat.nlink;
        pstat->st_uid = pjstat.uid;
        pstat->st_gid = pjstat.gid;
        pstat->st_rdev = pjstat.rdev;
        pstat->st_size = pjstat.size;
        pstat->st_blksize = pjstat.blksize;
        pstat->st_blocks = pjstat.blocks;
        pstat->st_atime = pjstat.atime;
        pstat->st_mtime = pjstat.mtime;
        pstat->st_ctime = pjstat.ctime;

        errno = psys_out->err;
        return( psys_out->rc );
}


int
unlink (const char *pathname)
{
        int rc;

        syscall_unlink_t sys ;
	syscall_out_t	*psys_out = ( syscall_out_t* )&sys;

	sys.pathname = ( unsigned int )pathname;

        send_to_ppe_0x2101 (JSRE_UNLINK, &sys);

        errno = psys_out->err;
        return ( psys_out->rc);
}

