int 
__fp_unordered_compare (double x,  double y){
  unsigned short retval;
  __asm__ (
	"fucom %%st(1);"
	"fnstsw;"
	: "=a" (retval)
	: "t" (x), "u" (y)
	);
  return retval;
}
