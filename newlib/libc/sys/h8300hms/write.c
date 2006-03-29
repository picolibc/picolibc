

int _write(file, ptr, len)
     int file;
     char *ptr;
     int len;
{
  int todo;
  
  for (todo = 0; todo < len; todo++) 
    {
      asm("mov.b #0,r1l\n mov.b %0l,r2l\njsr @@0xc4"   :  : "r" (*ptr++)  : "r1", "r2");
    }
  return len;
}

