#ifndef __VFYPR32_H__
#define __VFYPR32_H__

/* VFYPR.C  TONY FORBES  December 1999 */

#define version "1.15E"

/*
Function:

Primality proof by factorization of N^2 - 1 together with
the APRCL test.

Description of the primality proving algorithm

Preliminary tests

We assume N is not too small. We also assume that N is not
divisible by any of the relevant parameters of the algorithm.
This is achieved by trial-dividing N by all primes up to
a suitable limit and by anything else that could cause trouble.

Let N be given and suppose d is a prime divisor of N.

Pocklington's Theorem

Suppose N - 1 = F1*R1, 2|F1, gcd(F1, R1) = 1 and F1 is completely
factorized into verified primes. Suppose also that for each prime
factor f of F1, there exists a number a such that 1 < a < N - 1,
a^(N - 1) = 1 (mod N) and gcd(a^((N - 1)/f) - 1, N) = 1. Then,
by Pocklington's Theorem [BLS 1975, Theorem 4],

(1)  d == 1 (mod F1).

Morrison's Theorem

Suppose N + 1 = F2*R2, 2|F2, gcd(F2, R2) = 1 and F2 is completely
factorized into verified primes. Suppose also that there is an
integer D such that (D/N) = -1 and for each prime factor f of F,
there exists a Lucas sequence {U_k} with discriminant D = P^2 - 4*Q,
where gcd(N, Q) = 1, for which U_(N + 1) = 0 (mod N) and
gcd(U_((N + 1)/f), N) = 1. Then, by Morrison's Theorem
[BLS 1975, Theorem 16],

(2)  d == +/-1 (mod F2).

The APRCL method

This part follows closely Section 9.1 of Henri Cohen's book
[Cohen 1995].

Denote by v_p(x) the exponent of the largest power of p that
divides x and by zeta_n the n-th root of unity e^(2*pi*i/n).

Let chi be a character modulo q. The Gauss sum is defined by

tau(chi) = sum{x in (Z/qZ)*: chi(x)*(zeta_q)^x}.

Let chi and psi be characters modulo q. The Jacobi sum is
defined by

j(chi, psi) = sum{x in (Z/qZ)*: chi(x)*psi(1 - x)}.

Let T be an even integer. Write

e(T) =  2*product{q prime, q - 1|T: q^(v_q(T) + 1)}.

Let S be a factor of e(T) such that gcd(S, e(T)/S) = 1.
Observe that N^T == 1 (mod S).

For each pair of primes q|S and p|q - 1, let k = v_p(q - 1)
and let chi be the character modulo q of order p^k defined by
chi(x) = (zeta_p^k)^ind_g(x), where g is a primitive root
modulo q. Assume gcd(N, S*T) = 1. Assume that for each pair
(p, q) the character chi satisfies condition (*beta) for some
beta (depending on p and q) for which (zeta_p)^beta is not
equal to 1. Suppose also that for all primes p|T, condition
L_p is satisfied. Then

(3)    d == N^i (mod S) for some i, 1 <= i < T.

Condition (*beta)

Let q, p, k and chi be as above; q|S, p^k||q - 1, and chi is a
character modulo q of order p^k. Let sigma_x denote the
homomorphism of (Z/N*Z)[zeta_p^k] that sends zeta_p^k to
(zeta_p^k)^x. If h = A*sigma_a + B*sigma_b + ... is a linear
combination of such homomorphims and if f(zeta_p^k) is a
polynomial in zeta_p^k, we use the expression f(zeta_p^k)^h to
denote the product f((zeta_p^k)^a)^A * f((zeta_p^k)^b)^B * ....

The condition (*beta) is defined by

(*beta)  tau^(beta*(N - sigma_N)) == eta(chi)^(-beta*N) (mod N)

for some eta(chi) in <zeta_p^k>.

When applying the (*beta) test it is sufficient to show that
tau^(beta*(N - sigma_N)) is congruent to a p^k-th root of
unity modulo N. If also condition L_p is satisfied, then in
fact eta(chi) = chi(N) [Cohen 1995, Proposition 9.1.11].

For a practical primality-proving algorithm we replace (*beta)
by equivalent conditions involving Jacobi sums.

Suppose p > 2 (and p < 1093) [Cohen 1995, Proposition 9.1.20].
Let

beta = sum{p^k/2 < x < p^k, gcd(x, p) = 1: (sigma_x)^(-1)}.

Then (zeta_p)^beta is not equal to 1, and condition (*beta) is
equivalent to the congruence

j(chi, chi)^alpha == eta(chi)^(-c*N)  (mod N),

where

alpha = sum{0 < x < p^k, gcd(x, p) = 1:

[N*x/p^k]*(sigma_x)^(-1)}

and

c = 2*(2^((p - 1)*p^(k - 1)) - 1)/p^k.

Suppose p = 2 and k > 2 [Cohen 1995, Corollary 9.1.23]. Let

beta = sum{0 < x < 2^k, x == 1 or 3 (mod 8):

[3*x/2^k]*(sigma_x)^(-1)}.

Then (zeta_p)^beta is not equal to 1, and condition (*beta) is
equivalent to the congruence

(j(chi, chi)*j(chi, chi^2))^alpha *

j(chi^(2^(k - 3)), chi^(3*2^(k - 3)))^(2*delta) ==

(-1)^delta * eta(chi)^(-c*N)  (mod N),

where

alpha = sum{0 < x < 2^k, x == 1 or 3 (mod 8):

[N*x/2^k]*(sigma_x)^(-1)},

c = 3*(3^(2^(k - 2)) - 1)/2^k

and

delta = 0 if N == 1 or 3 (mod 8),

delta = 1 if N == 5 or 7 (mod 8).

Suppose p = 2 and k = 2. Then condition (*1) is equivalent
to the congruence

j(chi, chi)^((N - 1)/2) * q^((N - 1)/4) ==

eta(chi)^(-1)  (mod N)

if N == 1 (mod 4), and to the congruence

j(chi, chi)^((N + 1)/2) * q^((N - 3)/4) ==

- eta(chi)     (mod N)

if N == 3 (mod 4).

Suppose p = 2 and k = 1. Then condition (*1) is equivalent
to the congruence

q^((N - 1)/2) == eta(chi)  (mod N)

if N == 1 (mod 4), and to the congruence

q^((N - 1)/2) == - eta(chi)  (mod N)

if N == 3 (mod 4).

Condition L_p

For prime p, we say that condition L_p is satisfied if for all
prime divisors r of N and all integers a > 0 we can find an
integer l_p(r, a) such that

r^(p - 1) == N^((p - 1)*l_p(r, a))  (mod p^a).

Assume that we can find a character chi modulo q of order p^k
and a beta, with (zeta_p)^beta not equal to 1, for which
(*beta) is satisfied. Then condition L_p is satisfied if the
following additional condition is true:

p > 2 and N^(p - 1) is not congruent to 1 (mod p^2).

However, if also eta(chi) is a primitive p^k-th root of unity,
then condition L_p is satisfied if one of the following
additional conditions is true [Cohen 1995, Proposition 9.1.17]:

(i)    p > 2

(ii)   p = 2, k = 1 and N == 1 (mod 4)

(iii)  p = 2, k >= 2 and q^((N - 1)/2) == -1 (mod N).

In case (iii) N is composite if it fails the congruence;
q^((N - 1)/2) == 1 (mod N) is ruled out by condition (*beta)
with eta(chi) a primitive 2^k-th root of unity.

Checking the divisors d of N

Assume gcd(S, F1*F2) = 1, Let G = F1*F2*S/2. We require G^3 > N.

From the conditions (1), (2) and (3) above, by the Chinese
Remainder Theorem, d must belong to one of at most 2*T residue
classes mod G. We compute the representatives of these residue
classes which are less than G and try them one by one as
possible non-trivial divisors of N. If there are none, we
can conclude that d > G.

If G > N^(1/2), N is prime.

Otherwise, let F be the greater of F1 and F2. Let H = G*F^2.
In addition to G > N^(1/3), the final condition we require
is that m*H > N, where m is not too large. For then we can
complete a primality proof by means of one of the following
theorems.

Theorem 1. Suppose N is composite, suppose N - 1 = F*R,
where F is even, R is odd, R > 1 and gcd(F, R) = 1.
Suppose also that for each prime factor f of F, there is
an integer a, 1 < a < N-1, such that a^(N-1) == 1 (mod N)
and gcd(a^((N-1)/f) - 1, N) = 1. If every prime factor of
N is greater than G, then N is the product of two primes,
c*F + 1 and d*F + 1. Furthermore, if

N < G*(2*A*F^2 - G + r*F + 2)

for some A >= 1, where r and s are defined by R = 2*F*s + r,
0 < r < 2*F, then 2*(s - A) < c*d <= 2*s.

Theorem 2. Suppose N is composite, suppose N + 1 = F*R,
where F is even, R is odd, R > 1 and gcd(F, R) = 1.
Suppose there exists an integer D for which (D/N) = -1 and
also for each prime factor f of F a Lucas sequence {U(i)}
with discriminant D such that N divides U(N+1) and
gcd(U((N+1)/f), N) = 1. If every prime factor of N is
greater than G, then N is the product of two primes,
c*F + e and d*F - e, where e = 1 or -1. Furthermore, if

N < G*(2*A*F^2 + G - |r*F - 2|)

for some A >= 1, where r and s are defined by R = 2*F*s + r,
|r| < F, then 2*(s - A) < c*d <= 2*(s + A).

Representation of numbers

A number in Z/(N*Z)[zeta_p^k] is represented by a p^k-dimensional
vector (A)_i with value

sum{0 <= i < p^k: A_i * (zeta_p^k)^i}.

A number with A_i = 0 for (p - 1)*p^(k - 1) <= i < p^k is said
to be normalized. For recognizing a root of unity in the (*bete)
test it is sufficient for the vector to be normalized and reduced
modulo N.

There are four basic operations. Let (A)_i and (B)_i be two
vectors.

Normalization. We normalize (A)_i by subtracting A_i from
A_{i - (p - 1)*p^(k - 1) + m*p^(k - 1)}, m = 0, 1, ..., p - 1,
for (p - 1)*p^(k - 1) <= i < p^k. We are thus using the
property

sum{i = 0 to p - 1: (zeta_p^k)^(i*p^(k-1))} = 0

of zeta_p^k.

Addition. This is trivial;

(A + B)_i  = A_i + B_i.

Multiplication. We compute

(A*B)_i = sum{j, k, j + k == i (mod p^k): A_j * B_k},

0 <= i < p^k, and normalize.

Sigma_x. To obtain sigma_x((A)_i) we move each term A_i to
A_{i * x mod p^k} and normalize.


File VFYPR.INP (created by the user)

Numbers to be tested, each on a separate line. A zero
terminates the list. Expressions may be used.

If this file does not exist, input is via the keyboard.

File VFYPR.PRM (created by the user)

List of verified primes to try as possible factors of
N^2 - 1. Each on a separate line. A zero terminates the
list.

It is Vitally Important that VFYPR.PRM contains only
primes that have been verified. Otherwise the algorithm
is Not Valid.

You can verify the primes in VFYPR.PRM by copying them
into VFYPR.INP and running VFYPR. If no composite
numbers are detected, then they are all true primes.

File VFYPR.FAC (created by VFYPR)

List of prime factors of N^2 - 1 found by the program.

factor exponent +/-1

Interfering with this file may invalidate the algorithm.

File VFYPR.INI (created by VFYPR)

Controls the restart mechanism. This file contains
information that was saved by the program for the last
number tested. If the program program was interrupted,
you can get it to carry on from where it left off by
by using the 'r' option.

0 N F1 F2 S T r 0 0 : N was completed.
1 N F1 F2 f t m n 0 : Restart BLS test from factor f.
2 N F1 F2 S T i j L : Restart APRCL main test at (i j).
3 N  1  1 S T i 0 0 : Restart APRCL divisor test at i.
4 N F1 F2 S T i 0 0 : Restart main divisor test at i.
5 N F1 F2 S T t r i : Restart final divisor test.

Interfering with this file may invalidate the algorithm.

File VFYPR.DAT (created by VFYPR)

Test results. Echoed on the screen.

File VFYPR.JNL (created by VFYPR)

Journal. Used for debugging.

Run parameters

VFYPR.EXE r k a h C D J F100000 S100000

r    Restart unfinished work, if any.
k    Get input from keyboard. Ignore VFYPR.INP file.
a    APRCL test only. Do not use factors of N^2 - 1.
h    Halt if N is not prime.
C    Do the current segment only.
D    Debug mode. Does various debugging functions.
J    Journal mode. Logs much diagnostic data to the journal.
F#   Specify an upper limit for small factors of N^2 - 1.
S#   Specify a minimum value of S for the APRCL test.

Stopping and restarting

Every so often during a long run parameters are saved so that
the test can be resumed without too much loss of work. If your
computer crashes, or you want to stop a test for some other
reason, when you restart VFYPR with option 'r', it will carry
on from the last checkpoint.

To stop the program gracefully, press upper-case 'Q' on the
keyboard. Wait for the 'VFYPR stopped' message to appear.
Usually this takes a few seconds but if N is large, it could
be several days. Then press 'X'.

If you change your mind and want to continue, press ENTER.
Pausing the program in this way can be useful. It temporarily
closes the log (VFYPR.DAT) thereby allowing you to view it with
your favourite editor before restarting the program.

Usage notes

1.    If N^2 - 1 has a lot of small prime factors, run
VFYPR.EXE without any options. The small factors will get
detected and used for the BLS tests.

2.    If there is nothing special about N, you can consider
doing a pure APRCL test. Use option 'a': VFYPR.EXE a.
This saves the trouble of trial-dividing N^2 - 1 by
small primes.

3.    If N^2 - 1 has known, large prime factors, either by
design or because you have extracted them: First verify
that the factors are true primes by running them past
your favourite primality-prover. Then put them in
VFYPR.PRM and run VFYPR.EXE without any options. VFYPR
searches VFYPR.PRM first, then it does 2 and if it
still needs more factors, it tries small primes up to a
limit which you can specify by the 'F' parameter. If
the factors of N^2 - 1 used by VFYPR are not all proven
primes, the algorithm is not valid.

4.    It is possible to split the APRCL tests into segments
that can be run on separate computers.

Start VFYPR with option 'C' and let it run until the
APRCL main tests for p=2 start to appear. Pause VFYPR by
pressing Q and waiting for the 'VFYPR stopped' message
to appear. Save VFYPR.INI. Make a note of the parameter
T, which is the second number printed in the 'Parameters
saved' message. Restart VFYPR by pressing ENTER. It will
eventually stop when it has completed the APRCL main
tests for p=2.

Meanwhile, make several copies of the saved VFYPR.INI
file. The last three numbers (which are all together on
the last line of VFYPR.INI) must be set for each prime
factor p of T according to the following scheme

2 1 0       APRCL main tests for p=3,
3 1 0       APRCL main tests for p=5,
4 1 0       APRCL main tests for p=7,
5 1 0       APRCL main tests for p=11,
6 1 0       APRCL main tests for p=13,
7 1 0       APRCL main tests for p=17.

Make a final copy of the saved VFYPR.INI with the first
number (in the first line) set to

3           for an APRCL-only test,
4           otherwise,

and the last three numbers set to

1 0 0

Do not alter any other numbers in VFYPR.INI.

Start VYRPR with parameters 'Cr' (or 'Cra' for an APRCL-
only test) on each computer. If you are sending the work
to other people, you should set up a batch job with just
one line in it, VFYPR.EXE Cr (or VFYPR.EXE Cra), and ask
them to return the VFYPR.DAT file.

When each computer completes its work, it will end with
the 'Cannot tell' message. This is OK. However, once you
have collected together the data from all the separate
VFYPR.DAT files, you will have your primality proof.

5.    VFYPR will accept arithmetical expressions in VFYPR.INP,
VFYPR.PRM and stdin.

Expressions consist of decimal digits, '+', '-', '*',
'/' (integer division with truncation towards zero),
'@' (modulo), '^' (exponentiation), '!' (factorial),
'#' (prime product), 'q' (integer square root) and
brackets to a reasonable level. A semicolon terminates
the expression.

'+' and '-' can be prefixes; '-' negates what follows;
'@' always gives a non-negative result; x# is the product
of the primes <= x; qx = [x^(1/2)].

The order of precedence is '(' and ')'; '!' and '#'; prefix
'+', prefix '-' and 'q'; '^'; '*', '/' and '@'; '+' and '-'.
Exponentiation is left-to-right.

Known problems

1.    The program may not work correctly if N > 10^300000.

2.    Although Carmichael numbers are quickly declared
composite by a pure APRCL test, a hybrid test sometimes
appears to loop forever trying to find a number a for
which gcd(a^((N - 1)/2 - 1, N) = 1.

History

1.0   Adapted from UBASIC VFYPR 1.17F and UBASIC APRT-CLE.

1.1   In the APRCL test there is no need to do q's which do
not divide S, provided the L_p switch has been set.

Print-and-log routine added.

Messages, documentation and comments improved.

Double-checking routines added.

Added 'APRCL-only' option, which does a pure APRCL test
without using factors of N^2 - 1.

BUG! We omitted a vital 'continue' statement; the one
corresponding to line 730 in APRT-CLE. As a result, the
L_p condition was possibly getting triggered when it
shouldn't have been. Also it was possible that a prime
would be reported as composite. Fortunately, large
numbers were not affected. In this version I indicate
for each p when the L_p condition is satisfied.

Bug. We must save the state of the L_p switch in APRCL
between restarts. Otherwise, it is possible that a very
long test restarted near the end might run out of q's.

1.2   Journalling functions added. Mainly for comparing with
APRT-CLE.

Use wmulmod(a, b, c) rather than (a*b) % c. It's safer.

1.3   Bug. One of the messages in BLS_test should read
'Pass: gcd(a^((N-1)/f) - 1, N) = 1: R20= ...' (not
'Pass: gcd(a^((N-1)/f), N) = 1: R20= ...'). The code is
correct, however.

Integer power test omitted. But now we need (F1+1)^2
> N in BLS_test.

Write APRCL message after each (p, q) test is done.

1.4   Added a fast 'Is it square?' test; CCANT book, page 40.

Prompt for parameters if none specified.

Small bug in the reporting of aprcl_Lp corrected.

Restart is disabled if APRCL 'Retry' code is executed.

Bug in restarting BLS tests corrected. We need to save
t as well as f in the restart file.

Bug! We should check gcd(N, q) = 1 in BLS_test. (But
if q < prmdiv_hi, this is true anyway.) Selection of p
and q changed.

Reduce BLS test factor search limit from 10^6 to 10^5.

Bug! Must try to remember: fgets - NULL, fscanf - EOF.

1.5   Added a simple expression-evaluator. Expressions are now
accepted in VFYPR.INP, VFYPR.PRM and from stdin.

1.6   Close the log after parameters saved; otherwise it will
be lost if the program terminates unexpectedly.

Another level added. Level 13: T = 64*27*25*7*11*13*17 =
735134400. Also q limit is now 1201201. The upper limit
for an APRCL-only test is now 10^2982.

1.7   Changes necessary for AR2 version 2.2.0. The executable
is now called VFYPR.EXE.

Two new parameters: F_max limits the search for small
factors of N^2 - 1. S_min is the minimum value of S for
an APRCL test. Specify as run time parameters F# and S#,
for example, 'F100000 S100000'.

APRCL divisor test and main divisor test: No need to test
divisors that are greater that sqrt(N).

1.8   Reduce N mod S first in the APRCL and main divisor tests.

Code in vfypr() rearranged.

Bug! BLS_test() loops on certain numbers. When we compute
gcd(a^((N-1)/f) - 1, N) or gcd(U_((N + 1)/f), N) we must
report N as composite if the gcd is between 1 and N.

Improved small-factor search.

1.9   No BLS tests for N - 1 if F1 <= 2 because, trivially,
d divides N -> d == 1 (mod 2). Nor N + 1 if F2 <= 4
because d divides N -> d == +/-1 (mod F2) for F2 = 2,
3 or 4.

Log occurrences of gcd(a^((N-1)/f) - 1, N) != 1 and
gcd(U{(N+1)/f}, N) != 1 in BLS_test.

1.10  Small bug. In final_divisor_test, when t = 1 we should
test r >= F, not r > F. However, this is merely cosmetic
because r cannot equal F.

BUG in imul() corrected.

1.11  Routine MatchingRoot() renamed unity_root_J(). This tests
to see if the Jacobi sum calculation results in a
p^k-th root of unity, in which case Hj is set equal to
this root. If (*beta) is satisfied, Hj is the result
of the left hand side of the (*beta) congruence.

New routine added: unity_root_C() computes Hc, the
exponent of zeta_p^k on the right hand side of the
(*beta) congruence. It is a final double-check of the
Jacobi sum computation that resulted in Hj. If Hj is
not equal to Hc: we declare N composite if L_p is not
satisfied, otherwise it's a bug!

In the APRCL 'done' message, report g, the primitive
root modulo q, and Hj, the p^k-th root of unity returned
from unity_root_J().

It is worth pointing out that Hj is the result of a long
and delicate computation of a certain product of powers
of Jacobi sums modulo N, a computation that would almost
certainly produce gibberish in the event of a hardware
error or a mistake in the implementation of the algorithm.
On the other hand, Hc is a simple calculation involving
only integers modulo p^k. The verification that Hj = Hc
provides very strong evidence that the program and the
computer are functioning perfectly.

1.12  Bug! q_r_mod_N(a) should be i^(a & N & 2)*kro(N mod a, a),
not i^(a & N & 2)*kro(a, N mod a).

1.13  Compute f(x) = ind_g(1 - g^x) on the fly rather than
using a precomputed table. It saves a lot of bytes and
there is no noticeable effect on performance.

Compute ind_g(x) using half-size vIndx table and
ind_g(-x) == (q - 1)/2 + ind(x) (mod q - 1).

In the 'APRCL main test done' message we now report the
last 20 digits of the trace of j(0, p, q)^[N/p^k] mod N.

kQmax increased from 1201201 to 6283201. Hence level 12
(T = 64*27*25*7*11*13) will now handle numbers of up
to 1672 digits and level 13 (T = 64*27*25*7*11*13*17)
numbers of up to 3317 digits.

New run parameter 'C'. Causes VFYPR to stop when it has
completed the current segment. This is useful if a
large N is to be processed by several machines. The
segments are the BLS tests together with the APRCL main
tests for p = 2, the APRCL main tests for each odd prime
factor of T, and the divisor tests.

Format of VFYPR.INI changed slightly.

1.14  Don't rely on default type = long.

1.15  BUG! Numbers in the region of 2^500 crash the program with
message 'Invalid parameter in idiv call'. A freak rounding
condition causes the initial estimate for sqrt(x) to be
negative. Routine isqrt modified to ensure that this does
not happen. Thanks to Petr Simon.

Ideas and observations

1.    Sort out a sensible naming convention. Use large-integer
variables more efficiently. Tidy up the Jacobi sum
computations. Index arrays from 0.

2.    Try and find some awkward numbers to test.

3.    Numbers like m*n! + 1 and m*n! - 1 (where m is large
and unfactorizable) could be problematical because of the
difficulty of finding q's which do not divide n!.

4.    Each (p, q) pair in the APRCL test is independent and
therefore the APRCL main test can be restarted with
just the few parameters that are saved. However, the
journal output differs from that of APRT-CLE in a few
places. Clearly, it is because we (VFYPR and APRT-CLE)
leave the Jacobi vectors dirty between (p, q)-tests,
and when VFYPR is restarted they are clean.

5.    The 'Retry' code is executed when the L_p condition
remains unsatisfied after all the q's for a given p have
been tested. We must obtain more q's and test them until
one of them sets L_p. However, I do not include these
new q's in S. APRT-CLE does. I believe it is not useful
to do so.

For example, a q introduced for the first time when
p = 3, will not have been tested when p = 2. Therefore
it is better to exclude it from S, for otherwise there
is the possibility that not all the (*beta) conditions
will be met.

6.    I am still using the same method as APRT-CLE for the
selection of q's, except that q's which are factors of
F1*F2 are not included in S. Therefore I do not test q's
which do not belong to S once the L_p switch has been
set. These q's are identified in the log as 'not needed'.

7.    Using the journal to compare VFYPR and UBASIC APRT-CLE.
Put the following in a directory:

VFYPR.EXE, VFYPRTJ.UB, VFYPRTK.EXE, UBASIC

VFYPRTJ.UB is a version of APRT-CLE modified to accept
input from VFYPR.INP and log data to VFYPRTJ.JNL.

Delete VFYPR.INI, VFYPR.DAT, VFYPR.JNL and VFYPRTJ.JNL
if any of these exists. Do VFYPR.EXE aJ. Then start
UBASIC and run VFYPRTJ.UB. Wait until both have finished.
Then run VFYPRTK.EXE. This will compare the two journals.

A number of 300 digits requires about 2M disk space.

Stopping and restarting VFYPR will give misleading
results.

8.    In the absence of suitable numbers to test, we need to
invent bizarre restart situations for testing the parts
of the code that normal numbers can't reach. For example,

3 1000000000124000000003843 1 1 1000000000062 60 7 0 0

'restarts' aprcl_divisor_test which should then find the
divisor 1000000000061 = N^7 (mod 1000000000062) of N.

9.    We stop the main divisor test when W == 1 (mod G). For
then W == 1 (mod S) and, as with the APRCL divisor test,
there will be no new values of N^i (mod S) to consider.
It can happen that N^t == 1 (mod S), with t | T and
t < T; for example, if N = (10^86 + 1)/(101*338669),
only 840 out of 2520 divisors are tested.

10.   Let F = max(F1, F2), let R = min(R1, R2), let B be the
upper limit for trial-dividing R by small primes and let
G' = F*B. If G' > N^(1/3) and G'*F^2/N is not too small,
we can use G' instead of G. We would need an extra BLS
test: either a^(N-1) = 1 (mod N) and gcd(a^F - 1, N) = 1,
or U_(N+1) = 0 (mod N) and gcd(U_F, N) = 1.

11.   Testing of P7 (2310 digits), the eigth in a chain of
primes starting with P0 = 2185103796349763249 and linked
by p -> (p^2 + 1)/2. F1: 517 digits, F2: 6 digits, S: 637
digits, T = 21621600. Time: 19.7 days on an AMD/K6/2/400.

References

[BLS 1975] J. Brillhart, D.H. Lehmer and J.L. Selfridge, 'New
Primality Criteria and Factorizations of 2^m +/- 1', Math. Comp.,
29 (1975), 620-647.

[Cohen 1995] Henri Cohen, A Course in Computational Algebraic
Number Theory, Springer-Verlag, Berlin, 1995.

H. Cohen and H.W. Lenstra, 'Primality testing and Jacobi Sums',
Math. Comp. 42 (1984), 297-330

H. Cohen and A.K. Lenstra, 'Implementation of a new primality test',
Math. Comp. 48 (1987), 103-121.

Acknowledgements

I acknowledge the work of the following people who created the
UBASIC program APRT-CLE, from which the APRCL sections of this
program were derived:

Original programming by Koichiro Akiyama, 1988.
Modified for UBASIC version 7 by Yuji Kida, 1989.
Amended by Frank O'Hara, 1990.
Extended to 844 digits Yuji Kida and Frank O'Hara, 1991.

I would like to thank Harvey Dubner for help with the development
and testing of VFYPR.

*/

