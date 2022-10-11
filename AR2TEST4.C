/* AR2TEST4.C  2.2.2E  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include <time.h>
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2t.h";
#include "ar2f.h";
#include "ar2f77.h";
#include "ar2f80.h";
#include "ar2f88.h";
#include "ar2f89.h";
#include "ar2f8A.h";
#include "ar2fA0.h";

void ttimefsqu(void (fsqu)(ireg *a), long k0, long n, char *id);
void ttimenxtprm(long p0);

static ireg ra;
static ireg *a = &ra;
static FILE *dat;

void main()
  {
  tprint("ARITH2 TEST 4  Version 2.2.1F\n");
  
  ttimefsqu(tsqusch,       3000,   4000, "tsqusch");
  ttimefsqu(tsqufft,       3000,    500, "tsqufft");
  ttimefsqu(tsqumulfft,    3000,    500, "tsqumulfft");
  ttimefsqu(isqufft77,     3000,   6000, "isqufft77");
  ttimefsqu(isqufft80,     3000,   6000, "isqufft80");
  ttimefsqu(isqufftA0,     3000,   6000, "isqufftA0");

  ttimefsqu(tsqusch,       4000,   2000, "tsqusch");
  ttimefsqu(tsqufft,       4000,    500, "tsqufft");
  ttimefsqu(tsqumulfft,    4000,    500, "tsqumulfft");

  ttimefsqu(tsqusch,       6000,   1500, "tsqusch");
  ttimefsqu(tsqufft,       6000,    300, "tsqufft");
  ttimefsqu(tsqumulfft,    6000,    250, "tsqumulfft");

  ttimefsqu(tsqusch,       8000,   1000, "tsqusch");
  ttimefsqu(tsqufft,       8000,    300, "tsqufft");
  ttimefsqu(tsqumulfft,    8000,    250, "tsqumulfft");

  ttimefsqu(tsqusch,      10000,    500, "tsqusch");
  ttimefsqu(tsqufft,      10000,    200, "tsqufft");
  ttimefsqu(tsqumulfft,   10000,    100, "tsqumulfft");
  
  ttimefsqu(tsqusch,      15000,    250, "tsqusch");
  ttimefsqu(tsqufft,      15000,    200, "tsqufft");
  ttimefsqu(tsqumulfft,   15000,    100, "tsqumulfft");
  ttimefsqu(isqufft88,    15000,   1000, "isqufft88");
  ttimefsqu(isqufft80,    15000,   1000, "isqufft80");
  ttimefsqu(isqufftA0,    15000,   1000, "isqufftA0");

  ttimefsqu(tsqusch,      20000,    200, "tsqusch");
  ttimefsqu(tsqufft,      20000,    100, "tsqufft");
  ttimefsqu(tsqumulfft,   20000,     50, "tsqumulfft");
  ttimefsqu(isqufft80,    20000,    500, "isqufft80");
  ttimefsqu(isqufftA0,    20000,    500, "isqufftA0");

  ttimefsqu(tsqusch,      25000,    100, "tsqusch");
  ttimefsqu(tsqufft,      25000,    100, "tsqufft");
  ttimefsqu(tsqumulfft,   25000,     50, "tsqumulfft");
  ttimefsqu(isqufft80,    25000,    500, "isqufft80");
  ttimefsqu(isqufftA0,    25000,    500, "isqufftA0");
  
  ttimefsqu(tsqusch,      30000,     50, "tsqusch");
  ttimefsqu(tsqufft,      30000,    100, "tsqufft");
  ttimefsqu(tsqumulfft,   30000,     50, "tsqumulfft");
  ttimefsqu(isqufft89,    30000,    500, "isqufft89");
  ttimefsqu(isqufft80,    30000,    500, "isqufft80");
  ttimefsqu(isqufftA0,    30000,    500, "isqufftA0");
  
  ttimefsqu(tsqusch,      35000,     30, "tsqusch");
  ttimefsqu(tsqufft,      35000,     50, "tsqufft");
  ttimefsqu(tsqumulfft,   35000,     25, "tsqumulfft");
  ttimefsqu(isqufft80,    35000,    250, "isqufft80");
  ttimefsqu(isqufftA0,    35000,    250, "isqufftA0");
  
  ttimefsqu(tsqusch,      40000,     20, "tsqusch");
  ttimefsqu(tsqufft,      40000,     50, "tsqufft");
  ttimefsqu(tsqumulfft,   40000,     25, "tsqumulfft");
  ttimefsqu(isqufft80,    40000,    250, "isqufft80");
  ttimefsqu(isqufftA0,    40000,    250, "isqufftA0");

  ttimefsqu(tsqusch,      50000,     20, "tsqusch");
  ttimefsqu(tsqufft,      50000,     50, "tsqufft");
  ttimefsqu(tsqumulfft,   50000,     25, "tsqumulfft");
  ttimefsqu(isqufft80,    50000,    250, "isqufft80");
  ttimefsqu(isqufftA0,    50000,    250, "isqufftA0");

  ttimefsqu(tsqusch,      55000,     15, "tsqusch");
  ttimefsqu(tsqufft,      55000,     50, "tsqufft");
  ttimefsqu(tsqumulfft,   55000,     25, "tsqumulfft");
  ttimefsqu(isqufft80,    55000,    250, "isqufft80");
  ttimefsqu(isqufftA0,    55000,    250, "isqufftA0");

  ttimefsqu(tsqusch,      60000,     10, "tsqusch");
  ttimefsqu(tsqufft,      60000,     50, "tsqufft");
  ttimefsqu(tsqumulfft,   60000,     25, "tsqumulfft");
  ttimefsqu(isqufft8A,    60000,    250, "isqufft8A");
  ttimefsqu(isqufft80,    60000,    250, "isqufft80");
  ttimefsqu(isqufftA0,    60000,    250, "isqufftA0");

  ttimefsqu(tsqusch,     125000,      3, "tsqusch");
  ttimefsqu(tsqufft,     125000,     20, "tsqufft");
  ttimefsqu(tsqumulfft,  125000,     10, "tsqumulfft");
  ttimefsqu(isqufftA0,   125000,    100, "isqufftA0");

  ttimefsqu(tsqusch,     250000,      1, "tsqusch");
  ttimefsqu(tsqufft,     250000,      5, "tsqufft");
  ttimefsqu(tsqumulfft,  250000,      4, "tsqumulfft");
  ttimefsqu(isqufftA0,   250000,     25, "isqufftA0");
  xfft_free();
  
  ttimefsqu(tsqufft,     500000,      2, "tsqufft");
  ttimefsqu(tsqumulfft,  500000,      2, "tsqumulfft");
  ttimefsqu(isqufftA0,   500000,     10, "isqufftA0");
  xfft_free();

  ttimefsqu(tsqufft,    1000000,      2, "tsqufft");
//ttimefsqu(tsqumulfft, 1000000,      1, "tsqumulfft");
  ttimefsqu(isqufftA0,  1000000,      5, "isqufftA0");

  xfft_free();
  ttimefsqu(tsqufft,    2000000,      1, "tsqufft");

  halt();

  ttimenxtprm(10000000);
  ttimenxtprm(20000000);
  ttimenxtprm(50000000);
  ttimenxtprm(100000000);
  ttimenxtprm(200000000);
  ttimenxtprm(500000000);
  ttimenxtprm(1000000000);
  ttimenxtprm(2000000000);

  tprint("All done\n"); getchar();
  }  

/* 
   The program crashes with a page fault at address (unrelocated) = 1:00014BA7
   in ttimefsqu after call ttimefsqu(tsqumulfft, 1000000, 1, "tsqumulfft");
   during the first invokation of fsqu. possibly because it is outside the design
   limits of the FFT multiplication that I am using. However, I am still mystified.
*/
void ttimefsqu(void (fsqu)(ireg *a), long k0, long n, char *id)
  {
  static long i, k, d;
  static double time, insttime;
  static clock_t sclock, tclock;
  if (test_quit()) halt();
  d = (k0 + 31) >> 5;
  k = d << 5;
  /* Allocate 2*d digits for a and fill with garbage */
  imovk(a, 999999999);
  for (i = 0; i < d+d; i++) { imul2k(a, 32); iaddk(a, 2111222333 - 9999*i); }
  /* First invokation gets res allocated */
  a->digits = d;
//printf("E k=%d d=%d n=%d id=%s a-digits=%d \n", k, d, n, id, a->digits);
  fsqu(a);
  tprint("c=%d d=%d \r", a->capacity, a->digits);
  /* Now do the timing */
  sclock = clock();
  for (i = 1; i <= n; i++)
    {
    a->digits = d;
    a->value[0] = -1;
    a->value[d-1] = 999999999;
    fsqu(a);
    }
  tclock = clock();
  tprint("c=%d d=%d \r", a->capacity, a->digits);
  time = (tclock - sclock); 
  time /= CLOCKS_PER_SEC; 
  insttime = time / n;
  tprint("%s: k=%d, d=%d %d, n=%d, Time=%9.7f, Total=%7.3f sec\n",
          id, k, d, a->digits, n, insttime, time);
  dat = fopen("AR2TEST4.DAT", "a");
  fprintf(dat, "%s: k=%d, d=%d %d, n=%d, Time=%9.7f, Total=%7.3f sec\n",
          id, k, d, a->digits, n, insttime, time);
  fclose(dat);
  }

void ttimenxtprm(long p0)
  {
  static long i, j, p;
  static double time, insttime;
  static clock_t sclock, tclock;
  if (test_quit()) halt();
  p = p0;
  for (i = 1; i <= 3; i++)
    {
    sclock = clock();
    for (j = 1; j <= 1000; j++)
      {
      p = xnxtprmk(p);
      }
    tclock = clock();
    time = (tclock - sclock); 
    time /= CLOCKS_PER_SEC; 
    insttime = time / 1000;
    tprint("xnxtprmk: p=%d, Time=%9.7f, Time1000=%7.3f sec\n",
          p, insttime, time);
    }
  }
  
