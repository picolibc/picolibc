/* libc/sys/linux/inode.c - Inode-related system calls */

/* Written 2000 by Werner Almesberger */


#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <linux/dirent.h>
#include <sys/syscall.h>


_syscall2(int,link,const char *,oldpath,const char *,newpath)
_syscall1(int,unlink,const char *,pathname)
_syscall1(int,chdir,const char *,path)
_syscall3(int,mknod,const char *,pathname,mode_t,mode,dev_t,dev)
_syscall2(int,chmod,const char *,path,mode_t,mode)
_syscall2(int,utime,const char *,filename,struct utimbuf *,buf)
_syscall2(int,access,const char *,filename,int,mode)
_syscall2(int,mkdir,const char *,pathname,mode_t,mode)
_syscall1(int,rmdir,const char *,pathname)
_syscall1(int,pipe,int *,filedes)
_syscall1(mode_t,umask,mode_t,mask)
_syscall1(int,chroot,const char *,path)
_syscall2(int,symlink,const char *,oldpath,const char *,newpath)
_syscall3(int,readlink,const char *,path,char *,buf,size_t,bufsiz)
_syscall2(int,stat,const char *,file_name,struct stat *,buf)
_syscall2(int,lstat,const char *,file_name,struct stat *,buf)
_syscall2(int,fstat,int,filedes,struct stat *,buf)
_syscall3(int,getdents,int,fd,struct dirent *,dirp,unsigned int,count)
