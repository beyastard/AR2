/* AR2F80.H   2.2.0F  TONY FORBES  October 1999 */
void isqufft80 (ireg *a);
 
/* a = a^2  [d(a) > 0, cap(a) = max(2*d(a), min(4*d(a), 3842)) is sufficient, cap(f) >= 9225] */

#define usqufft80(areg,freg)  xsqufft80(areg->value, areg->digits, freg->value); areg->digits <<= 1

void xsqufft80 (long *a, long da, long *f);
