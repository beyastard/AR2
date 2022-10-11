/* AR2F89.H  2.2.0F  TONY FORBES  October 1999 */
void isqufft89 (ireg *a);
 
/* a = a^2  [0 < d(a) <= 960, cap(a) >= 1922, cap(f) >= 4617] */
#define usqufft89(areg,freg)  xsqufft89(areg->value, areg->digits, freg->value); areg->digits <<= 1

void xsqufft89 (long *a, long da, long *f);
