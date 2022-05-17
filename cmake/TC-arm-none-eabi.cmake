# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_SYSTEM_PROCESSOR arm)

# which compilers to use for C
set(TARGET_COMPILE_OPTIONS -mthumb -march=armv7-m -mfloat-abi=soft)
set(CMAKE_C_COMPILER arm-none-eabi-gcc -nostdlib)

# where is the target environment located
set(CMAKE_FIND_ROOT_PATH  /usr/bin)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(_HAVE_PICOLIBC_TLS_API TRUE)

set(FORMAT_DEFAULT_DOUBLE 1)
set(FORMAT_DEFAULT_FLOAT 0)
set(FORMAT_DEFAULT_INTEGER 0)

set(PICOLIBC_LINK_FLAGS -nostartfiles -T ${CMAKE_CURRENT_SOURCE_DIR}/cmake/TC-arm-none-eabi.ld -lgcc)
