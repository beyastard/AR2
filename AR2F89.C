/* AR2F89.C  2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2f89.h";

void isqufft89 (ireg *a)
  /*
     a = a^2  (FFT mod F_8, a < (2^120)^256) = 2^30720)
  */
  {
  if (a->value == NULL) { iinvparm("isqufft89"); }
  if (a->digits < 1)    { iinvparm("isqufft89"); }
  
  utrim(a);
  if (a->digits <= 960)
    {
    if (res->value == NULL)   { ualloc(res, 4617, 1); }
    if (res->capacity < 4617) { uextend(res, 4617); }
    /* 2 digits breathing space required */
    if (a->capacity < 1922) { uextend(a, 1922); }
    xsqufft89(a->value, a->digits, res->value);
    a->digits <<= 1;
    a->flags = 0;
    }
  else
    { isqu(a); }
  }
  
