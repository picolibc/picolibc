MEMORY
{
  VECT0 (w)     : ORIGIN = 0x0FFC0, LENGTH = 2
  VECT1 (w)     : ORIGIN = 0x0FFC2, LENGTH = 2
  VECT2 (w)     : ORIGIN = 0x0FFC4, LENGTH = 2
  VECT3 (w)     : ORIGIN = 0x0FFC6, LENGTH = 2
  VECT4 (w)     : ORIGIN = 0x0FFC8, LENGTH = 2
  VECT5 (w)     : ORIGIN = 0x0FFCA, LENGTH = 2
  VECT6 (w)     : ORIGIN = 0x0FFCC, LENGTH = 2
  VECT7 (w)     : ORIGIN = 0x0FFCE, LENGTH = 2
  VECT8 (w)     : ORIGIN = 0x0FFD0, LENGTH = 2
  VECT9 (w)     : ORIGIN = 0x0FFD2, LENGTH = 2
  VECT10 (w)    : ORIGIN = 0x0FFD4, LENGTH = 2
  VECT11 (w)    : ORIGIN = 0x0FFD6, LENGTH = 2
  VECT12 (w)    : ORIGIN = 0x0FFD8, LENGTH = 2
  VECT13 (w)    : ORIGIN = 0x0FFDA, LENGTH = 2
  VECT14 (w)    : ORIGIN = 0x0FFDC, LENGTH = 2
  VECT15 (w)    : ORIGIN = 0x0FFDE, LENGTH = 2
  VECT16 (w)    : ORIGIN = 0x0FFE0, LENGTH = 2
  VECT17 (w)    : ORIGIN = 0x0FFE2, LENGTH = 2
  VECT18 (w)    : ORIGIN = 0x0FFE4, LENGTH = 2
  VECT19 (w)    : ORIGIN = 0x0FFE6, LENGTH = 2
  VECT20 (w)    : ORIGIN = 0x0FFE8, LENGTH = 2
  VECT21 (w)    : ORIGIN = 0x0FFEA, LENGTH = 2
  VECT22 (w)    : ORIGIN = 0x0FFEC, LENGTH = 2
  VECT23 (w)    : ORIGIN = 0x0FFEE, LENGTH = 2
  VECT24 (w)    : ORIGIN = 0x0FFF0, LENGTH = 2
  VECT25 (w)    : ORIGIN = 0x0FFF2, LENGTH = 2
  VECT26 (w)    : ORIGIN = 0x0FFF4, LENGTH = 2
  VECT27 (w)    : ORIGIN = 0x0FFF6, LENGTH = 2
  VECT28 (w)    : ORIGIN = 0x0FFF8, LENGTH = 2
  VECT29 (w)    : ORIGIN = 0x0FFFA, LENGTH = 2
  VECT30 (w)    : ORIGIN = 0x0FFFC, LENGTH = 2
  VECT31 (w)    : ORIGIN = 0x0FFFE, LENGTH = 2
}

SECTIONS
{
  __interrupt_vector_0  : { KEEP (*(__interrupt_vector_0))  } > VECT0
  __interrupt_vector_1  : { KEEP (*(__interrupt_vector_1))  } > VECT1
  __interrupt_vector_2  : { KEEP (*(__interrupt_vector_2))  } > VECT2
  __interrupt_vector_3  : { KEEP (*(__interrupt_vector_3))  } > VECT3
  __interrupt_vector_4  : { KEEP (*(__interrupt_vector_4))  } > VECT4
  __interrupt_vector_5  : { KEEP (*(__interrupt_vector_5))  } > VECT5
  __interrupt_vector_6  : { KEEP (*(__interrupt_vector_6))  } > VECT6
  __interrupt_vector_7  : { KEEP (*(__interrupt_vector_7))  } > VECT7
  __interrupt_vector_8  : { KEEP (*(__interrupt_vector_8))  } > VECT8
  __interrupt_vector_9  : { KEEP (*(__interrupt_vector_9))  } > VECT9
  __interrupt_vector_10 : { KEEP (*(__interrupt_vector_10)) } > VECT10
  __interrupt_vector_11 : { KEEP (*(__interrupt_vector_11)) } > VECT11
  __interrupt_vector_12 : { KEEP (*(__interrupt_vector_12)) } > VECT12
  __interrupt_vector_13 : { KEEP (*(__interrupt_vector_13)) } > VECT13
  __interrupt_vector_14 : { KEEP (*(__interrupt_vector_14)) } > VECT14
  __interrupt_vector_15 : { KEEP (*(__interrupt_vector_15)) } > VECT15
  __interrupt_vector_16 : { KEEP (*(__interrupt_vector_16)) } > VECT16
  __interrupt_vector_17 : { KEEP (*(__interrupt_vector_17)) } > VECT17
  __interrupt_vector_18 : { KEEP (*(__interrupt_vector_18)) } > VECT18
  __interrupt_vector_19 : { KEEP (*(__interrupt_vector_19)) } > VECT19
  __interrupt_vector_20 : { KEEP (*(__interrupt_vector_20)) } > VECT20
  __interrupt_vector_21 : { KEEP (*(__interrupt_vector_21)) } > VECT21
  __interrupt_vector_22 : { KEEP (*(__interrupt_vector_22)) } > VECT22
  __interrupt_vector_23 : { KEEP (*(__interrupt_vector_23)) } > VECT23
  __interrupt_vector_24 : { KEEP (*(__interrupt_vector_24)) } > VECT24
  __interrupt_vector_25 : { KEEP (*(__interrupt_vector_25)) } > VECT25
  __interrupt_vector_26 : { KEEP (*(__interrupt_vector_26)) KEEP (*(__interrupt_vector_watchdog)) } > VECT26
  __interrupt_vector_27 : { KEEP (*(__interrupt_vector_27)) } > VECT27
  __interrupt_vector_28 : { KEEP (*(__interrupt_vector_28)) } > VECT28
  __interrupt_vector_29 : { KEEP (*(__interrupt_vector_29)) } > VECT29
  __interrupt_vector_30 : { KEEP (*(__interrupt_vector_30)) KEEP (*(__interrupt_vector_nmi)) } > VECT30
  __interrupt_vector_31 : { KEEP (*(__interrupt_vector_31)) KEEP (*(__interrupt_vector_reset)) KEEP (*(.resetvec)) } > VECT31
}
