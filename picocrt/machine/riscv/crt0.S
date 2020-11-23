.option nopic
.text
.section .text._cstart,"ax",@progbits
.align 1
.globl _cstart
.type _cstart, @function
_cstart:

.option push
.option norelax
la gp, __global_pointer$
.option pop

#ifdef __riscv_flen
csrr t0, mstatus
// 1 << 13 = 8192
li t1, 8192
or t0, t1, t0
csrw mstatus, t0
csrwi fcsr, 0
#endif

// __start

// call to memcpy(__data_start, __data_source, __data_size)
lui a0, %hi(__data_start)
addi a0, a0, %lo(__data_start)
lui a1, %hi(__data_source)
addi a1, a1, %lo(__data_source)
lui a2, %hi(__data_size)
addi a2, a2, %lo(__data_size)
call memcpy

// call to memset(__bss_start, '\0', __bss_size)
lui a0, %hi(__bss_start)
addi a0, a0, %lo(__bss_start)
li a1, 0
lui a2, %hi(__bss_size)
addi a2, a2, %lo(__bss_size)
call memset

#ifdef PICOLIBC_TLS
// call to _set_tls(__tls_base)
lui a0, %hi(__tls_base)
addi a0, %lo(__tls_base)
call _set_tls
#endif

#ifdef HAVE_INITFINI_ARRAY
call __libc_init_array
#endif

// call to main(0, NULL)
li a0, 0
li a1, 0
call main

#ifdef HAVE_INITFINI_ARRAY
// save return value a0 in saved register s5
mv s5, a0
call __libc_fini_array
mv a0, s5
#endif

lui t0, %hi(_exit)
addi t0, t0, %lo(_exit)
beqz t0, .SKIP_EXIT
call _exit
.SKIP_EXIT:
/**
 * TODO what to do after main and exit return?
 * AFAI understood a hardware reset would be good,
 *  but there seems to be no reset instruction in riscv
 * So for now, just jump to _start again
 * (This makes sure the stack is correctly set, as opposed to _cstart)
 */
j _start
.size _cstart, .-_cstart

.section .text.init.enter,"ax",@progbits
.align 1
.globl _start
.type _start, @function
_start:
.option push
.option norelax
la sp, __stack
.option pop
j _cstart
.size _start, .-_start
