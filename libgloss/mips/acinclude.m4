MIPS_PART_SPECIFIC_OBJ=
MIPS_PART_SPECIFIC_DEFINES=
MIPS_SCRIPT_LIST=
MIPS_BSP_LIST=
MIPS_CRT0=crt0.o

case "${target}" in
  mips*-tx39*-*|mipstx39*-*-*)
    MIPS_SCRIPT_LIST="dve idt jmr3904app jmr3904dram jmr3904dram-java jmr3904app-java sde32 sde64 mti32 mti64 mti64_n32 mti64_64"
    MIPS_BSP_LIST="libdve.a libidt.a libjmr3904.a"
    ;;
  mipsisa32-*-* | mipsisa32el-*-* | \
  mipsisa32r2-*-* | mipsisa32r2el-*-* | \
  mipsisa64*-*-*)
    MIPS_CRT0="crt0_cfe.o crt0_cygmon.o crt0.o"
    MIPS_SCRIPT_LIST="idt32 idt64 cfe"
    MIPS_BSP_LIST="libcygmon.a libidt.a libcfe.a"
    ;;
  mips*-lsi*-*)
    MIPS_PART_SPECIFIC_OBJ="entry.o"
    MIPS_SCRIPT_LIST="lsi"
    MIPS_BSP_LIST=liblsi.a
    ;;
  mips64vr5*-*-*)
    MIPS_PART_SPECIFIC_OBJ="vr5xxx.o cma101.o"
    MIPS_PART_SPECIFIC_DEFINES=-DR5000
    MIPS_SCRIPT_LIST="idt pmon ddb ddb-kseg0 lsi idtecoff nullmon"
    MIPS_BSP_LIST="libidt.a libpmon.a liblsi.a libnullmon.a"
    ;;
  mips64vr-*-* | mips64vrel-*-*)
    MIPS_PART_SPECIFIC_OBJ="vr5xxx.o cma101.o"
    MIPS_SCRIPT_LIST="ddb ddb-kseg0 nullmon"
    MIPS_BSP_LIST="libpmon.a libnullmon.a"
    ;;
  mips*)
    MIPS_CRT0="crt0_cfe.o crt0.o"
    MIPS_PART_SPECIFIC_OBJ="vr4300.o cma101.o"
    MIPS_SCRIPT_LIST="idt pmon ddb ddb-kseg0 lsi cfe idtecoff nullmon sde32 sde64 mti32 mti64 mti64_n32 mti64_64"
    MIPS_BSP_LIST="libidt.a libpmon.a liblsi.a libcfe.a libnullmon.a"
    ;;
esac

AC_SUBST(MIPS_PART_SPECIFIC_OBJ)
AC_SUBST(MIPS_PART_SPECIFIC_DEFINES)
AC_SUBST(MIPS_SCRIPT_LIST)
AC_SUBST(MIPS_BSP_LIST)
AC_SUBST(MIPS_CRT0)
