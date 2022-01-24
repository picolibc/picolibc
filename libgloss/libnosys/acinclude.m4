dnl Make sure syscall names match those being used by newlib
case "${target}" in
  *-*-cygwin*) ;;
  a29k-amd-udi) ;;
  aarch64*-*-*) ;;
  arc-*-*) ;;
  arm*-*-*) ;;
  bfin-*-*) ;;
  cris-*-* | crisv32-*-*) ;;
  d10v*) ;;
  h8300*-*-*) ;;
  h8500-*-*) ;;
  i[3456]86-*-sco*) ;;
  lm32-*-*) ;;
  m32r-*-*) ;;
  mn10?00-*-*) ;;
  riscv*-*-*) ;;
  powerpcle-*-pe) ;;
  sh*-*-*) ;;
  sparc-sun-sunos*) ;;
  sparc64-*-*) ;;
  v850*-*-*) ;;
  w65-*-*) ;;
  xstormy16-*-*) ;;
  z8k-*-*) ;;
  *) AC_DEFINE(MISSING_SYSCALL_NAMES, 1, [Missing syscall names]) ;;
esac
