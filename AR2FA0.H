/* AR2FA0.H  2.1.0m */
void isqufftA0 (ireg *a);
 
/* a = a^2  [d(a) > 0, cap(a) = max(2*d(a), min(4*d(a), 64514)) is sufficient, cap(f) >= 135201] */

#define usqufftA0(areg,freg)  xsqufftA0(areg->value, areg->digits, freg->value); areg->digits <<= 1

void xsqufftA0 (long *a, long da, long *f);
