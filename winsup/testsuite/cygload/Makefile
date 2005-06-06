# Makefile for cygload

###
### MinGW options
###
CC = gcc
CFLAGS = -mno-cygwin -Wall
LINKFLAGS = -lstdc++ -Wl,-e,_cygloadCRTStartup@0

###
### MSVC options
###
ifndef MSVCDir
MSVCDir = C:/cygwin/usr/local/tools/i686_win32/vc7/Vc7
endif

CL = $(MSVCDir)/bin/cl
# If you want to look at the assembly, add "/Famsvc-cygload.asm /FAs".
MSVCCFLAGS = /nologo /GX /MDd /Zi /W4 /TP
MSVCINCLUDES = /I $(MSVCDir)/include /I $(MSVCDir)/PlatformSDK/Include
# Using /ENTRY seems to automatically invoke /NODEFAULTLIBS.
MSVCLIBS = /link /LIBPATH:$(MSVCDir)/lib /LIBPATH:$(MSVCDir)/PlatformSDK/lib \
	/ENTRY:cygloadCRTStartup uuid.lib msvcprtd.lib msvcrtd.lib \
	oldnames.lib kernel32.lib

all:	mingw-cygload.exe

mingw-cygload.exe:	cygload.cc cygload.h
	$(CC) $(CFLAGS) $< -o $@ $(LINKFLAGS)

msvc-cygload.exe:	cygload.cc cygload.h
	$(CL) $(MSVCCFLAGS) $(MSVCINCLUDES) $< /o $@ $(MSVCLIBS)

clean:
	rm -f msvc-cygload.exe msvc-cygload.ilk cygload.obj \
		msvc-cygload.pdb vc70.pdb mingw-cygload.exe
