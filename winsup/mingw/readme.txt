                      Minimalist GNU-Win32 Readme
                            version 0.1.3
                           March 20, 1997
             Colin Peters <colin@bird.fu.is.saga-u.ac.jp>


0. Introduction

Mingw32 is short for the Minimalist GNU-Win32 package, and it is a 
package which allows you to use GCC (as supplied by Cygnus in their GNU-
Win32 or Cygwin32 package) the GNU compiler, on Win32 platforms like 
Windows 95 or NT, to compile "native" programs.

In this case "native" means programs which don't require extra DLLs like 
the cygwin DLL. Mingw32 programs use CRTDLL.DLL to provide their C run 
time library functions, and CRTDLL.DLL is supplied with all current 
Win32 platforms. Thus the programs are light weight and easy to 
distribute, they also do not automatically fall under the GNU Public 
License as programs written with the GPL version of Cygwin32 do.


0.1 Archive Contents

Mingw32 version 0.1.3 is distributed in two files, mingw32_013.tar.gz 
and mingsrc013.tar.gz. The first file contains the following components:

	- Import libraries for building programs which use the
	  CRTDLL.DLL C run time library supplied with Win32 platforms.

	- crt0.o and dllcrt0.o, two "startup code" object files that
	  perform program or DLL initialization without using
	  CRTDLL.DLL (instead of CYGWIN.DLL).

	- specs, a configuration file for GCC which defines appropriate
	  options for creating executables which use the CRTDLL.DLL C
	  run time library.

	- Include files with appropriate type and macro definitions,
	  and function prototypes for use with CRTDLL.DLL.

The source distribution (mingsrc013.tar.gz) contains the .def files and 
source files used to create the various import libraries and object 
files in the above list.


0.2 Usage Notes

Unlike some previous releases of Mingw32 the current version defaults to 
building console applications, the same way that GCC normally does when 
installed from the Cygnus distribution. The Mingw32 specs file also 
introduces two command line arguments to GCC which can be used to 
conveniently specify a console or GUI type build. When building console 
programs "-console" can be used on the GCC command line, while GUI 
programs can be built by specifying "-windows" (I tried defining -gui, 
and it works, but produces an annoying warning about -gui not being 
supported (?)). For example:

	gcc -o hellogui.exe hellogui.c -luser32 -windows

Although using different "crt0" files for GUI and console applications 
has been suggested I have left the system more-or-less as it was in 
0.1.1: crt0 sets up for and calls main, and if you don't supply a main 
there is one in libmingw32.a, which in turn calls WinMain (actually 
WinMain@16). This allows either main or WinMain entry points in console 
or GUI applications, but if you don't supply main or WinMain, or don't 
prototype WinMain as __stdcall__ you will get a linker error about an 
"unresolved reference to WinMain@16." This is unfortunately cryptic, but 
otherwise the system works quite well.

An important note if you want to rebuild from the sources of Mingw32 or 
otherwise use the special version of Jam made for Mingw32: you need to 
have a version of "rm", the UNIX equivalent of del, somewhere in your 
path to use the current Jambase (which is built into the Jam 
executable). The version that comes with the Cygnus files is perfectly 
adequate.


0.3 Fixes and Improvements

Numerous small bug fixes have been made in the header files.

Floating point initialization, originally added in version 0.1.2, has 
been modified to use the _fpreset function from CRTDLL.DLL instead of 
cryptic and possibly less portable assembly code.

A new DLL-building option has been added to the specs file so that the 
following link line will appropriately link in dllcrt0.o instead of the 
normal crt0.o, and set the entry point correctly:

	gcc -dll -o dll.dll dll.o -Wl,dll.exp

A bug that would cause the wrong include files to be included in dual 
installations of Cygwin32 and Mingw32 has been fixed (I hope) in the 
Mingw32 specs file.

Alongside this release is a new release of Jam specially built for use 
with Mingw32. It should be available from the same place you got this 
file. This release of Jam includes rules for building DLLs, including 
resources in your executables and creating import libraries. I also 
intend to distribute a small set of example files showing how to do all 
of these things with Mingw32 and Jam.

