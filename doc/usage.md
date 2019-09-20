### Using the library

We should configure the compiler so that selecting a suitable target
architecture combination would set up the library paths to match, but
at this point you'll have to figure out the right -L line by yourself
by matching the path name on the left side of the --print-multi-lib
output with the compiler options on the right side. For instance, my
STM32F042 cortex-M0 parts use

    $ arm-none-eabi-gcc -mlittle-endian -mcpu=cortex-m0 -mthumb

To gcc, '-mcpu=cortex=m0' is the same as '-march=armv6s-m', so looking
at the output above, the libraries we want are in

    /usr/local/lib/newlib-nano/arm-none-eabi/lib/thumb/v6-m

so, to link, we need to use:

    $ arm-none-eabi-gcc ... -L/usr/local/lib/newlib-nano/arm-none-eabi/lib/thumb/v6-m -lm -lc -lgcc

