/* AR2FA0.C  2.1.0m */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2fA0.h";

void isqufftA0 (ireg *a)
  /*
     a = a^2  (FFT mod F_10)
  */
  {
  static long ca;
  if (a->value == NULL) { iinvparm("isqufftA0"); }
  if (a->digits < 1)    { iinvparm("isqufftA0"); }
  
  utrim(a);
  if (a->digits <= 32256)
    {
    ca = a->digits << 2;
    if (ca > 64514) { ca = 64514; };
    if (res->value == NULL)   { ualloc(res, 135201, 1); }
    if (res->capacity < 135201) { uextend(res, 135201); }
    }
  else
    {
    ca = a->digits << 1;
    }
  if (a->capacity < ca) { uextend(a, ca); }
  xsqufftA0(a->value, a->digits, res->value);
  a->digits <<= 1;
  a->flags = 0;
  }