#include <stdio.h>

#include "ar2u.h"

#define kNPdim    (7 + 1)
#define kPWdim    (64 + 1)
#define kNQdim    (388 + 1)
#define kQdim     ((6283201 + 1)/2)
#define kLEVELdim (13 + 1)

/* 388 primes */
long pr_tab[kNQdim] =
{
	0, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
	31, 37, 41, 43, 53, 61, 67, 71, 73, 79,
	89, 97, 101, 103, 109, 113, 127, 131, 137, 151,
	157, 181, 193, 199, 211, 239, 241, 271, 281, 307,
	313, 331, 337, 353, 379, 397, 401, 409, 421, 433,
	443, 449, 463, 521, 541, 547, 577, 601, 613, 617,
	631, 661, 673, 701, 757, 859, 881, 911, 919, 937,
	953, 991, 1009, 1021, 1051, 1093, 1123, 1171, 1201, 1249,
	1301, 1321, 1327, 1361, 1429, 1531, 1601, 1801, 1871, 1873,
	1951, 2003, 2017, 2081, 2113, 2143, 2161, 2311, 2341, 2377,
	2381, 2521, 2551, 2731, 2801, 2857, 2861, 2971, 3061, 3121,
	3169, 3301, 3361, 3433, 3511, 3571, 3673, 3697, 3851, 4159,
	4201, 4421, 4591, 4621, 4801, 4951, 5101, 5237, 5281, 5441,
	5851, 6007, 6121, 6301, 6337, 6427, 6553, 6733, 7151, 7393,
	7481, 7489, 7561, 7723, 8009, 8161, 8191, 8317, 8581, 8641,
	8737, 9181, 9241, 9283, 9521, 9829, 9901, 10099, 10711, 11551,
	11701, 11969, 12097, 12241, 12377, 12601, 12853, 13729, 14281, 14401,
	14561, 14851, 15121, 15401, 15601, 15913, 16381, 16633, 16831, 17137,
	17551, 17681, 17851, 18481, 18701, 19009, 19801, 19891, 20021, 20161,
	20593, 21601, 21841, 22441, 23563, 23761, 23801, 23869, 24481, 24571,
	25741, 26209, 27457, 27847, 28051, 28081, 29173, 29921, 30241, 30941,
	33151, 33601, 34273, 34651, 36037, 36721, 37441, 38611, 39313, 40801,
	42433, 42841, 42901, 43201, 43759, 44201, 46411, 47521, 47737, 48049,
	50051, 51481, 52361, 53551, 53857, 54601, 55441, 56101, 59671, 62401,
	63649, 65521, 66301, 66529, 70201, 70687, 72073, 72931, 74257, 77351,
	78541, 79201, 79561, 81901, 83777, 87517, 91801, 92401, 92821, 93601,
	96097, 97241, 100801, 100981, 102103, 103951, 107101, 107713, 108109, 109201,
	110881, 116689, 117811, 118801, 120121, 122401, 123553, 127297, 128521, 131041,
	140401, 145601, 145861, 148513, 150151, 151201, 157081, 160651, 161569, 167077,
	180181, 185641, 192193, 193051, 196561, 198901, 200201, 201961, 205633, 209441,
	212161, 216217, 218401, 224401, 232051, 235621, 238681, 243101, 257401, 269281,
	270271, 291721, 300301, 314161, 321301, 323137, 332641, 367201, 371281, 388961,
	393121, 411841, 415801, 417691, 428401, 432433, 436801, 445537, 448801, 450451,
	471241, 477361, 495041, 504901, 514081, 530401, 538561, 540541, 565489, 571201,
	576577, 600601, 612613, 617761, 656371, 673201, 680681, 700129, 729301, 734401,
	786241, 795601, 800801, 816817, 897601, 960961, 982801, 1029601, 1093951, 1108801,
	1178101, 1201201, 1458601, 1633633, 1670761, 1713601, 1837837, 1909441, 2088451, 2187901,
	2402401, 2570401, 2625481, 2673217, 2692801, 2702701, 3088801, 3141601, 3712801, 4084081,
	4200769, 4324321, 4455361, 5105101, 5250961, 5654881, 5834401, 6283201
};

