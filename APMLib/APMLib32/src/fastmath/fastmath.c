#include "fastmath.h"

unsigned long Zdx;		// Save EDX
unsigned long Zbx;		// Save EBX
unsigned long Zcx;		// Save ECX
unsigned long Zbp;		// Save EBP
unsigned long Zsi;		// Save ESI
unsigned long Zdi;		// Save EDI
unsigned long Zsg;		// Sign indicator
unsigned long Zmulda;
unsigned long Zmulb0;
unsigned long Zmuldb;
unsigned long Zdiva0;	// Save &a
unsigned long Zdivb0;	// Save &b
unsigned long Zdivda;	// Save d(a)
unsigned long Zdivdb;	// Save d(b)
unsigned long Zdivq0;	// Save &q
unsigned long Zdivbz;	// b_z = number of high order zero bits of b
unsigned long Zdivv1;	// v_1 = high order 32 bits of b
unsigned long Zdivv2;	// v_2 = next 32 bits of b
unsigned long Zdivii;	// i
unsigned long Zdivai;	// &a_i
unsigned long Zdivqi;	// q_i

extern double* FFTsincos = NULL;
extern double* FFTv1 = NULL;
extern double* FFTv2 = NULL;
extern long    FFTdim = 0;
extern long    FFTdimq1 = 0;
extern long    FFTdimq2 = 0;
extern long    FFTdimq3 = 0;
extern long    FFTdimv1 = 0;
extern long    FFTdimv2 = 0;
extern long    FFTsqu = 730;
extern long    FFTmul = 1200;
extern long    checkFFTerror = 1;
extern double  maxFFTerror = 0;
