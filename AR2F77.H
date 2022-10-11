/* AR2F77.H   2.2.0H  TONY FORBES  October 1999 */
void isqufft77 (ireg *a);
 
/* a = a^2  [0 < d(a) <= 120, cap(a) >= 242, cap(f) >= 645] */
#define usqufft77(areg,freg)  xsqufft77(areg->value, areg->digits, freg->value); areg->digits <<= 1

void xsqufft77 (long *a, long da, long *f);
