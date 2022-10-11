/* AR2F8A.C  2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2f8A.h";

void isqufft8A (ireg *a)
  /*
     a = a^2  (FFT mod F_8, a < (2^120)^512) = 2^61440)
  */
  {
  if (a->value == NULL) { iinvparm("isqufft8A"); }
  if (a->digits < 1)    { iinvparm("isqufft8A"); }
  
  utrim(a);
  if (a->digits <= 1920)
    {
    if (res->value == NULL)   { ualloc(res, 9225, 1); }
    if (res->capacity < 9225) { uextend(res, 9225); }
    /* 2 digits breathing space required */
    if (a->capacity < 3842) { uextend(a, 3842); }
    xsqufft8A(a->value, a->digits, res->value);
    a->digits <<= 1;
    a->flags = 0;
    }
  else
    { isqu(a); }
  }
  