long vIndx[kQdim];
long vP[kNPdim];
long vInv[kPWdim];
long vQmem[kNQdim];
long vQ[kNQdim];
long vG[kNQdim];
long vNP[kLEVELdim];
long vNQ[kLEVELdim];
long vT[kLEVELdim];

long kNPmax;
long kPWmax;
long kQmax;
long kLEVELmax;
long kTmax = 64 * 27 * 25 * 7 * 11 * 13 * 17;

long kLEVELnow;
long kTestedQs;
long kTestingQs;
long kNQall;

long kPK;
long kPL;
long kPM;
long kQM;
long kQH;
long kNP;
long kVK;
long kIV;
long kX;
long kInvX;
long aprcl_Lp;

long kG;
long kHj;
long kHc;
long kI;
long kJ;
long kK;
long kP;
long kQ;
long kS;
long kT;

ireg vrJS[kPWdim];
ireg vrJW[kPWdim];
ireg vrJX[kPWdim];
ireg vrJ0[kPWdim];
ireg vrJ1[kPWdim];
ireg vrJ2[kPWdim];
ireg vrJ[2][kPWdim];

#define rJS &vrJS
#define rJW &vrJW
#define rJX &vrJX
#define rJ0 &vrJ0
#define rJ1 &vrJ1
#define rJ2 &vrJ2
#define rJ  &vrJ

