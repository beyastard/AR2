/* AR2FC0.H  2.2.0F  TONY FORBES  October 1999 */
void isqufftC0 (ireg *a);
 
/* a = a^2  [d(a) > 0, cap(a) = max(2*d(a), min(4*d(a), 258048)) is sufficient, cap(f) >= 528513] */

#define usqufftC0(areg,freg)  xsqufftC0(areg->value, areg->digits, freg->value); areg->digits <<= 1

void xsqufftC0 (long *a, long da, long *f);
