/* AR2F80.C   2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2f80.h";

void isqufft80 (ireg *a)
  /*
     a = a^2  (FFT mod F_8)
  */
  {
  static long ca;
  if (a->value == NULL) { iinvparm("isqufft80"); }
  if (a->digits < 1)    { iinvparm("isqufft80"); }
  
  utrim(a);
  if (a->digits <= 1920)
    {
    ca = a->digits << 2;
    if (ca > 3842) { ca = 3842; };
    if (res->value == NULL)   { ualloc(res, 9225, 1); }
    if (res->capacity < 9225) { uextend(res, 9225); }
    }
  else
    {
    ca = a->digits << 1;
    }
  if (a->capacity < ca) { uextend(a, ca); }
  xsqufft80(a->value, a->digits, res->value);
  a->digits <<= 1;
  a->flags = 0;
  }
