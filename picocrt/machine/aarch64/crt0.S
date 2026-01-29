#ifndef CRT0_LINUX
/************ Page table ************/
/*
 * The smallest VA we can construct is 8GB, which needs 8 block page table
 * entries, each covering 1GiB.
 */
#define MMU_BLOCK_COUNT       8

#define MMU_DESCRIPTOR_VALID    (1 << 0)
#define MMU_DESCRIPTOR_BLOCK    (0 << 1)
#define MMU_DESCRIPTOR_TABLE    (1 << 1)

#define MMU_BLOCK_XN            (1LL << 54)
#define MMU_BLOCK_PXN           (1LL << 53)
#define MMU_BLOCK_CONTIG        (1LL << 52)
#define MMU_BLOCK_DBM           (1LL << 51)
#define MMU_BLOCK_GP            (1LL << 50)

#define MMU_BLOCK_NT            (1 << 16)
#define MMU_BLOCK_OA_BIT        12
#define MMU_BLOCK_NG            (1 << 11)
#define MMU_BLOCK_AF            (1 << 10)
#define MMU_BLOCK_SH_BIT        8
#define MMU_BLOCK_SH_NS         (0 << MMU_BLOCK_SH_BIT)
#define MMU_BLOCK_SH_OS         (2 << MMU_BLOCK_SH_BIT)
#define MMU_BLOCK_SH_IS         (3 << MMU_BLOCK_SH_BIT)
#define MMU_BLOCK_AP_BIT        6
#define MMU_BLOCK_NS            (1 << 5)
#define MMU_BLOCK_ATTR_BIT      2

#define MMU_NORMAL_FLAGS        (MMU_DESCRIPTOR_VALID |         \
                                 MMU_DESCRIPTOR_BLOCK |         \
                                 MMU_BLOCK_AF |                 \
                                 MMU_BLOCK_SH_IS |              \
                                 (0 << MMU_BLOCK_ATTR_BIT))

#define MMU_DEVICE_FLAGS        (MMU_DESCRIPTOR_VALID | \
                                 MMU_DESCRIPTOR_BLOCK | \
                                 MMU_BLOCK_AF | \
                                 (1 << MMU_BLOCK_ATTR_BIT))

#define MMU_INVALID_FLAGS       0

  .macro start_page_table
  .section .rodata
  .global __identity_page_table
  .balign 65536
__identity_page_table:
  .set block_num, 0
  .endm

  .macro page_table_entries count, flags
  .rept \count
    .8byte (block_num << 30) | \flags
    .set block_num, block_num + 1
  .endr
  .endm

  .macro end_page_table
  .size __identity_page_table, MMU_BLOCK_COUNT * 8
  .if block_num != MMU_BLOCK_COUNT
  .error "Wrong number of page table entries"
  .endif
  .endm

#if defined(MACHINE_qemu)
  start_page_table
  // [0x0000_0000,0x8000_0000): 2GiB normal memory
  page_table_entries 2, MMU_NORMAL_FLAGS
  // [0x8000_0000,0x1_0000_0000): 2GiB device memory
  page_table_entries 2, MMU_DEVICE_FLAGS
  // [0x1_0000_0000,0x2_0000_0000): 4GiB un-mapped
  page_table_entries 4, MMU_INVALID_FLAGS
  end_page_table
#elif defined(MACHINE_fvp)
  start_page_table
  // [0x0000_0000,0x8000_0000): 2GiB unmapped. This actually contains a lot
  // of different memory regions and devices, but we don't need any of them
  // for testing.
  page_table_entries 2, MMU_INVALID_FLAGS
  // [0x8000_0000,0x1_0000_0000): 2GiB normal memory
  page_table_entries 2, MMU_NORMAL_FLAGS
  // [0x1_0000_0000,0x2_0000_0000): 4GiB un-mapped
  page_table_entries 4, MMU_INVALID_FLAGS
  end_page_table
#else
#error "Unknown machine type"
#endif

#if defined(MACHINE_qemu)
  #define BOOT_EL(Reg) Reg ## _EL1
#elif defined(MACHINE_fvp) && __ARM_ARCH_PROFILE == 'R'
  #define BOOT_EL(Reg) Reg ## _EL2
