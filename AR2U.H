/* AR2U.H  2.2.0F  TONY FORBES  October 1999 */

struct IntegerRegister
       {
       long *value;     /* Pointer to units digit of integer array         */
       long capacity;   /* Maximum number of digits integer array can hold */
       long digits;     /* Number of "active" digits in integer array      */
       long flags;      /* Status flags */
       } ;
typedef struct IntegerRegister ireg ;

/* -> flags */
#define NegativeReg 0x80000000 

/* Hardware flags as stored by lahf */
#define SignFlag    0x00008000
#define ZeroFlag    0x00004000
#define AuxFlag     0x00001000
#define ParityFlag  0x00000200
#define CarryFlag   0x00000100

/* Hardware register sign bit */
#define SignBit     0x80000000

void ualloc    (ireg *a, long c, long d);
void ufree     (ireg *a);
void uextend   (ireg *a, long d);
void usep2k    (ireg *a, ireg *b, long k);
void udump     (ireg *a);

/* a = b  [d(a) = d(b) > 0] */
#define umovf(areg,breg)   xmovf(areg->value, breg->value, breg->digits); areg->digits = breg->digits

/* a = b  [d(a) >= d(b) > 0) */
#define umov(areg,breg)    xmov(areg->value, breg->value, areg->digits, breg->digits)
  
/* a = b  [b sign extended, d(a) >= d(b) > 0] */
#define umovs(areg,breg)   xmovs(areg->value, breg->value, areg->digits, breg->digits)
  
/* a = k  [d(a) > 0] */
#define umovk(areg,knum)   xmovk(areg->value, knum, areg->digits)

/* a = k  [k sign extended, d(a) > 0] */
#define umovks(areg,knum)  xmovks(areg->value, knum, areg->digits)
  
/* a = 0  [d(a) > 0] */
#define umovz(areg)   xmovz(areg->value, areg->digits)
  
/* a = a + b (mod 2^(32*d(a))) [d(a) = d(b) > 0] */
#define uaddf(areg,breg)  xaddf(areg->value, breg->value, breg->digits)
  
/* a = a + b (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [d(a) >= d(b) > 0] */
#define uadd(areg,breg)   xadd(areg->value, breg->value, areg->digits, breg->digits)
  
/* a = a + b (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [sign extended, d(a) >= d(b) > 0] */
#define uadds(areg,breg)  xadds(areg->value, breg->value, areg->digits, breg->digits)
  
/* a = a + k (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [k >= 0, d(a) >= d(b) > 0] */
#define uaddk(areg,knum)  xaddk(areg->value, knum, areg->digits)
  
/* a = a - b (mod 2^(32*d(a)))  [d(a) = d(b) > 0] */
#define usubf(areg,breg)  xsubf(areg->value, breg->value, breg->digits)
  
/* a = a - b (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [d(a) >= d(b) > 0] */
#define usub(areg,breg)   xsub(areg->value, breg->value, areg->digits, breg->digits)
  
/* a = a - b (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [sign extended, d(a) >= d(b) > 0] */
#define usubs(areg,breg)  xsubs(areg->value, breg->value, areg->digits, breg->digits)
  
/* a = a - k (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [k >= 0, d(a) >= d(b) > 0] */
#define usubk(areg,knum)  xsubk(areg->value, knum, areg->digits)
  
/* a = 2^(32*d(a)) - a  [d(a) > 0] */
#define uneg(areg)   xneg(areg->value, areg->digits)
  
/* flags(a-b)  [d(a), d(b) > 0] */
#define ucmp(areg,breg)  xcmp(areg->value, breg->value, areg->digits, breg->digits)
  
/* flags(a-b)  [shorter of a and b sign extended, d(a), d(b) > 0] */
#define ucmps(areg,breg)  xcmps(areg->value, breg->value, areg->digits, breg->digits)
  
/* high order digit of a  [d(a) > 0] */
#define usig(areg)  xsig (areg->value, areg->digits)
  
/* a = a^2  [d(a) > 0, c(a) >= 2*d(a)] */
#define usqu(areg)  xsqu(areg->value, areg->digits); areg->digits = 2*areg->digits
  
/* a = a * b  [d(a), d(b) > 0, c(a) >= d(a) + d(b)] */
#define umul(areg,breg)  xmul(areg->value, breg->value, areg->digits, breg->digits); areg->digits = areg->digits + breg->digits
  
/* a = a * k  [d(a) > 0, c(a) > d(a)] */
#define umulk(areg,knum)  xmulk(areg->value, knum, areg->digits); areg->digits++
  
/* a = a * 2^k  [d(a) > 0, c(a) >= d(a) + [(k + 31)/32]] */
#define umul2k(areg,knum)  xmul2k(areg->value, knum, areg->digits); areg->digits += ((k + 31) >> 5)
  
/* a = a * 2^(32*d)  [d(a) > 0, c(a) >= d(a) + d] */
#define umul2d(areg,dnum)  xmul2d(areg->value, dnum, areg->digits); areg->digits += dnum
  
/* a = (a mod b), q = [a / b]  [c(a) > d(a) >= d(b) > 0, c(q) >= d(a) - d(b) + 1] */
#define udiv(areg,breg,qreg)  xdiv(areg->value, breg->value, areg->digits, breg->digits, qreg->value); qreg->digits = areg->digits - breg->digits + 1  

/* a = [a / k], returns (a mod k)  [k > 0, d(a) > 0] */
#define udivk(areg,knum)  xdivk(areg->value, knum, areg->digits)
  
/* a = [a / 2^k]  [0 <= k < 32*d(a), d(a) > 0] */
#define udiv2k(areg,knum)  xdiv2k(areg->value, knum, areg->digits); areg->digits -= (knum >> 5)
  
/* a = [a / 2^(32*d)]  [0 <= d < d(a), d(a) > 0] */
#define udiv2d(areg,dnum)  xdiv2d(areg->value, dnum, areg->digits); areg->digits -= dnum
  
/* a = (a mod b)  [b > 0, c(a) > d(a) >= d(b) > 0] */
#define umod(areg,breg,qreg)  xdiv(areg->value, breg->value, areg->digits, breg->digits)
  
/* a = (a mod k)  [k > 0, d(a) > 0] */
#define umodk(areg,knum)  xmodk(areg->value, knum, areg->digits); areg->digits = 1

/* Remove non-significant digits from an integer register */
#define utrim(areg)  areg->digits = xdig(areg->digits, areg->value)
