#include <ieeefp.h>
#include <machine/registers.h>


fp_except fpsetmask (fp_except newmask)

{
  fp_except oldmask;
  v60_tkcw_type tkcw;
  
  sysv60(0, 8, &tkcw);
  oldmask = tkcw.fp_trap;
  tkcw.fp_trap = newmask;
  sysv60(0, 8, &tkcw);
  return oldmask;

}

fp_except fpgetmask (void)
{
  v60_tkcw_type tkcw;
  sysv60(0, 8, &tkcw);
  return tkcw.fp_trap;
}


fp_rnd fpgetround (void)
{
  v60_tkcw_type tkcw;
  sysv60(0, 8, &tkcw);
  return tkcw.fp_rounding;
}

fp_rnd fpsetround (fp_rnd rnd)
{
  fp_rnd oldrnd;
  v60_tkcw_type tkcw;
  
  sysv60(0, 8, &tkcw);
  oldrnd = tkcw.fp_rounding;
  tkcw.fp_rounding = rnd;
  sysv60(0, 8, &tkcw);
  return oldrnd;
}





fp_rdi fpgetroundtoi (void)
{
  v60_tkcw_type tkcw;
  sysv60(0, 8, &tkcw);
  return tkcw.integer_rounding;
}

fp_rdi fpsetroundtoi (fp_rdi rnd)
{
  fp_rdi oldrnd;
  v60_tkcw_type tkcw;
  
  sysv60(0, 8, &tkcw);
  oldrnd = tkcw.integer_rounding;
  tkcw.integer_rounding = rnd;
  sysv60(0, 8, &tkcw);
  return oldrnd;
}



