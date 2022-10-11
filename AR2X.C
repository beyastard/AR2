/* AR2X.C  2.2.2E  TONY FORBES  October 1999 */

/* 
  FFT programs from R.E Crandall, Projects in Scientific Computation.
*/

#include <conio.h>;
#include <stdio.h>;
#include <stdlib.h>;
#include <math.h>;
#include "ar2x.h";

#define TWOPI (2 * 3.14159265358979323846)
#define SQRTHALF 0.70710678118654752440
#define TWO16 65536.0
#define TWOM16 0.0000152587890625

/* Increment bit-reversed index x */
#define inc_bit_rev(x) rb=half; while(x&rb) {x-=rb; rb>>=1;} x+=rb

extern double *FFTsincos = NULL;
extern double *FFTv1 = NULL;
extern double *FFTv2 = NULL;
extern long   FFTdim = 0;
extern long   FFTdimq1 = 0;
extern long   FFTdimq2 = 0;
extern long   FFTdimq3 = 0;
extern long   FFTdimv1 = 0;
extern long   FFTdimv2 = 0;
extern long   FFTsqu = 730;
extern long   FFTmul = 1200;
extern long   checkFFTerror = 1;
extern double maxFFTerror = 0;

void stop()
  /*
    Stop. Then exit if 'X' pressed
  */
  {
  if (getch() == 'X') exit(0);
  }

void give_up()
  /*
    Give up.
  */
  {
  printf ("Sorry, cannot continue\n"); 
  while (1) stop();
  }

void xfft_init(long n, long b)
  /*
    Allocate FFT vectors and initialise sin/cos table
    b = 1, 2, 3: allocate FFTv1, FFTv2, both
  */
  {
  static double *v;
  static long j;
  static double e;
  
  if (b & 1)
    {
    if (n > FFTdimv1)
      {
      v = (double *) realloc(FFTv1, sizeof(double) * n);
      if (v != NULL)
        { FFTv1 = v; }
      else
        {
        printf("Unable to allocate FFT %d-vector in xfft_init\n", n);
        give_up();
        }
      FFTdimv1 = n;
      }
    }
  
  if (b & 2)
    {
    if (n > FFTdimv2)
      {
      v = (double *) realloc(FFTv2, sizeof(double) * n);
      if (v != NULL)
        { FFTv2 = v; }
      else
        {
        printf("Unable to allocate FFT %d-vector in xfft_init\n", n);
        give_up();
        }
      FFTdimv2 = n;
      }
    }
    
  if (n > FFTdim)
    {
    e = TWOPI / n;
    j = 1 + (n >> 2);
    v = (double *) realloc(FFTsincos, sizeof(double) * j);
    if (v != NULL)
      {
      FFTsincos = v;
      }
    else
      {
      printf("Unable to allocate %d-vector for sin/cos table in xfft_init\n", j);
      give_up();
      }
    for (j = 0; j <= (n >> 2); j++)
      {
      FFTsincos[j] = sin(e * j);
      }
    FFTdim = n;
    FFTdimq2 = (FFTdim >> 1);
    FFTdimq1 = (FFTdimq2 >> 1);
    FFTdimq3 = FFTdimq1 + FFTdimq2;
    }
  }

void xfft_free()
  /*
    Free memory occupied by FFT vectors and sin/cos table
  */
  {
  if (FFTdimv1) { free(FFTv1); FFTdimv1 = 0; FFTv1 = NULL;}
  if (FFTdimv2) { free(FFTv2); FFTdimv2 = 0; FFTv1 = NULL;}
  if (FFTdim)   { free(FFTsincos); FFTdim = 0; FFTsincos = NULL;}
  }
  
double xsin(long n)
  {
  if (n < FFTdimq1) return(FFTsincos[n]);
  if (n < FFTdimq2) return(FFTsincos[FFTdimq2 - n]);
  if (n < FFTdimq3) return(-FFTsincos[n - FFTdimq2]);
                    return(-FFTsincos[FFTdim - n]);
  }

double xcos(long n)
  {
  if (n < FFTdimq1) return(FFTsincos[FFTdimq1 - n]);
  if (n < FFTdimq2) return(-FFTsincos[n - FFTdimq1]);
  if (n < FFTdimq3) return(-FFTsincos[FFTdimq3 - n]);
                    return(FFTsincos[n - FFTdimq3]);
  }

