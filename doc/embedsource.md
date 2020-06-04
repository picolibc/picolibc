# Using picolibc as an embedded source

For projects where deterministic builds, reproduceability and traceability is important, it is possible to embed the entire picolibc library as a meson subproject into the source directory of the main project. The benefit is that git submodules and meson subprojects work nicely together, and it is possible to lock down the libc version together with the source tree.

For more information on the meson subproject feature, refer to the documentation at : https://mesonbuild.com/Subprojects.html

## Adding picolibc as a subproject

First you need to add picolibc as a git submodule:

  ```
  mkdir subprojects
  git submodule add https://github.com/keith-packard/picolibc subprojects/picolibc
  ```

Your source directory could now look something like this:

  ```
  subprojects/
    picolibc/
    otherlib/
  src/
    main.c
  meson.build
  ```

## Add dependency to meson.build

First of all, when using picolibc as an embedded source, you should not specify -specs=picolibc.specs in the c_args.

Next you will need to include the dependency in your meson.build file:

  ```
  dependencies += dependency('picolibc', fallback: ['picolibc', 'picolibc_dep'])
  ```
  
In order to avoid linker conflicts with any compiler bundled c-library, use the following link flags (also see the warning below):

  ```
  link_args += '-nolibc'
  link_args += '-lgcc'
  ```
  
Finally use the dependency and linker arguments on your executable

  ```
  exec = executable('myproject.elf', sources, dependencies: dependencies, link_args: link_args)
  ```

## Warning: Remove bundled libc (newlib)

As of today there is no option in gcc to ignore the bundled c-library. The development version GCC 9.0 will have an `-nolibc` option that we are waiting for. So currently the only way to avoid including the headers of the bundled library is to use the `-nostdinc` option. However, this also removes all the include directories, including the headers for libgcc, which we want. And i have found no good poartable way of re-adding the libgcc include header path.

So until a better solution appears, it is best to uninstall the c-library if possible. On Debian/Ubuntu, the gcc-arm-none-eabi and libnewlib-arm-none-eabi are separate packages, so just uninstall libnewlib and any conflict is avoided.

You can run a command in order to check which standard paths are included with your GCC:

  ```
  echo | arm-none-eabi-gcc -E -Wp,-v -
    /usr/lib/gcc/arm-none-eabi/7.3.1/include
    /usr/lib/gcc/arm-none-eabi/7.3.1/include-fixed
    /usr/lib/gcc/arm-none-eabi/7.3.1/../../../arm-none-eabi/include <-- This is newlib (configured through ubuntu /etc/alternatives)
  
  sudo apt remove libnewlib-arm-none-eabi libnewlib-dev
  
  echo | arm-none-eabi-gcc -E -Wp,-v -
    /usr/lib/gcc/arm-none-eabi/7.3.1/include
    /usr/lib/gcc/arm-none-eabi/7.3.1/include-fixed
  