In the "coming soon" category I have a version of the GNU Standard C++ 
library ported to Mingw32. This means you can use iostreams, complex 
numbers and all those neat STL (Standard Template Library) things 
without needing the Cygwin DLL. I hope to put this port up for 
downloading soon (along with the source of course).



1. Installing

1.1 Download and Unpack GNU-Win32 Beta 17.1

Because of the enormous size of the beta 17.1 release from Cygnus this 
process will require about 85 MB or more of free disk space. The first 
step, after downloading the Mingw32 package, is to download the GCC 
binary distribution, all.tar.gz, from Cygnus (or a mirror), which is 
about 10 MB. (Of course, if you just want the Cygwin32 install and are 
not actually interested in adding on Mingw32 you don't need the Mingw32 
package at all.)

Just to be safe, and if you have the 10 MB to spare, you should probably 
copy the all.tar.gz file to a reasonably safe place at this point. This 
will save you from the pain of downloading it again if something goes 
wrong later.

To complete this step you need a gzip program (or just gunzip) and a tar 
program. You can use the ones supplied by Cygnus (although some people 
seem to have trouble with them, especially if you try to use pipes) or 
one of the other ports available from your favorite freeware/shareware 
software site.

First un-gzip the file with a command line like:
	gunzip all.tar.gz
or
	gzip -d all.tar.gz

This will produce a all.tar file and erase the all.tar.gz file (there 
are options for gzip if you want to keep the original around). The tar 
file is about 40 MB.

Make a directory for the cygnus stuff, such as C:\cygnus for example. 
Move the tar file there (e.g. move \tmp\all.tar \cygnus). Don’t copy it 
unless you like waiting and wasting 40 MB of disk space.

Unpack the tar file into your new directory with a command line like:
	tar xvf all.tar

Run from the new directory (now containing the tar file). This is the 
step where disk space usage reaches its peak, since the tar extraction 
does not delete the all.tar file, and the amount of space taken by the 
extracted files plus the tar file itself is well in excess of 80 MB 
(mainly because, on my system at least, the files which are symbolic 
links in the tar archive are copied as they are expanded onto the FAT 
filesystem, so for example, a symbolic link to cygwin.dll, a 3 MB file, 
takes an extra 3 MB, since the file is simply duplicated in the new 
location). I could not actually do this on my laptop and had to extract 
the tar file from a mounted network drive!

NOTE: From here on I will refer to files as if you had installed in 
C:\cygnus. If you installed somewhere else then just replace C:\cygnus 
with the appropriate path wherever it occurs.


1.2 Setup Cygwin32

This step is not 100% necessary, but it helps at this point to determine 
if you’ve gotten this far without any major problems. Also, if you 
intend to use both Cygwin32 and Mingw32 you will have to do some of 
these steps eventually.

GCC and the other programs in the compiler suite all require cygwin.dll 
to run. There are two copies of this file: one in C:\cygnus\H-i386-
cygwin32\bin (this might be a symbolic link), and one in C:\cygnus\H-
i386-cygwin32\i386-cygwin32\lib (the original). Since this DLL is 
required by all Cygwin32 programs it makes sense to put one copy of it 
in your C:\Windows\System directory (or equivalent) and remove the extra 
copies. This will also save you headaches when the next release comes 
along and you have to make sure that everything is using the latest 
release of the DLL.

After doing that run the cygwin32.bat batch file included with this 
distribution, or otherwise perform the following settings:

	PATH=%PATH%;C:\cygnus\H-i386-cygwin32\bin
	SET GCC_EXEC_PREFIX=C:\cygnus\H-i386-cygwin32\lib\gcc-lib\i386-
		cygwin32\cygnus-2.7.2-961023
	SET LIBRARY_PATH=/cygnus/H-i386-cygwin32/lib/gcc-lib/i386-
		cygwin32/cygnus-2.7.2-961023:/cygnus/H-i386-cygwin32/i386-
		cygwin32/lib:/cygnus/H-i386-cygwin32/lib
	SET C_INCLUDE_PATH=/cygnus/H-i386-cygwin32/lib/gcc-lib/i386-
		cygwin32/cygnus-2.7.2-961023/include:/cygnus/H-i386-
		cygwin32/i386-cygwin32/include:/cygnus/include
	SET CPLUS_INCLUDE_PATH=%C_INCLUDE_PATH%

