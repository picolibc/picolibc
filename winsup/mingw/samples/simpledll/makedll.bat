rem *** Create the import library for the dll ***
dlltool --dllname dll.dll --def dll.def --output-lib libdll.a  

rem *** Compile the dll ***
gcc -c -o dll.o  dll.c 

rem *** Link the dll ***
gcc -s -mdll -o dll.dll -Wl,--base-file,dll.b dll.o
dlltool --dllname dll.dll --base-file dll.b --output-exp dll.e --def dll.def 
gcc -s -mdll -o dll.dll -Wl,--base-file,dll.b dll.o -Wl,dll.e 
dlltool --dllname dll.dll --base-file dll.b --output-exp dll.e --def dll.def 
gcc -s -mdll -o dll.dll dll.o -Wl,dll.e 

rem *** Delete temporary files from dll linking ***
del dll.b 
del dll.e 

rem *** Compile exe, which uses dll. ***
gcc -c -o exe.o exe.c 

rem *** Link exe.exe, which uses dll.dll ***
gcc -s -L. -o exe.exe exe.o libdll.a

