/* AR2F8A.H  2.2.0F  TONY FORBES  October 1999 */
void isqufft8A (ireg *a);
 
/* a = a^2  [0 < d(a) <= 1920, cap(a) >= 3842, cap(f) >= 9225] */
#define usqufft8A(areg,freg)  xsqufft8A(areg->value, areg->digits, freg->value); areg->digits <<= 1

void xsqufft8A (long *a, long da, long *f);
