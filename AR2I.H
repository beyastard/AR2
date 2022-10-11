/* AR2I.H  2.2.0F  TONY FORBES  October 1999 */

void imov      (ireg *a, ireg *b);
void imovk     (ireg *a, long k);
void imovd     (ireg *a, char *c);
void iadd      (ireg *a, ireg *b);
void iaddk     (ireg *a, long k);
void isub      (ireg *a, ireg *b);
long isgn      (ireg *a);
long icmp      (ireg *a, ireg *b);
long icmpk     (ireg *a, long k);
              
void isqu      (ireg *a);
void isqufft   (ireg *a);
void imul      (ireg *a, ireg *b);
void imulk     (ireg *a, long k);
void imul2k    (ireg *a, long k);
void idiv      (ireg *a, ireg *b);
void idivk     (ireg *a, long k);
void idiv2k    (ireg *a, long k);
void imod      (ireg *a, ireg *b);
void imodk     (ireg *a, long k);
void imod2k    (ireg *a, long k);

void iexp(ireg *a, ireg *b, ireg *c);
void iexpmod(ireg *a, ireg *b, ireg *c, ireg *q);
void iexpmodm2ke(ireg *a, ireg *b, ireg *c, ireg *q, long m, long k, ireg *e);
void igcd(ireg *a, ireg *b);
void igcdext(ireg *u, ireg *d, ireg *a, ireg *b);
double ilog    (ireg *a);
void isqrt     (ireg *a, ireg *b);

long ilen      (ireg *a);    
long ibit      (ireg *a, long k);    
void idisp     (ireg *a);
void idispf    (FILE *f, ireg *a);
void iinvparm  (char *c);

#define isubk(areg,klng) iaddk (areg, -(klng))
#define ineg(areg) (areg->flags ^= NegativeReg)
#define iabs(areg) (areg->flags &= (0xFFFFFFFF-NegativeReg))
#define iodd(areg) (areg->value[0] & 1)
#define ieven(areg) !(areg->value[0] & 1)
#define iwd0(areg) (areg->value[0])

/* Remainder register */
extern ireg *res;
extern long kres;

/* Temporary registers */
extern ireg *temp1;
extern ireg *temp2;
extern ireg *temp3;
extern ireg *temp4;
extern ireg *temp5;

/* Useful constants */
extern double ilog2;
extern double ilog10;