NOTE: You may need to increase the amount of environment space available 
at the command prompt to get these extremely long environment variables 
set. You can do this under Windows 95 by modifying the properties of the 
command prompt shortcut you use under the "Program" tab, adding a 
/e:#### argument to the command line COMMAND.COM, where #### is the 
number of bytes to set aside for the environment.

NOTE: Under Windows 95 changes made in your autoexec.bat file will not 
show up in new DOS boxes unless you reboot your machine.

Now write and compile a small test hello world program like this:

#include <stdio.h>

int
main ()
{
	printf ("Hello, world!\n");
	return 0;
}

Then compile it like this (assuming your file is called hello.c):

	gcc -o hello.exe hello.c

The compile should proceed without problems and you should be able to 
run the hello program at the end. It should print "Hello, world!" 
(without the quotes) to the console and then return to the command 
prompt.

If you wanted a full Cygwin32 install you now have it. With this setup 
(say, by adding those lines above to your autoexec.bat or global 
settings) you can port a great deal of UNIX code to run under Win32 
systems. No more steps are necessary.

If you are a minimalist or otherwise want to save disk space you should 
continue from here. Also if you intend to use the Minimalist GNU-Win32 
files to compile programs which don't use the Cygwin32 API you will need 
to do some of the things mentioned below.

If the compile didn't work for some reason check very carefully that you 
followed the instructions above correctly and then check whether one or 
more of the files in the download got corrupted. If neither of these 
seems to be the case then your system is not behaving like my system. 
Try looking at the troubleshooting section later in this file, and if 
none of that helps then you can email me (colin@bird.fu.is.saga-
u.ac.jp), though I can't promise I'll be a lot of help.


1.3 Separating the Win32 API Files

Mingw32 and Cygwin32 share the same set of Win32 API include files and 
import libraries as included in the GCC distribution from Cygnus. In 
order to use the Win32 API with a dual setup or with Mingw32 alone you 
will have to separate those files from the bulk of the Cygwin32 API 
files.

Make a new directory to serve as the root for the Win32 API files. I put 
mine under C:\cygnus and called it win32, but you can put it where you 
like and just replace later references to C:\cygnus\win32 with your own 
root directory.

Move the following from C:\cygnus\H-i386-cygwin32\i386-cygwin32\include 
to a new C:\cygnus\win32\include directory:

windows.h, winadvapi.h, winbase.h, wincon.h, windef.h, windowsx.h, 
winerror.h, wingdi.h, winkernel.h, winnt.h, wintypes.h, winuser.h, 
winversion.h, commdlg.h, ddeml.h and the Windows32 sub-directory and all 
its contents.

Move the following files from C:\cygnus\H-i386-cygwin32\i386-
cygwin32\lib to a new C:\cygnus\win32\lib directory:

libadvapi32.a, libcomctl32.a, libcomdlg32.a, libctl3d32.a, libgdi32.a, 
libglaux.a, libglu32.a, libimm32.a, libkernel32.a, liblz32.a, 
libmapi32.a, libmfcuia32.a, libmgmtapi.a, libmpr.a, libmsacm32.a, 
libnddeapi.a, libnetapi32.a, libodbc32.a, libodbccp32.a, libole32.a, 
liboleaut32.a, liboledlg.a, libolepro32.a, libopengl32.a, libpenwin32.a, 
libpkpd32.a, librasapi32.a, librpcdce4.a, librpcndr.a, librpcns4.a, 
librpcrt4.a, libscrnsave.a, libshell32.a, libsnmp.a, libsvrapi.a, 
libtapi32.a, libth32.a, libthunk32.a, liburl.a libuser32.a, libvdmdbg.a, 
libversion.a, libvfw32.a, libwin32spl.a, libwinmm.a, libwinserve.a, 
libwinspool.a, libwinstrm.a, libwow32.a, libwsock32.a, libwst.a.

