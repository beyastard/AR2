/* AR2FC0.C  2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "AR2FC0.h";

void isqufftC0 (ireg *a)
  /*
     a = a^2  (FFT mod F_12)
  */
  {
  static long ca;
  if (a->value == NULL) { iinvparm("isqufftC0"); }
  if (a->digits < 1)    { iinvparm("isqufftC0"); }
  
  utrim(a);
  if (a->digits <= 129024)
    {
    ca = a->digits << 2;
    if (a->capacity < ca) uextend(a, ca); 
    if (ca > 258048) ca = 258048;
    if (res->value == NULL)     ualloc(res, 528513, 1);
    if (res->capacity < 528513) uextend(res, 528513);
    xsqufftC0(a->value, a->digits, res->value);
    a->digits <<= 1;
    a->flags = 0;
    }
  else
    isqu(a);
  }
