extern int outbyte(unsigned char byte);
extern void print (const char *);

int main()
{
  outbyte ('&');
  outbyte ('@');
  outbyte ('$');
  outbyte ('%');

  /* whew, we made it */
  
  print ("\r\nDone...");

  return 0;
}
