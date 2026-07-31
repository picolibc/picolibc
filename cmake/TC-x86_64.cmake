# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_SYSTEM_PROCESSOR x86)

# which compilers to use for C
set(TARGET_COMPILE_OPTIONS
  -march=core2
  -mfpmath=sse
  -msse2
  -static
  -fno-pic
  -fno-PIE
  -nostdlib
  -nostartfiles
  -Wl,--build-id=none
)
set(CMAKE_C_COMPILER x86_64-linux-gnu-gcc)

# where is the target environment located
set(CMAKE_FIND_ROOT_PATH  /usr/bin)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(__THREAD_LOCAL_STORAGE_API TRUE)

set(TEST_RUNNER run-x86)

set(PICOLIBC_LINK_FLAGS -nostartfiles -T ${CMAKE_CURRENT_SOURCE_DIR}/cmake/TC-x86_64.ld -lgcc)
