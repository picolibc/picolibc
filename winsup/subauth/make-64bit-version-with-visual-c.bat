rem This batchfile shows how to generate a 64 bit version of cygsuba.dll.
rem The 32 bit version will not work on 64 bit systems.
rem
rem This can be used as long as no x86_64-pe/coff capable gcc is available.
rem Note that this is for building inside the source dir as not to interfere
rem with the "official" 32 bit build in the build directory.
rem
rem Install the dll into the 64 bit $SYSTEMROOT.
rem
sed -e 's/ = .*$//' < cygsuba.din > cygsuba.def
cl /LDd /Wp64 /Fecygsuba.dll cygsuba.c /link /def:cygsuba.def
