/* AR2F88.H  2.2.0F  TONY FORBES  October 1999 */
void isqufft88 (ireg *a);
 
/* a = a^2  [0 < d(a) <= 480, cap(a) >= 962, cap(f) >= 2313] */
#define usqufft88(areg,freg)  xsqufft88(areg->value, areg->digits, freg->value); areg->digits <<= 1

void xsqufft88 (long *a, long da, long *f);
