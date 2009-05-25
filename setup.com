$! setup files for openVMS/Alpha
$!
$ define aout [-.INCLUDE.AOUT]
$ define coff [-.INCLUDE.COFF]
$ define elf [-.INCLUDE.ELF]
$ define nlm [-.INCLUDE.NLM]
$ define opcode [-.INCLUDE.OPCODE]
$!
$! Build procedures
$!
$! Note: you need make 3.76
$ MAKE="gmake_3_76"
$ OPT=
$!
$ if (P1 .EQS. "CONFIGURE") .OR. (P1 .EQS. "ALL")
$ then
$    set def [.bfd]
$    @configure
$    set def [-.binutils]
$    @configure
$    set def [-.gas]
$    @configure
$    set def [-]
$ endif
$ if (P1 .EQS. "MAKE") .OR. (P1 .EQS. "ALL")
$ then
$   set def [.bfd]
$   'MAKE "ARCH=ALPHA" "OPT=''OPT'"
$   set def [-.libiberty]
$   'MAKE "ARCH=ALPHA" "OPT=''OPT'"
$   set def [-.opcodes]
$   'MAKE "ARCH=ALPHA" "OPT=''OPT'"
$   set def [-.binutils]
$   'MAKE "ARCH=ALPHA" "OPT=''OPT'"
$   set def [-.gas]
$   'MAKE "ARCH=ALPHA" "OPT=''OPT'"
$   set def [-]
$ endif