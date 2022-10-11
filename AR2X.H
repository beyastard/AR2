/* AR2X.H  2.2.0F  TONY FORBES  October 1999 */

double xsin(long n);
double xcos(long n);

void stop();
void give_up();

void xfft_init(long n, long b);
void xfft_free();
void xfft_pack(double *z, unsigned short *x, long n, long w);
void xfft_unpack(double *z, unsigned short *x, long n);
void xfft_real_to_hermitian(double *z, long n);
void xfft_inv_hermitian_to_real(double *z, long n);
void xfft_square_hermitian(double *b, long n);
void xfft_mul_hermitian(double *a, double *b, int n);

void xmovf  (long *a, long *b, long da);
void xmov   (long *a, long *b, long da, long db);
void xmovs  (long *a, long *b, long da, long db);
void xmovk  (long *a, long k, long da);
void xmovks (long *a, long k, long da);
void xmovz  (long *a, long da);

void xaddf  (long *a, long *b, long da);
long xadd   (long *a, long *b, long da, long db);
long xadds  (long *a, long *b, long da, long db);
long xaddk  (long *a, long k, long da);

void xsubf  (long *a, long *b, long da);
long xsub   (long *a, long *b, long da, long db);
long xsubs  (long *a, long *b, long da, long db);
long xsubk  (long *a, long k, long da);

void xneg   (long *a, long da);
long xcmp   (long *a, long *b, long da, long db);
long xcmps  (long *a, long *b, long da, long db);
long xsig   (long *a, long da);
long xdig   (long da, long *a);
void xcar   (long *a);
void xbor   (long *a);

void xsqu      (long *a, long da);
void xmul      (long *a, long *b, long da, long db);
void xmulk     (long *a, long k, long da);
void xmul2k    (long *a, long k, long da);
void xmul2d    (long *a, long d, long da);

void xdiv      (long *a, long *b, long da, long db, long *q);
long xdivk     (long *a, long k, long da);
void xdiv2k    (long *a, long k, long da);
void xdiv2d    (long *a, long d, long da);
void xmod      (long *a, long *b, long da, long db);
void xmodk     (long *a, long k, long da);
void xsep2k    (long *a, long *q, long k, long d);
long xnxtprmk  (long k);

void xdump     (long *a, long d);
void xdumps    (short *a, long d);
void xdumpd    (double *a, long d);

/* FFT stuff */
extern double *FFTsincos;
extern double *FFTv1;
extern double *FFTv2;
extern long   FFTdim;
extern long   FFTdimq1;
extern long   FFTdimq2;
extern long   FFTdimq3;
extern long   FFTdimv1;
extern long   FFTdimv2;
extern long   FFTsqu;
extern long   FFTmul;
extern long   checkFFTerror;
extern double maxFFTerror;