That list is quite excessive for most basic Windows programming, which 
will only require kernel32, user32, gdi32, shell32 and possibly a couple 
of others like the common control and dialog libraries or advapi32. You 
may not need the ODBC support, or OLE, or Pen Windows, TAPI and on and 
on. Still, if you have the space and intend to use the Win32 API you 
might as well keep the ones you’re not sure you’ll ever use around.

The lists above can also act as lists of files you can safely delete if 
you are never going to use the Win32 API in your programs except that 
libkernel32.a is still required even if you don’t use the Win32 API 
yourself. Note that this means that libkernel32.a must be on the library 
path as well, even if you don’t use the Win32 API. (Actually this 
appears to be an artifact of the specs file supplied with Cygwin32. If 
you like, and feel up to it, you can play around with the specs file and 
remove the reference to kernel32.)

Here are the variable settings you need to make to allow GCC to find the 
Win32 API files in their new positions:

	SET LIBRARY_PATH=%LIBRARY_PATH%:/cygnus/win32/lib
	SET C_INCLUDE_PATH=%C_INCLUDE_PATH%:/cygnus/win32/include
	SET CPLUS_INCLUDE_PATH=%CPLUS_INCLUDE_PATH%:/cygnus/win32/include

The file win32-api.bat performs these settings. Run it after you run 
cygwin32.bat (or mingw32.bat below).

At this point you should be able to compile programs that use the Win32 
API, just as you could before. You might want to do a simple test 
compile to find out, for example this code:

#include <windows.h>

int STDCALL
WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
	MessageBox (NULL, "Test message", "Test", MB_OK);
	return 0;
}

Should compile with the following command line:
	gcc -o test.exe test.c -lkernel32 -luser32 -Wl,--subsystem,windows

It will produce a warning at link time about not finding 
_WinMainCRTStartup, but this is harmless.

If you have trouble check the troubleshooting section later in this 
file.


1.4 Specs

The file C:\cygnus\H-i386-cygwin32\lib\gcc-lib\i386-cygwin32\cygnus-
2.7.2-961023\specs includes a set of options and defaults for GCC, 
including such things as which libraries are automatically linked into 
executables and such. A different specs file is required depending on 
whether you use Cygwin32 or Mingw32.

To avoid GCC accidentally using the wrong specs file move specs to 
C:\cygnus\H-i386-cygwin32\i386-cygwin32\lib.

You can verify what specs file is being used by attempting a compile 
with the -v option to gcc. Note that if no specs file is mentioned the 
compiler will default to Cygwin32 behavior.


1.5 The Mingw32 Files

Now we can install the Mingw32 files and start making programs which 
don’t use cygwin.dll or the Cygwin32 API. I install my copy under a 
separate directory called C:\mingw32, but you could put them wherever 
you like (e.g. C:\cygnus\mingw32). Again simply replace references to 
C:\mingw32 with the directory where you perform your installation.

After making the install directory copy mingw32_012.tgz to that 
directory and run a command like this:
	gunzip -d mingw32_012.tgz

in that directory, followed by:
	tar xvf mingw32_012.tar

This will unpack the required files. Then you can use the following 
environment variable settings (as included in mingw32.bat) to setup for 
compiles using Mingw32:

	PATH=%PATH%;C:\cygnus\H-i386-cygwin32\bin
	SET GCC_EXEC_PREFIX=C:\cygnus\H-i386-cygwin32\lib\gcc-lib\i386-
		cygwin32\cygnus-2.7.2-961023\
	SET LIBRARY_PATH=/mingw32/lib
	SET C_INCLUDE_PATH=/mingw32/include:/mingw32/include/nonansi
	SET CPLUS_INCLUDE_PATH=%C_INCLUDE_PATH%

