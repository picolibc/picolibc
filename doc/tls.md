# Thread Local Storage in Picolibc
Copyright © 2019 Keith Packard

The standard C library API includes many functions that use persistent
state held by the library not the application. One obvious example is
'errno', a global variable originall designed to hold error status
from the Unix kernel, but which was co-opted by libc to provide
additional error status values from the library itself. There are
numerous other examples of this kind of state.

To permit multiple threads sharing a single address space to use the
library without conflicting over this data, each thread needs a
private copy of data that it uses. Newlib did this by creating a
global structure holding all thread-specific values and then defining
an API for the library to get the global structure for the current
thread.

Picolibc does this by using the built-in thread-local-storage
mechanisms in the toolchain. This has several benefits:

 1) Source code is simpler as thread-local variables are
    accessed directly by name.

 2) Thread local storage contains only values used by the
    application.

 3) Generated code is smaller and faster.

## TLS model used by Picolibc

Picolibc is normally compiled with -ftls-model=local-exec. The selected
model is included in the .specs file so applications should not
include -ftls-model in their compile commands.

Local-exec defines a single block of storage for all TLS
variables. Each TLS variable is assigned an offset within this block
and those values are placed in the binary during relocation.

## Initial TLS block

The sample Picolibc linker script (picolibc.ld) allocates RAM for an
initial TLS block and arranges for it to be initialized as a part of
the normal data/bss initialization process for the application.

| flash | symbol |
| ----- | ------ |
| code  |        |
| rodata |       | 
| data initializers | __data_source |
| TLS data initializers | __tdata_source |

| RAM  | symbol |
| ---- | ------ |
| data | __data_start |
| TLS data | __tls_base |
|          | __data_end |
|          | __tdata_size = . - __tls_base |
| TLS bss | __bss_start |
|         | __tbss_size = . - __bss_start |
|         | __tls_size = . - __tls_base  |
| bss | |
|     | __bss_end 

The crt0 code copies __data_end - __data_start bytes from _data_source
to _data_start. This initializes the regular data segment *and* the
initial TLS data segment. Then, it clears memory from __bss_start to
__bss_end, initializing the TLS bss segment *and* the regular bss
segment. Finally, it sets the architecture-specific TLS data pointer
to __tls_base. Once that is set, access to TLS variables will
reference this initial TLS block.

## Creating more TLS blocks

If the application is multi-threaded and wants to allow these threads
to have separate TLS data, it may allocate memory for additional TLS
blocks:

 1) Allocate a block of size  __tls_size
 2) Copy __tdata_size bytes from __tdata_source to the new block to
    set the initial TLS values.
 3) Clear __tbss_size bytes starting _tdata_size bytes into the new
    block
 4) Set the TLS pointer as necessary

## Picolibc APIs related to TLS

Picolib provides a couple of helper APIs for TLS:

* _set_tls
```
void
_set_tls(void *tls);
```
This is an architecture-specific function which sets the TLS
block pointer for the processor to `tls`.

* _init_tls
```
void
_init_tls(void *tls);
```
This function initializes the specified TLS block, copying values
into the initialized data portion and clearing values in the
uninitialized data portion.

Picolib also provides architecture-specific internal GCC APIs as
necessary, for example, __aeabi_read_tp for ARM processors.
