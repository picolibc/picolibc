int
isinf (double x)
{
  return __builtin_isinf_sign (x);
}

int
isinff (float x)
{
  return __builtin_isinf_sign (x);
}

int
isinfl (long double x)
{
  return __builtin_isinf_sign (x);
}

