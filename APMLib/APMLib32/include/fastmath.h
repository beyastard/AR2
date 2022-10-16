#ifndef __FAST_MATH_H__
#define __FAST_MATH_H__

// AR2X.H  2.2.0F  TONY FORBES  October 1999

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TWOPI    (2 * 3.14159265358979323846)
#define SQRTHALF  0.70710678118654752440
#define TWO16     65536.0
#define TWOM16    0.0000152587890625

// Increment bit-reversed index x
#define inc_bit_rev(x)  rb=half; while((x)&rb) {(x)-=rb; rb>>=1;} (x)+=rb

extern unsigned long Zdx;		// Save EDX
extern unsigned long Zbx;		// Save EBX
extern unsigned long Zcx;		// Save ECX
extern unsigned long Zbp;		// Save EBP
extern unsigned long Zsi;		// Save ESI
extern unsigned long Zdi;		// Save EDI
extern unsigned long Zsg;		// Sign indicator
extern unsigned long Zmulda;
extern unsigned long Zmulb0;
extern unsigned long Zmuldb;
extern unsigned long Zdiva0;	// Save &a
extern unsigned long Zdivb0;	// Save &b
extern unsigned long Zdivda;	// Save d(a)
extern unsigned long Zdivdb;	// Save d(b)
extern unsigned long Zdivq0;	// Save &q
extern unsigned long Zdivbz;	// b_z = number of high order zero bits of b
extern unsigned long Zdivv1;	// v_1 = high order 32 bits of b
extern unsigned long Zdivv2;	// v_2 = next 32 bits of b
extern unsigned long Zdivii;	// i
extern unsigned long Zdivai;	// &a_i
extern unsigned long Zdivqi;	// q_i

double xsin(long n);
double xcos(long n);

void stop(void);
void give_up(void);

void xfft_init(long n, long b);
void xfft_free(void);
void xfft_pack(double* z, unsigned short* x, long n, long w);
void xfft_unpack(double* z, unsigned short* x, long n);
void xfft_real_to_hermitian(double* z, long n);
void xfft_inv_hermitian_to_real(double* z, long n);
void xfft_square_hermitian(double* b, long n);
void xfft_mul_hermitian(double* a, double* b, int n);

// AR2XA.ASM  2.2.0F  TONY FORBES  October 1999
//
// A collection of high - speed routines for doing arithmetic on integer
// arrays.
//
// There is no array bound checking of any kind.We always assume that the
// integer arrays are set up properly and that the size parameters have
// something sensible in them.We also assume that the result integer
// array has sufficient space to allow the operation to complete
// successfully.Multiplication requires high order space in the result
// that is equal in size to the multiplier operand.Division requires
// one extra high order digit.
//
// Except where we clearly state otherwise, we are unconcerned with signs.
// It is up to the caller to decide how to interpret an integer which has
// the high order bit set.Also we are not concerned with overflow;
// so add and subtract operations are treated modulo 2 ^ b for some
// suitable b.
//
// The basic operations, mov, add and sub come in different flavours,
// depending on the type of second operand, the relative operand sizes,
// and how they treat the sign bit.In the following scheme it may be
// helpful to remember that "f" stands for "fixed length", "s" for
// "signed" and "k" is the initial letter of a word that sounds like
// "constant".
//
//     1st       2nd      same |
//   operand   operand    size   signed | mov     add     sub
//   ---------------------------------- - +------------------------
//    array     array      yes     no | xmovf   xaddf   xsubf
//    array     array      no      no | xmov    xadd    xsub
//    array     array      no      yes | xmovs   xadds   xsubs
//    array    constant    no      no | xmovk   xaddk   xsubk
//    array    constant    no      yes | xmovks - -
//
// In the routines that recognise signed integers the second operand,
// if shorter than the first, is extended with sign bits.
//
// Routines xadd, xadds xaddk, xsub, xsubs and xsubk return the sign bit
// of the result in register EAX, bit 31, and the carry flag in EAX, bit 8.
// Routines addf and subf do not.Observe that routines xaddks and xsubks
// don't exist; there seems to be no need for them. 
//
// The C++ compiler passes words in the order EAX, EDX, EBX, ECX; then
// any further parameters are placed on the stack.
//
// The compiler insists that we do not alter any register that is not
// used for passing parameters.PUSH and POP are expensive in clock ticks
// (and produce catastrophic results when they get out of step), so we often
// use MOV and designated words to save and restore the contents of registers.
// Conversly, we think we can do what we like with registers that *are*
// used for passing parameters.
//
// Let a be an integer array.We denote the capacity of a in digits by
// c(a) and the number of "active" digits by d(a).

void xmovf(long* a, long* b, long da);
void xmov(long* a, long* b, long da, long db);
void xmovs(long* a, long* b, long da, long db);
void xmovk(long* a, long k, long da);
void xmovks(long* a, long k, long da);
void xmovz(long* a, long da);

void xaddf(long* a, long* b, long da);
long xadd(long* a, long* b, long da, long db);
long xadds(long* a, long* b, long da, long db);
long xaddk(long* a, long k, long da);

void xsubf(long* a, long* b, long da);
long xsub(long* a, long* b, long da, long db);
long xsubs(long* a, long* b, long da, long db);
long xsubk(long* a, long k, long da);

void xneg(long* a, long da);
long xcmp(long* a, long* b, long da, long db);
long xcmps(long* a, long* b, long da, long db);
long xsig(long* a, long da);
long xdig(long da, long* a);
void xcar(long* a);
void xbor(long* a);

void xsqu(long* a, long da);
void xmul(long* a, long* b, long da, long db);
void xmulk(long* a, long k, long da);
void xmul2k(long* a, long k, long da);
void xmul2d(long* a, long d, long da);

void xdiv(long* a, long* b, long da, long db, long* q);
long xdivk(long* a, long k, long da);
void xdiv2k(long* a, long k, long da);
void xdiv2d(long* a, long d, long da);
void xmod(long* a, long* b, long da, long db);
void xmodk(long* a, long k, long da);
void xsep2k(long* a, long* q, long k, long d);
long xnxtprmk(long k);

void xdump(long* a, long d);
void xdumps(short* a, long d);
void xdumpd(double* a, long d);

// FFT stuff
extern double* FFTsincos;
extern double* FFTv1;
extern double* FFTv2;
extern long    FFTdim;
extern long    FFTdimq1;
extern long    FFTdimq2;
extern long    FFTdimq3;
extern long    FFTdimv1;
extern long    FFTdimv2;
extern long    FFTsqu;
extern long    FFTmul;
extern long    checkFFTerror;
extern double  maxFFTerror;

#endif
