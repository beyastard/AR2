/* AR2F77.C   2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2f77.h";

void isqufft77 (ireg *a)
  /*
     a = a^2  (FFT mod F_7, a < (2^60)^64) = 2^3840)
  */
  {
  if (a->value == NULL) { iinvparm("isqufft77"); }
  if (a->digits < 1)    { iinvparm("isqufft77"); }
  
  utrim(a);
  if (a->digits <= 120)
    {
    if (res->value == NULL)  { ualloc(res, 645, 1); }
    if (res->capacity < 645) { uextend(res, 645); }
    /* 2 digits breathing space required */
    if (a->capacity < 242) { uextend(a, 242); }
    xsqufft77(a->value, a->digits, res->value);
    a->digits <<= 1;
    a->flags = 0;
    }
  else
    { isqu(a); }
  }
  
