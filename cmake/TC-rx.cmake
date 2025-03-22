# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_SYSTEM_PROCESSOR rx)

# which compilers to use for C
set(TARGET_COMPILE_OPTIONS -ffinite-math-only)
set(CMAKE_C_COMPILER rx-zephyr-elf-gcc -nostdlib)

# where is the target environment located
set(CMAKE_FIND_ROOT_PATH  /opt/rx-zephyr-elf)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(__THREAD_LOCAL_STORAGE_API FALSE)
set(__THREAD_LOCAL_STORAGE FALSE)

set(TEST_RUNNER run-rx)

set(PICOLIBC_LINK_FLAGS -nostartfiles -T ${CMAKE_CURRENT_SOURCE_DIR}/cmake/TC-rx.ld -lgcc)