void xfft_pack(double *z, unsigned short *x, long n, long w)
  {
  register long j, r, rb, half = n >> 1;

  for (j = 0, r = 0; j < w; j++)
    {
    z[r] = x[j];
    inc_bit_rev(r);
    }
  while (r) 
    {
    z[r] = 0.0;
    inc_bit_rev(r);
    }
  }

void xfft_unpack(double *z, unsigned short *x, long n)
  {
  register long j, r, rb, half = n >> 1;
  register double d, e, f, g, h;

  maxFFTerror = 0;
  d = 1 / (double)n;
  g = 0.0;
  for (j = 0, r = 0; j < n; j++) 
    {
    h = z[r] * d;
    f = floor(h + 0.5);
    if (checkFFTerror) 
       {
       e = fabs(f - h);
       if (e > maxFFTerror)
         {
         maxFFTerror = e;
         }
       }
    f += g;   
    g = floor(f * TWOM16);
    f -= (g * TWO16);
    x[j] = f;
    inc_bit_rev(r);
    }
  if (g < TWO16) { x[n] = g; }
  else           { maxFFTerror += 100; }
  x[n+1] = 0;
  if (checkFFTerror)
    {
    if (maxFFTerror > 0.25)
      {
      if (maxFFTerror > 0.5)
        printf("FFT error: maxFFTerror=%g \n", maxFFTerror);
      else
        printf("FFT warning: maxFFTerror=%g \n", maxFFTerror);
      }
    }
  }

void xfft_real_to_hermitian(double *z, long n)
  /*
    Input  is {z[0], z[n/2], z[n/4], ..., z[n-1]} in bit-reversed order
    Output is {Re(z^[0]), ..., Re(z^[n/2]), Im(z^[n/2-1]), ..., Im(z^[1]).
    This is a decimation-in-time, split-radix algorithm. 
  */
  {
  register double cc1, ss1, cc3, ss3, e;
  register long is, id, i0, i1, i2, i3, i4, i5, i6, i7, a, a3, scmask = FFTdim - 1, dil;
  long nn = n >> 1;
  double t0, t1, t2, t3, t4, t5;
  register long n2, n4, n8, i, j;

  is = 1;
  id = 4;
  do 
    {
    for (i1 = is; i1 <= n; i1 += id) 
      {
      i0 = i1 - 1;
      e = z[i0];
      z[i0] = e + z[i1];
      z[i1] = e - z[i1];
      }
    is = (id << 1) - 1;
    id <<= 2;
    } while (is < n);
  n2 = 2;
  while (nn >>= 1)
    {
    n2 <<= 1;
    n4 = n2 >> 2;
    n8 = n2 >> 3;
    is = 0;
    id = n2 << 1;
    do 
      {
      for (i = is; i < n; i += id) 
        {
        i0 = i;
        i1 = i0 + n4;
        i2 = i1 + n4;
        i3 = i2 + n4;
        t0 = z[i3] + z[i2];
        z[i3] -= z[i2];
        z[i2] = z[i0] - t0;
        z[i0] += t0;
        if (n4 == 1) continue;
        i0 += n8;
        i1 += n8;
        i2 += n8;
        i3 += n8;
        t0 = (z[i2] + z[i3]) * SQRTHALF;
        t1 = (z[i2] - z[i3]) * SQRTHALF;
        z[i3] = z[i1] - t0;
        z[i2] = -z[i1] - t0;
        z[i1] = z[i0] - t1;
        z[i0] += t1;
        }
      is = (id << 1) - n2;
      id <<= 2;
      } while (is < n);
    dil = FFTdim / n2;
    a = dil;
    for (j = 1; j < n8; j++)
      {
      a3 = (a + (a << 1)) & scmask;
      /* cc1 = xcos(a) */
      if (a < FFTdimq1)      cc1 = FFTsincos[FFTdimq1 - a];
      else if (a < FFTdimq2) cc1 = -FFTsincos[a - FFTdimq1];
      else if (a < FFTdimq3) cc1 = -FFTsincos[FFTdimq3 - a];
      else                   cc1 = FFTsincos[a - FFTdimq3];
      /* cc3 = xcos(a3) */
      if (a3 < FFTdimq1)      cc3 = FFTsincos[FFTdimq1 - a3];
      else if (a3 < FFTdimq2) cc3 = -FFTsincos[a3 - FFTdimq1];
      else if (a3 < FFTdimq3) cc3 = -FFTsincos[FFTdimq3 - a3];
      else                    cc3 = FFTsincos[a3 - FFTdimq3];
      /* ss1 = xsin(a); */
      if (a < FFTdimq1)      ss1 = FFTsincos[a];
      else if (a < FFTdimq2) ss1 = FFTsincos[FFTdimq2 - a];
      else if (a < FFTdimq3) ss1 = -FFTsincos[a - FFTdimq2];
      else                   ss1 = -FFTsincos[FFTdim - a];
      /* ss3 = xsin(a3); */
      if (a3 < FFTdimq1)      ss3 = FFTsincos[a3];
      else if (a3 < FFTdimq2) ss3 = FFTsincos[FFTdimq2 - a3];
      else if (a3 < FFTdimq3) ss3 = -FFTsincos[a3 - FFTdimq2];
      else                   ss3 = -FFTsincos[FFTdim - a3];
      a = (a + dil) & scmask;
      is = 0;
      id = n2 << 1;
      do 
        {
        for (i = is; i < n; i += id) 
          {
          i0 = i + j;
          i1 = i0 + n4;
          i2 = i1 + n4;
          i3 = i2 + n4;
          i4 = i + n4 - j;
          i5 = i4 + n4;
          i6 = i5 + n4;
          i7 = i6 + n4;
          t0 = z[i2] * cc1 + z[i6] * ss1;
          t1 = z[i6] * cc1 - z[i2] * ss1;
          t2 = z[i3] * cc3 + z[i7] * ss3;
          t3 = z[i7] * cc3 - z[i3] * ss3;
          t4 = t0 + t2;
          t5 = t1 + t3;
          t2 = t0 - t2;
          t3 = t1 - t3;
          t1 = z[i5] + t5;
          z[i2] = t5 - z[i5];
          z[i7] = t1;
          t1 = z[i1] - t2;
          z[i6] = -z[i1] - t2;
          z[i3] = t1;
          t0 = z[i0] + t4;
          z[i5] = z[i0] - t4;
          z[i0] = t0;
          t0 = z[i4] + t3;
          z[i4] -= t3;
          z[i1] = t0;
          }
        is = (id << 1) - n2;
        id <<= 2;
        } while (is < n);
      }
    }
  }

