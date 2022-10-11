/* AR2F88.C  2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2f88.h";

void isqufft88 (ireg *a)
  /*
     a = a^2  (FFT mod F_8, a < (2^120)^128) = 2^15360)
  */
  {
  if (a->value == NULL) { iinvparm("isqufft88"); }
  if (a->digits < 1)    { iinvparm("isqufft88"); }
  
  utrim(a);
  if (a->digits <= 480)
    {
    if (res->value == NULL)   { ualloc(res, 2313, 1); }
    if (res->capacity < 2313) { uextend(res, 2313); }
    /* 2 digits breathing space required */
    if (a->capacity < 962) { uextend(a, 962); }
    xsqufft88(a->value, a->digits, res->value);
    a->digits <<= 1;
    a->flags = 0;
    }
  else
    { isqu(a); }
  }
  
