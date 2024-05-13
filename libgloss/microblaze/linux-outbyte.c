extern int _write (int, char *, int);

void outbyte (unsigned char c)
{
	_write(1, &c, 1);
}
