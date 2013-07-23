int inbyte(void)
{
	char ch = 0;
	read(0, &ch, 1);
	return ch;
}
