@echo off
rem This batchfile shows how to generate a 64 bit version of cyglsa.dll.
rem The 32 bit version will not work on 64 bit systems.
rem
rem Note that you need not only the SDK headers and libs, but also the
rem 64 bit ntdll.lib file from a DDK supporting 64 bit builds.
rem
rem Make sure all necessary include paths are set in %Include% (inc\ddk,
rem inc\atl, inc\crt) and make sure that %Lib% points to the 64 bit libs, not
rem to the 32 bit libs.  In the latter case the link stage will succeed,
rem but the resulting DLL is non-functional.
rem
rem This can be used as long as no x86_64-pe/coff capable gcc is available.
rem Note that this is for building inside the source dir as not to interfere
rem with the "official" 32 bit build in the build directory.
rem
rem Install the dll into /bin and use the cyglsa-config script to register it.
rem Don't forget to reboot afterwards.
rem
rem Use "/DDEBUGGING" in the cl line to create debugging output to
rem C:\cyglsa.dbgout at runtime.
rem
rem No idea when that changed, but in the latest SDKs you have to disable
rem the security checks and there's apparently no runtmchk.lib anymore.
rem I leave the old statements in for reference.
rem cl /Wp64 /c cyglsa.c
rem link /nodefaultlib /dll /machine:x64 /entry:DllMain /out:cyglsa64.dll /def:mslsa.def cyglsa.obj runtmchk.lib advapi32.lib kernel32.lib ntdll.lib
cl /Wp64 /EHs-c- /GS- /GR- /GL- /c cyglsa.c
link /nodefaultlib /dll /machine:x64 /entry:DllMain /out:cyglsa64.dll /def:mslsa.def cyglsa.obj advapi32.lib kernel32.lib ntdll.lib