#elif defined(MACHINE_fvp) && __ARM_ARCH_PROFILE != 'R'
  #define BOOT_EL(Reg) Reg ## _EL3
#else
#error "Unknown machine type"
#endif
#endif /* CRT0_LINUX */


/************ Entry point ************/

  // Defined in crt0.c
  .global _cstart
  .type cstart, %function

  // _start: Main entry point function, sets up the hardware to the point where
  // we can execute C code.
  .section .text.init.enter, "ax", %progbits
  .global _start
  .type _start, %function
_start:
#ifdef CRT0_LINUX
        mov x0, sp
#else
  /* Use EL-banked stack pointer */
	msr     SPSel, #1
#endif

#ifdef __pic__
        adrp x1, :got:__stack
        ldr  x1, [x1, :got_lo12:__stack]
#else
	/* Initialize stack */
	adrp x1, __stack
	add  x1, x1, :lo12:__stack
#endif
	mov sp, x1

#ifndef CRT0_LINUX
	/* Enable FPU */
#if __ARM_FP
#if defined(MACHINE_qemu)
	mov x1, #(0x3 << 20)
	msr BOOT_EL(CPACR), x1
#elif defined(MACHINE_fvp)
	mrs x0, BOOT_EL(CPTR)
  /* Clear CPTR_ELx.TFP, to enable FP/SIMD instructions at EL0 and EL1. */
	and x0, x0, #~(1<<10)
  /* Set CPTR_ELx.EZ and .ESM, to enable SVE and SME instructions at EL3. These
   * bits are ignored for cores which don't have the relevant feature. */
  ORR x0, x0, #1<<8
	ORR x0, x0, #1<<12
	msr BOOT_EL(CPTR), x0
#else
#error "Unknown machine type"
#endif
#endif // __ARM_FP
#endif

	/* Jump into C code */
	bl _cstart
  .size _start, .-_start

#ifndef CRT0_LINUX

/************ Exception handlers ************/
#ifdef CRT0_SEMIHOST

  .macro vector_common
  sub sp, sp, #256
  str x0, [sp, #0]
  str x1, [sp, #8]
  str x2, [sp, #16]
  str x3, [sp, #24]
  str x4, [sp, #32]
  str x5, [sp, #40]
  str x6, [sp, #48]
  str x7, [sp, #56]
  str x8, [sp, #64]
  str x9, [sp, #72]
  str x10, [sp, #80]
  str x11, [sp, #88]
  str x12, [sp, #96]
  str x13, [sp, #104]
  str x14, [sp, #112]
  str x15, [sp, #120]
  str x16, [sp, #128]
  str x17, [sp, #136]
  str x18, [sp, #144]
  str x19, [sp, #152]
  str x20, [sp, #160]
  str x21, [sp, #168]
  str x22, [sp, #176]
  str x23, [sp, #184]
  str x24, [sp, #192]
  str x25, [sp, #200]
  str x26, [sp, #208]
  str x27, [sp, #216]
  str x28, [sp, #224]
  str x29, [sp, #232]
  str x30, [sp, #240]
#if defined(MACHINE_qemu)
  mrs x0, BOOT_EL(ELR)
  str x0, [sp, #248]
  mrs x0, BOOT_EL(ESR)
  str x0, [sp, #256]
  mrs x0, BOOT_EL(FAR)
  str x0, [sp, #264]
#elif defined(MACHINE_fvp)
  mrs x0, BOOT_EL(ELR)
  str x0, [sp, #248]
  mrs x0, BOOT_EL(ESR)
  str x0, [sp, #256]
  mrs x0, BOOT_EL(FAR)
  str x0, [sp, #264]
#else
#error "Unknown machine type"
#endif
  mov x0, sp
  .endm

  .global aarch64_fault
  .type aarch64_fault, %function

  .macro exception_handler name, number
  .section .init, "ax", %progbits
  .global aarch64_\name\()_vector
  .type aarch64_\name\()_vector, %function
aarch64_\name\()_vector:
  vector_common
  mov x1, #\number
  b  aarch64_fault
  .endm

  exception_handler sync, 0
  exception_handler irq, 1
  exception_handler fiq, 2
  exception_handler serror, 3

#endif // CRT0_SEMIHOST
#endif // CRT0_LINUX