The mingw32.bat file can be used the same way as the cygwin32.bat file. 
Depending on which one you run you will be able to do Mingw32 compiles 
or Cygwin32 compiles. Note that whichever one you use you must follow it 
with an invocation of win32-api.bat so that libkernel32.a will be in the 
library path.

Setup is now complete, you have complete working Mingw32 and Cygwin32 
compiles available along with the bash shell, tons of UNIX-like 
utilities.

If you had trouble with any of the steps above then the next section is 
for you.



2. Troubleshooting Setup Problems

If you ran into trouble at any stage in the section 1 here are a few 
general guidelines as well as some solutions to common problems.

2.1 Winzip, gunzip or tar Complains of Errors

Winzip may complain that it could not create a file with garbage 
characters in it's name. Gunzip, gzip or tar may complain about 
formatting errors. Usually this means that the downloaded file is 
corrupted. As of this writing this problem was most commonly caused when 
downloading the files from Geocities using Netscape Navigator for 
Windows 95 or NT. A combination of a badly set MIME type at Geocities 
and a bug in Netscape will corrupt files saved with "Save Link As" (and 
clicking on the links would display the files as garbage text). At this 
time the only solutions are to use another browser (IE, or Netscape for 
UNIX or Apple systems) or to download from the Japanese mirror 
(http://www.fu.is.saga-u.ac.jp/~colin/gcc.html). Hopefully Geocities 
will eventually fix their problem.


2.2 Compile and Link Time Problems: General Steps

First, evaluate that your environment variables are what you expect them 
to be by running the SET command with no arguments (if you are using the 
bash shell then the output of env might also be illuminating). Do this 
immediately before you attempt a compile in the same window as the 
compile.

Secondly include the '-v' option on the gcc command line. This will give 
you far more information on what happens during the compile, especially 
important are which specs file is being used and what include file 
directories are being read, as well as the arguments to cpp and ld.

If you send me email about a problem the output of these two general 
steps will be very helpful in making a diagnosis.


2.3 Cannot exec 'cpp'

On compiling you get an error message like this:

	GCC.EXE: installation problem, cannot exec `cpp': No such file
	or directory
	GCC.EXE: Internal compiler error: program cpp got fatal signal 127

This means more or less what it says. The program cpp is the C 
preprocessor (it strips comments and interprets all those lines 
beginning in '#') and running it is the first step in compiling a C or 
C++ program. The problem here is that GCC.EXE cannot find CPP.EXE. 
Normally CPP.EXE is in the directory C:\cygnus\H-i386-cygwin32\lib\gcc-
lib\i386-cygwin32\cygnus-2.7.2-961023\. If the file is there then 
probably the GCC_EXEC_PREFIX environment variable is not correctly set.


2.4 Can't Find Include Files

You get an error like this:

	hello.c:2: No include path in which to find stdio.h

This, again, means what it says (more or less). The compiler cannot find 
the file stdio.h which is #included in the source file hello.c at line 
2. Of course the particular file names may differ in your case. If this 
is not simply a case of including a really non-existent file or 
misspelling the file name then probably your C_INCLUDE_PATH or 
CPLUS_INCLUDE_PATH environment variable is wrong. (If not, see "But the 
environment variables are right" below.)


2.5 Can't Find Libraries

At link time you get an error like this:

	ld: cannot open -lkernel32: No such file or directory

This one is a bit cryptic, mainly because the name of the file that 
can't be opened is not "-lkernel32" but "libkernel32.a". "-lname" is the 
ld command line syntax for linking the library named "libname.a". So 
basically this error is saying it can't find libkernel32.a (or whatever 
library matches the error you got). If you weren't trying to manually 
link in a library that doesn't exist or was misspelled (by accidentally 
including the 'lib' or '.a' on the command line for example) then 
probably your LIBRARY_PATH environment variable is wrong. (If not, see 
"But the environment variables are right" below.)


2.6 But the Environment Variables are Right!

You had one of the problems with not finding include files or libraries 
but the environment variables all seem to be pointing at the right 
places and the files are all there.

If you installed on a drive other than C: drive this may be your 
problem. The Cygwin DLL, and thus all the basic compiler tools, 
automatically map C: drive to (UNIX-style) '/'. Thus /cygnus is actually 
C:\cygnus. There are a few ways to fix this (without reinstalling on C: 
drive):

    - Map your actual install directory to /cygnus using mount
      (mount.exe is included with the Cygnus distribution). Simply
      type "mount D:\mydir /cygnus" (assuming you installed in the
      directory \mydir on D: drive). Similar tricks can be used for
      other directories which you may have installed on other drives.

    - Change the mount of C: to / to the actual install drive. This is
      possible by using the registry editor (regedit) included with
      Windows. Start the editor and go to the key (or folder) "My
      Computer\HKEY_CURRENT_USER\Software\Cygnus Support\CYGWIN.DLL
      setup\b15.0\mounts". Under this key there are several numbered
      keys. One of them will have the variables "native" set to "c:" and
      "unix" set to "/". Change the value of "native" to whatever drive
      you did your install on and everything should be fixed. NOTE: You
      should probably do this after a fresh boot with no Cygnus based
      programs running.


2.7 Unresolved References to _impure_ptr and/or _ctype_ etc.

At link time your code produces unresolved references to _impure_ptr, 
_ctype_ and/or _errno, among others.

This is the result of using the Cygwin header files but linking against 
the Mingw32 libraries. I have hopefully managed to fix the bug that used 
to cause this problem on any dual installation, but perhaps I haven't. 
To check you can run gcc with the -v option and see if the list of 
directories searched for include files contains any include directories 
with Cygwin headers in them. If everything is working correctly you 
should only see the directories on your C_INCLUDE_PATH in this list.

If you have this problem then you may have to modify the Mingw32 specs 
file, specifically the part that says:

*cpp:
%{posix:-D_POSIX_SOURCE} -iprefix /mingw32/include/

These are options that get passed to the C preprocessor by gcc. Consult 
the documentation for cpp and try options other than -iprefix. You may 
have to use -nostdinc and/or -nostdinc++ plus -I options to get the 
correct behavior.


2.8 My Program Doesn't Print Any Output OR My Windows Program Creates
    A Console Window

Your console application runs, but doesn't print any output, or your GUI 
application runs fine, but always creates an extra console window when 
run from Explorer or by double clicking on an icon.

These are basically two sides of the same coin. You have created a GUI 
(or console) application when you meant to create a console (or GUI) 
application. By default gcc creates console applications. If you make a 
windows GUI application with a WinMain and all that you will still get a 
console application if you don't tell gcc what to do at link time. The 
relevant options are "-windows" "-Wl,--subsystem,windows" or "-Wl,--
subsystem,console". The first two, if used on a gcc link line, will 
create a proper GUI application. The last will make sure you are making 
a console application.



3. Optimizing and Reducing Disk Space Usage

There are still vast amounts of disk space used by the Cygwin32 
installation on your hard-drive, and much of it can be removed while 
still maintaining a fully functional compiler system. The following 
sections point out which files you actually need for certain tasks, so 
that you won’t delete them.


3.1 Bare Minimum

For C only, Mingw32 compiles which don’t use the Win32 API, and if you 
don’t want to produce DLLs or do debugging with any of the GNU tools the 
list of files required is as follows:

In C:\cygnus\H-i386-cygwin32\bin:
	ar.exe, as.exe, gcc.exe, ld.exe

In C:\cygnus\H-i386-cygwin32\lib\gcc-lib\i386-cygwin32\cygnus-2.7.2-
961023:
	cc1.exe, cpp.exe, libgcc.a

In C:\cygnus\win32\lib:
	libkernel32.a

Plus all the files in C:\mingw32\lib and C:\mingw32\include and their 
subdirectories.


3.2 C++ Support

To add C++ Support to the above the following extra files are required:

In C:\cygnus\H-i386-cygwin32\lib\gcc-lib\i386-cygwin32\cygnus-2.7.2-
961023:
	cc1plus.exe

Note that this does not include support for the standard C++ libraries 
(only the C run time libraries) or for iostreams. That support is still 
only available with the Cygwin32 API.


3.3 Extra Utilities of Extreme Usefulness

Even if you do not use the bash shell or UNIX utilities in general some 
of the utilities in C:\cygnus\H-i386-cygwin32\bin are extremely useful 
for debugging and probably shouldn’t be deleted if you intend to do any 
actual programming using the system.

These include:
	dlltool.exe, gdb.exe, nm.exe, and strip.exe.


3.4 Jam

Jam is a make replacement program that I use pretty much exclusively, 
which is why you don't find any Makefile, makefile, makefile.mk or all 
that in the stuff that I do. You do find jamfiles and the occaisional 
mk.bat file. The executable of Jam is only 80 KB and the program is 
incredibly useful, so I would encourage you do download the special 
Mingw32 version and check it out. The Mingw32 version has built in rules 
for adding resources, building DLLs and import libraries as well as 
normal C and C++ files. The source code is, of course, freely available.

The actual point of this section though, is to point out that to use Jam 
you need not only the Jam executable but also rm.exe from the Cygwin 
distribution. You also might want to download rcl.exe and res2coff.exe 
as these are the helper programs Jam expects to use for resource script 
handling.


4. Legalities

All of the code in the Mingw32 package is available as public domain 
source. You may use and modify the code as you like. Of course I 
encourage you to write software which is free, either public domain or 
under the GNU Public License for example, but that is up to you. Linking 
with the libraries included with Mingw32 similarly does not impose any 
licensing restrictions on your code or binaries.

The library libgcc.a, which is linked into all code produced with GCC, 
is under a special version of the LGPL (as far as I know, you should 
check for yourself) which allows the distribution of programs which are 
simply linked with unmodified versions of libgcc.a with no licensing 
restrictions.

Thus, using Mingw32, you should be able to produce code with no 
licensing restrictions imposed by use of the compiler or libraries. The 
Cygwin32 API, and the GNU libraries are another matter and you should 
consult their license agreements.

Again I must stress that I am not a lawyer and the above statements only 
reflect my personal understanding of the situation. You would be well 
advised to consult the actual text of the appropriate copyright notices 
and license agreements if you have any concerns.


5. Support

First of all, the Mingw32 code is supplied AS IS with NO WARRANTY either 
EXPRESS or IMPLIED.

There is also no support staff standing by to take your calls. There 
are, however, a few people, including myself, using Mingw32 who might be 
able to help you. If you have problems you can email me at 
colin@bird.fu.is.saga-u.ac.jp and I will try to get back to you. No 
guarantees, but I will do my best.


6. Suggestions and Contributions

If you find a bug in the Mingw32 files themselves then feel free to 
report it, or even better to supply a fix, by emailing me at 
colin@bird.fu.is.saga-u.ac.jp. Any fixes I receive will probably go into 
the next release, and if they seem high-priority I may put the patched 
files on my web page until I can make a complete release. Please note 
that if you supply code it must be in the public domain or I cannot 
include it in Mingw32. Please attach an appropriate legal message to the 
code or otherwise make sure that there are no copyright issues. Of 
course if you just suggest a possible method for solving a problem or 
point out a bug then there should be no need for all that.

Note that the Win32 API header files are not actually part of the 
Mingw32 package. I know there are many bugs and omissions, and I try to 
keep informed about them, so I do appreciate mail pointing them out. 
However I can’t fix these problems at the source. You should send email 
to Scott Christley (the author of the GPL windows32-api) or possibly to 
Cygnus. Sending email to me might get me to mention it on my homepage or 
fix it in my personal copy of the header files, but that’s about it 
(sorry).

Aside from bug reports, suggestions for improvements, testing of the 
header files and otherwise praise or criticism is all welcome in my 
inbox.

Good luck,
Colin Peters (colin@bird.fu.is.saga-u.ac.jp)

