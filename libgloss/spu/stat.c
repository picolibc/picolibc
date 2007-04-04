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

#include <stdarg.h>
#include <fcntl.h>
#include "jsre.h"

int
stat (const char *pathname, struct stat *pstat)
{
	syscall_stat_t sys;
	syscall_out_t   *psys_out = ( syscall_out_t* )&sys;
	jsre_stat_t pjstat;

	sys.pathname = (unsigned int)pathname;
	sys.ptr = ( unsigned int )&pjstat;

	__send_to_ppe (JSRE_POSIX1_SIGNALCODE, JSRE_STAT, &sys);

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

	return( psys_out->rc );
}

