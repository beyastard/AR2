/* AR2F.H  2.2.0F  TONY FORBES  October 1999 */

void fexpmod(ireg *a, ireg *b, ireg *c, ireg *q, void (fsqu)(ireg *a));
void fexpmodm2ke(ireg *a, ireg *b, ireg *c, ireg *q, long m, long k, ireg *e, void (fsqu)(ireg *a));
long ffermatm2ke(long m, long k, ireg *e, long g, void (fsqu)(ireg *a));