void xfft_inv_hermitian_to_real(double *z, long n)
  /*
    Input  is   {Re(z^[0]), ..., Re(z^[n/2]), Im(z^[n/2-1]), ..., Im(z^[1]).
    Output is n*{z[0], z[n/2], z[n/4], ..., z[n-1]} in bit-reversed order
    This is a decimation-in-frequency, split-radix algorithm. 
  */
  {
  register double cc1, ss1, cc3, ss3, e;
  register long is, id, i0, i1, i2, i3, i4, i5, i6, i7, a, a3, scmask = FFTdim - 1, dil;
  long nn = n >> 1;
  double t0, t1, t2, t3, t4;
  long n2, n4, n8, i, j;
  
  n2 = n << 1;
  while (nn >>= 1)
    {
    is = 0;
    id = n2;
    n2 >>= 1;
    n4 = n2 >> 2;
    n8 = n4 >> 1;
    do 
      {
      for (i = is; i < n; i += id) 
        {
        i0 = i;
        i1 = i0 + n4;
        i2 = i1 + n4;
        i3 = i2 + n4;
        t0 = z[i0] - z[i2];
        z[i0] += z[i2];
        z[i1] += z[i1];
        z[i2] = t0 - 2.0 * z[i3];
        z[i3] = t0 + 2.0 * z[i3];
        if (n4 == 1) continue;
        i0 += n8;
        i1 += n8;
        i2 += n8;
        i3 += n8;
        t0 = (z[i1] - z[i0]) * SQRTHALF;
        t1 = (z[i3] + z[i2]) * SQRTHALF;
        z[i0] += z[i1];
        z[i1] = z[i3] - z[i2];
        z[i2] = -2.0 * (t1 + t0);
        z[i3] = 2.0 * (t0 - t1);
        }
      is = (id << 1) - n2;
      id <<= 2;
      } while (is < n - 1);
    dil = FFTdim / n2;
    a = dil;
    for (j = 1; j < n8; j++)
      {
      a3 = (a + (a << 1)) & scmask;
      /* cc1 = xcos(a) */
      if (a < FFTdimq1)      cc1 = FFTsincos[FFTdimq1 - a];
      else if (a < FFTdimq2) cc1 = -FFTsincos[a - FFTdimq1];
      else if (a < FFTdimq3) cc1 = -FFTsincos[FFTdimq3 - a];
      else                   cc1 = FFTsincos[a - FFTdimq3];
      /* cc3 = xcos(a3) */
      if (a3 < FFTdimq1)      cc3 = FFTsincos[FFTdimq1 - a3];
      else if (a3 < FFTdimq2) cc3 = -FFTsincos[a3 - FFTdimq1];
      else if (a3 < FFTdimq3) cc3 = -FFTsincos[FFTdimq3 - a3];
      else                    cc3 = FFTsincos[a3 - FFTdimq3];
      /* ss1 = xsin(a); */
      if (a < FFTdimq1)      ss1 = FFTsincos[a];
      else if (a < FFTdimq2) ss1 = FFTsincos[FFTdimq2 - a];
      else if (a < FFTdimq3) ss1 = -FFTsincos[a - FFTdimq2];
      else                   ss1 = -FFTsincos[FFTdim - a];
      /* ss3 = xsin(a3); */
      if (a3 < FFTdimq1)      ss3 = FFTsincos[a3];
      else if (a3 < FFTdimq2) ss3 = FFTsincos[FFTdimq2 - a3];
      else if (a3 < FFTdimq3) ss3 = -FFTsincos[a3 - FFTdimq2];
      else                   ss3 = -FFTsincos[FFTdim - a3];
      a = (a + dil) & scmask;
      is = 0;
      id = n2 << 1;
      do
        {
        for (i = is; i < n; i += id)
          {
          i0 = i + j;
          i1 = i0 + n4;
          i2 = i1 + n4;
          i3 = i2 + n4;
          i4 = i + n4 - j;
          i5 = i4 + n4;
          i6 = i5 + n4;
          i7 = i6 + n4;
          t0 = z[i0] - z[i5];
          z[i0] += z[i5];
          t1 = z[i4] - z[i1];
          z[i4] += z[i1];
          t2 = z[i7] + z[i2];
          z[i5] = z[i7] - z[i2];
          t3 = z[i3] + z[i6];
          z[i1] = z[i3] - z[i6];
          t4 = t0 - t3;
          t0 += t3;
          t3 = t1 - t2;
          t1 += t2;
          z[i2] = t4 * cc1 + t3 * ss1;
          z[i6] = -t3 * cc1 + t4 * ss1;
          z[i3] = t0 * cc3 - t1 * ss3;
          z[i7] = t1 * cc3 + t0 * ss3;
          }
        is = (id << 1) - n2;
        id <<= 2;
        } while (is < n - 1);
      }
    }
  is = 1;
  id = 4;
  do
    {
    for (i1 = is; i1 <= n; i1 += id)
      {
      i0 = i1 - 1;
      e = z[i0];
      z[i0] = e + z[i1];
      z[i1] = e - z[i1];
      }
    is = (id << 1) - 1;
    id <<= 2;
    } while (is < n);
  }