ireg sr0;
ireg sr1;
ireg rrD;
ireg rrDsqu;
ireg rrE;
ireg rrG;
ireg rrH;
ireg rrN;
ireg rrNm1;
ireg rrNmS;
ireg rrsqrtN;

ireg* r0 = &sr0;
ireg* r1 = &sr1;
ireg* rD = &rrD;
ireg* rDsqu = &rrDsqu;
ireg* rE = &rrE;
ireg* rG = &rrG;
ireg* rH = &rrH;
ireg* rN = &rrN;
ireg* rNm1 = &rrNm1;
ireg* rNmS = &rrNmS;
ireg* rsqrtN = &rrsqrtN;

ireg rra;
ireg rrc;
ireg rrf;
ireg rrF1;
ireg rrF2;
ireg rrR1;
ireg rrR2;

ireg* ra = &rra;
ireg* rc = &rrc;
ireg* rf = &rrf;
ireg* rF1 = &rrF1;
ireg* rF2 = &rrF2;
ireg* rR1 = &rrR1;
ireg* rR2 = &rrR2;

ireg ssF1;
ireg ssF2;
ireg ssRr;
ireg ssRs;
ireg ss2F;
ireg ss2Fsqu;

ireg* sF;
ireg* sR;

ireg* sF1 = &ssF1;
ireg* sF2 = &ssF2;
ireg* sRr = &ssRr;
ireg* sRs = &ssRs;
ireg* s2F = &ss2F;
ireg* s2Fsqu = &ss2Fsqu;

