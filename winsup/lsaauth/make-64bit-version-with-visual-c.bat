@echo off
rem This batchfile shows how to generate a 64 bit version of cyglsa.dll.
rem The 32 bit version will not work on 64 bit systems.
rem
rem Note that you need not only the SDK headers and libs, but also the
rem 64 bit ntdll.lib file from a DDK supporting 64 bit builds.
rem
rem This can be used as long as no x86_64-pe/coff capable gcc is available.
rem Note that this is for building inside the source dir as not to interfere
rem with the "official" 32 bit build in the build directory.
rem
rem Install the dll into /bin and use the cyglsa-config script to register it.
rem Don't forget to reboot afterwards.
rem
cl /Wp64 /c cyglsa.c
link /nodefaultlib /dll /machine:x64 /entry:DllMain /out:cyglsa64.dll /def:mslsa.def cyglsa.obj runtmchk.lib advapi32.lib kernel32.lib ntdll.lib
