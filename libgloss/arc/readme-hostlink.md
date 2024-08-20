MetaWare Hostlink IO
====================

Target program can use MetaWare hostlink interface to send messages to nSIM
simulator or MDB debugger.

Quick Start
-----------

To link with MetaWare hostlink library use `-specs=hl.specs` with baremetal
version of GCC for ARC processors - `arc-elf32-gcc` for ARCv1/2 targets or
`arc64-elf-gcc` for ARCv3 targets.

Consider this sample program named `hello.c`:

    #include <stdio.h>

    int main()
    {
        printf("Hello, World!\n");
        return 0;
    }

Build and run it for HS3x/4x targets:

    $ arc-elf32-gcc -mcpu=hs -specs=hl.specs hello.c -o hello.elf
    $ nsimdrv -tcf $NSIM_HOME/etc/tcf/templates/hs48_full.tcf -p nsim_hostlink=1 hello.elf
    Hello, World!

Build and run it for HS5x targets:

    $ arc64-elf-gcc -mcpu=hs5x -specs=hl.specs hello.c -o hello.elf
    $ nsimdrv -tcf $NSIM_HOME/etc/tcf/templates/hs58_full.tcf -p nsim_hostlink=1 hello.elf
    Hello, World!

Build and run it for HS6x targets:

    $ arc64-elf-gcc -mcpu=hs6x -specs=hl.specs hello.c -o hello.elf
    $ nsimdrv -tcf $NSIM_HOME/etc/tcf/templates/hs68_full.tcf -p nsim_hostlink=1 hello.elf
    Hello, World!

Contents
--------

* `hl/hl_gw.*` - Hostlink gateway. This API is used in the `hl_api.*`.
  Please use `hl_message()` from `hl_api.*` for hostlink message exchange.
* `hl/hl_api.*` - High-level API to send hostlink messages, as well as
  functions to work with messages.
* `hl/hl_<syscall>.*` - System calls implementations through hostlink API.
* `arc-timer.*` - Provides API to access ARC timers. Used by `hl/hl_clock.c`
  in `_clock()` implementation.
* `arc-main-helper.c` - Provides `__setup_argv_and_call_main()`. The function
  is called from `__start()` in `crt0.S`. It allows to setup `argc` and `arvg`
  as well as some custom things through `_setup_low_level()`.
* `hl-setup.c` - Provides `_setup_low_level()` for hostlink case. It just
  configures default timer if it exists. Default timer is used in the hostlink
  `clock()` implementation.
* `hl-stub.c` - Provides functions which are part of newlib but implemented
  without hostlink. For example, `_kill()` and `_getpid()`.
* `sbrk.c` - Provides `_sbrk()`. It uses `__start_heap` and `__end_heap`
  variables.
* `libcfunc.c` - Additional C system calls.
* `mcount.c` - Profiler support.

How It Works
------------

Simulator looks for `__HOSTLINK__` and `_hl_blockedPeek()` symbols.
`__HOSTLINK__` is the start of shared structure for message exchange and
`_hl_blockedPeek()` is a function to be called when program is waiting
for simulator response.

When program wants to send a message it should follow:

1. Fill `__HOSTLINK__.payload` with packed data. Packing format is following:
   `{u16 type, u16 size, char data[]}`. Supported types are `char`, `short`,
   `int`, `string` and `int64`. `hl_api` provides high-level API to this.
2. Fill `__HOSTLINK__.pkt_hdr`. See `hl_pkt_init()` from `hl_gw.c`.
3. Fill `__HOSTLINK__.hdr`. See `hl_send()` from `hl_gw.c`.
4. Call `_hl_blockedPeek()` to get response. At this point message should be
   delivered to debugger. Some implementations uses change of
   `__HOSTLINK__.hdr.target2host_addr` as a signal that packet is sent and can
   be processed. Other implementations wait for `_hl_blockedPeek()` to be
   called. It means that portable implementation must fill
   `__HOSTLINK__.hdr.target2host_addr` at the last step and then call
   `_hl_blockedPeek()`.
5. `_hl_blockedPeek()` returns pointer to debugger response which can be
   processed on target if needed. Because debugger and target share the same
   buffer the function actually returns `__HOSTLINK__.payload` that was
   filled with packed data (see step 1) by the debugger.