void xfft_square_hermitian(double *b, long n)
  {
  register long k, half = n >> 1;
  register double c, d;
 
  b[0] *= b[0];
  b[half] *= b[half];
  for (k = 1; k < half; k++)
    {
    c = b[k];
    d = b[n - k];
    b[n - k] = 2.0 * c * d;
    b[k] = (c + d) * (c - d);
    }
  }

void xfft_mul_hermitian(double *a, double *b, int n)
  {
  register int k, half = n >> 1;
  register double aa, bb, am, bm;

  a[0] *= b[0];
  a[half] *= b[half];
  for (k = 1; k < half; k++)
    {
    aa = a[k];
    bb = b[k];
    am = a[n - k];
    bm = b[n - k];
    a[k] = aa * bb - am * bm;
    a[n - k] = aa * bm + am * bb;
    }
  }

void xdump (long *a, long d)
  /* Unformatted print of an array */
  {
  static long i;
  printf("%d ", d);
  for (i = 0; i < d; i++) { printf("%d ", (unsigned long)a[i]); }
  printf("\n");
  }

void xdumps (short *a, long d)
  /* Unformatted print of an array */
  {
  static long i;
  printf("%d ", d);
  for (i = 0; i < d; i++) { printf("%d ", (unsigned short)a[i]); }
  printf("\n");
  }

void xdumpd (double *a, long d)
  /* Unformatted print of an array */
  {
  static long i;
  printf("%d ", d);
  for (i = 0; i < d; i++) { printf("%e ", a[i]); }
  printf("\n");
  }