ireg rrS;
ireg rrU;
ireg rrUj;
ireg rrV;
ireg rrW;
ireg rrWj;
ireg rrestart_s;
ireg rres20;

ireg* rS = &rrS;
ireg* rU = &rrU;
ireg* rUj = &rrUj;
ireg* rV = &rrV;
ireg* rW = &rrW;
ireg* rWj = &rrWj;
ireg* restart_s = &rrestart_s;
ireg* res20 = &rres20;

ireg rrcF1;
ireg rrcF2p;
ireg rrcF2m;
ireg rrcS;

ireg* rcF1 = &rrcF1;
ireg* rcF2p = &rrcF2p;
ireg* rcF2m = &rrcF2m;
ireg* rcS = &rrcS;

ireg rrx;
ireg rry;
ireg rrz;
ireg rrw1;
ireg rrw2;
ireg rrw3;

ireg* rx = &rrx;
ireg* ry = &rry;
ireg* rz = &rrz;
ireg* rw1 = &rrw1;
ireg* rw2 = &rrw2;
ireg* rw3 = &rrw3;

char qres11[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char qres63[63];
char qres65[65];
char qres256[256];

#define buffer_dim 16000
char buffer[buffer_dim];
char* cfirst;

#define prmdiv_hi 65536

time_t ptime;
time_t ttime;

double psecs;
double tsecs;

long pcount;
long quit;
long restart;
long restart_t;
long restart_i;
long restart_j;
long restart_k;

long aprcl_only;
long debug;
long journal;
long halt_not_prime;
long single_segment;

long F_max;
long S_min;

long num_fac_F1;
long num_fac_F2;
long p;
long q;

long cd_num;
long cd_tot;
long cd_real;
long cd_int;

double ilogN;

FILE* inp_file;
FILE* dat_file;
FILE* jnl_file;
FILE* ini_file;
FILE* prm_file;
FILE* fac_file;

#define iwd0(areg)  (areg->value[0])
#define iodd(areg)  (areg->value[0] & 1)
#define ieven(areg) !(areg->value[0] & 1)

long test_quit(void);
void halt(void);
void gen_fail(void);
void file_fail(char* f, char* t);
long instr(char* a, char b);
long iparm(char* a, long b, long c);
void get_res20(ireg* a);
void get_APRCL_res20(void);
void lprint(char* s, ...);
void iprint(char* s, ireg* a, ...);
void iprtcr(char* s, ireg* a, ...);
void jprint(char* c, ireg* a, ...);
void jprint_vector(char* c, long* v, long a, long b);
void jprint_jacobi(char* c, ireg* v);
void jprint_aprcl_main(long a);
long iprmdiv(ireg* a);
long prmdiv(long a);
long kro(long a0, long b0);
long q_r_mod_N(long a);
long prob_square(ireg* a);
void arithmetic_check(void);
void save_parameters(long r, ireg* s, long t, long i, long j, long k);
void expmod_Lucas(ireg* xW, ireg* xU, ireg* xWj, ireg* xUj, ireg* xk, long p, long q, ireg* xM);
void expmod_Lucas_check_print(char* a, ireg* rw, ireg* ru, long p, long q, long k, ireg* rm);
void expmod_Lucas_check_single(long k1, long k2, long k3, long p, long q);
void expmod_Lucas_check(void);
long test_sufficient(long q, long t, long m);
long BLS_check(ireg* rF, ireg* rR, ireg* sF, long num_fac_F, long t);
long BLS_test_factor(void);
void get_next_p_q(void);
long BLS_test(void);
void load_pr_tab(long maxQ);
long ind(long x);
long Genshi(long p);
long PowerIndex(long p, long a);
void NormalizeJW(void);
void NormalizeJS(void);
void JS_2(void);
void JS_JW(void);
void JS_E(void);
void GetInverseTable(void);
void JacobiSum(long a, long b);
void GetIndexTable(void);
long unity_root_J(void);
long unity_root_C(void);
void CalcJacobiPQVs(void);
void CalcJPQ0uJPQv(void);
void PrepareJacobis(void);
void GetQsGs(long t);
void GetAllQs(long t);
long GetPrimesToTest(void);
void GetAllPrimes(void);
void set_aprcl_Lp(void);
void aprcl_main_init(void);
long aprcl_main_test(void);
long aprcl_divisor_test(void);
long main_divisor_check(long i, long e);
long main_divisor_test(void);
long final_divisor_check(void);
long final_divisor_factor(void);
long final_divisor_test(void);
long preliminary_test(void);
long vfypr_init(char* exp, char* parameters);
long vfypr(char* exp, char* parameters);

#endif
