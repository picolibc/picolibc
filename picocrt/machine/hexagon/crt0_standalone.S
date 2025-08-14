    .section .text.init.enter, "awx", @progbits
    .global _start
    .type _start, @function
_start:
    jump hexagon_start_init
    .space _start + 0x180 - .

    .text
    .p2align 4

    .type event_handle_reset, @function
event_handle_reset:
    r0 = #0
    stid = r0
    jump coredump
    .size event_handle_reset, . - event_handle_reset

    .type event_handle_nmi, @function
event_handle_nmi:
    r0 = #1
    stid = r0
    jump coredump
    .size event_handle_nmi, . - event_handle_nmi

    .type event_handle_error, @function
event_handle_error:
    r0 = #2
    stid = r0
    jump coredump
    .size event_handle_error, . - event_handle_error

    .type event_handle_rsvd, @function
event_handle_rsvd:
    r0 = ##0xdeadbeef
    stid = r0
    jump coredump
    .size event_handle_rsvd, . - event_handle_rsvd

    .type event_handle_tlbmissx, @function
event_handle_tlbmissx:
    r0 = #0
    stid = r0
    jump coredump
    .size event_handle_tlbmissx, . - event_handle_tlbmissx

    .type event_handle_tlbmissrw, @function
event_handle_tlbmissrw:
    r0 = #0
    stid = r0
    jump coredump
    .size event_handle_tlbmissrw, . - event_handle_tlbmissrw

    .type event_handle_trap0, @function
event_handle_trap0:
    rte
    .size event_handle_trap0, . - event_handle_trap0

    .type event_handle_trap1, @function
event_handle_trap1:
    r0 = #9
    stid = r0
    jump coredump
    .size event_handle_trap1, . - event_handle_trap1

    .type event_handle_int, @function
event_handle_int:
    r0 = #2
    stid = r0
    jump coredump
    .size event_handle_int, . - event_handle_int

    .p2align 12, 0
    .type .EventVectors, %function
.EventVectors:
    jump event_handle_reset
    jump event_handle_nmi
    jump event_handle_error
    jump event_handle_rsvd
    jump event_handle_tlbmissx
    jump event_handle_rsvd
    jump event_handle_tlbmissrw
    jump event_handle_rsvd
    jump event_handle_trap0
    jump event_handle_trap1
    jump event_handle_rsvd /* 10 */
    jump event_handle_rsvd /* 11 */
    jump event_handle_rsvd /* 12 */
    jump event_handle_rsvd /* 13 */
    jump event_handle_rsvd /* 14 */
    jump event_handle_rsvd /* 15 */
    jump event_handle_int  /* Event number 16, Interrupt 0 */
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int
    jump event_handle_int  /* Event number 47, Interrupt 31 */
    .size .EventVectors, . - .EventVectors

// TODO: Merge this code with reset vector.
hexagon_start_init:
    { r0 = #0
      r1 = #0 }
    { r3:2 = r1:0
      r5:4 = r1:0
      r7:6 = r1:0
      r9:8 = r1:0 }
    { r11:10 = r1:0
      r13:12 = r1:0
      r15:14 = r1:0
      r17:16 = r1:0 }
    { r19:18 = r1:0
      r21:20 = r1:0
      r23:22 = r1:0
      r25:24 = r1:0 }
    { r27:26 = r1:0
      r29:28 = r1:0
      r31:30 = r1:0 }
    dckill
    isync
    ickill
    isync
    r0 = ##.EventVectors
    EVB = r0
    r0 = ##__stack
    sp = and (r0, #-16)
    r0 = #0
    framekey = r0
	fp = #0
    jump hexagon_start_init1
    .size .hexagon_start_init, . - .hexagon_start_init
