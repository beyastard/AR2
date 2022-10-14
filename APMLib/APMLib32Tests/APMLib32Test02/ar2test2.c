/* AR2TEST2.C  2.2.1F  TONY FORBES  October 1999 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ar2x.h"
#include "ar2u.h"
#include "ar2i.h"
#include "ar2t.h"
#include "ar2e.h"
#include "ar2p.h"

void ttestigcd(char* cx, char* cy);
void ttestigcdext(char* cx, char* cy);
void ttestiexp(char* cx, char* cy, char* cz);
void ttest_xnxtprmk(void);
void ttest_ilog_isqrt(long k);
void ttest_ieval(char *e, char *b);
void ttest_inxtprm_k(long p, long i);
void ttest_inxtprm_i(char* a, char* c);


static ireg ra;
static ireg rb;
static ireg rc;
static ireg rd;
static ireg re;
static ireg rv;
static ireg rw;
static ireg rx;
static ireg ry;
static ireg rz;

static ireg *a = &ra;
static ireg *b = &rb;
static ireg *c = &rc;
static ireg *d = &rd;
static ireg *e = &re;
static ireg *v = &rv;
static ireg *w = &rw;
static ireg *x = &rx;
static ireg *y = &ry;
static ireg *z = &rz;

static double dd;
static long   dl;

int main(int argc, char** argv)
{
	static long i;

	tprint("ARITH2 TEST 2  Version 2.1.6m\n");

	tprint("Check imovd and tidisp\n");
	imovd(x, "0000011111222223333344444555556666677777888889999900000");
	tprint("11111222223333344444555556666677777888889999900000 = \n");
	tidisp(x);
	tprint("\n");
	imovd(y, "-0000011111222223333344444555556666677777888889999900000");
	tprint("-11111222223333344444555556666677777888889999900000 = \n");
	tidisp(y);
	tprint("\n");
	imov(z, x);
	iadd(z, y);
	tprint("0 = ");
	tidisp(z);
	tprint("\n");
	imovd(v, "1");
	tprint("1 = ");
	tidisp(v);
	tprint("\n");
	imovd(w, "-1");
	tprint("-1 = ");
	tidisp(w);
	tprint("\n");
	imov(z, v);
	iadd(z, w);
	tprint("0 = ");
	tidisp(z);
	tprint("\n");
	kres = -1;
	imovk(res, kres);
	imovd(res, "_234");
	imovd(res, "_56789");
	imov(z, res);
	tprint("-123456789 = ");
	tidisp(z);
	tprint("\n");

	tprint("Check inxtprm\n");
	ttest_inxtprm_k(1, 2000);
	ttest_inxtprm_k(1999999000, 2000);
	ttest_inxtprm_k(999999000, 2000);
	ttest_inxtprm_k(99999000, 2000);
	ttest_inxtprm_k(9999000, 2000);
	ttest_inxtprm_k(999000, 2000);
	ttest_inxtprm_i("1000000000060", "1000000000061");
	ttest_inxtprm_i("1000000000061", "1000000000063");
	halt();

	tprint("Check ieval\n");
	ttest_ieval("0", "0");
	ttest_ieval("23", "23");
	ttest_ieval("-23", "-23");
	ttest_ieval("2+2", "4");
	ttest_ieval("2*3", "6");
	ttest_ieval("2*3+4*5", "26");
	ttest_ieval("2*3+4*5+6*7*8", "362");
	ttest_ieval("2^2", "4");
	ttest_ieval("3^3", "27");
	ttest_ieval("4^4", "256");
	ttest_ieval("5^5", "3125");
	ttest_ieval(" 666*(7*(6+7)) ", "60606");
	ttest_ieval(" 666*(7*(6+7)*(2+(91*2^10-3)*2 )) ", "11294776584");
	ttest_ieval(" 666*(7*(6+7)*(2+(91*2^10-3)*2 )+2000) ", "11296108584");
	ttest_ieval(" 23^(9+2) ", "952809757913927");
	ttest_ieval(" 666*(7*(6+7)*(2+(91*2^10-3)*2 )+2000)*23^(9+2) ",
	            "10763042485290472717849368");
	ttest_ieval(" 100000/3", "33333");
	ttest_ieval(" -100000/-3", "33333");
	ttest_ieval(" -100000/3", "-33333");
	ttest_ieval(" 100000/-3", "-33333");
	ttest_ieval(" 100000\\3", "33333"); /* Remember to double back-slashes */
	ttest_ieval(" -100000\\-3", "33333");
	ttest_ieval(" -100000\\3", "-33333");
	ttest_ieval(" 100000\\-3", "-33333");
	ttest_ieval(" 100000@3", "1");
	ttest_ieval(" -100000@-3", "1");
	ttest_ieval(" -100000@3", "2");
	ttest_ieval(" 100000@-3", "2");
	ttest_ieval("20017+2^(432-(3*6)+(6797/92 + 29*(2^1+9)+2))",
	            "1707011694817242694164442058424641996069058130512872489061441999811593532881313810309486643423117898430190057111918909554147533223454557460573019149396692491800360340355587726966548041193424390330615044130786970107312831497593974090537952628273");
	ttest_ieval("(20017+2^(432-(3*6)+(6797/92+29*(2^1 + 9)+2)))@(666*(7*(6+7)*(2+(91*2^10-3)*2)+2000)*23^(9+2))",
	            "324265510515057891816257");
	ttest_ieval("20017*666*(7*(6+7)*(2+(91*2^10-3) * 2)+2000)*23^(9+2)+2^(432-(3*6)+(6797/92+29*(2^1+9)+2))",
	            "1707011694817242694164442058424641996069058130512872489061441999811593532881313810309486643423117898430190057111918909554147533223454557460573019149396692491800360340355587726966548041193424390330615044130786970107528275319022033482931143407512");
	ttest_ieval(
		"20017*666*(7*(6^(2^(3^(3*3)-19680))+7)*(2+(91*2^10-3)*2)+2000)*23^(9+2)+2^(432-(3*6)+(6797/92+29*(2^1+9)+2))",
		"1707011694817242694164442058424641996069058130512872489061441999811593532881313810309486643423117898430190057111918909554147533223454557460573019149396692491800360340355587726966548041193424390330615044130786997939753458282063351568630625229832");
	ttest_ieval("5784278273878234@9999*(3223+88*(773+22)^(7-22+26*(34@3+1)))",
	            "75888644000535572135768265714773967188228169649778139501617676184197001607299072145950631238520145416259779126147");
	ttest_ieval("5784278273878234@9999*-(-3223+-88*(773+22)^(7-22+26*(34@3+1)))",
	            "75888644000535572135768265714773967188228169649778139501617676184197001607299072145950631238520145416259779126147");
	ttest_ieval("-5784278273878234@-9999*-(-3223+-88*(773+22)^-(-7--22+-26*(34@3+1)))",
	            "75888644000535572135768265714773967188228169649778139501617676184197001607299072145950631238520145416259779126147");
	ttest_ieval("---5784278273878234@---9999*---(---3223+---88*(773+22)^-----(---7----22+---26*(34@--3+--1)))",
	            "75888644000535572135768265714773967188228169649778139501617676184197001607299072145950631238520145416259779126147");
	ttest_ieval("qqq(9999999-9999996)!!!",
	            "200399721439662518532539769062294121418795586092388177419634181767901085696190806851793428631318564344885698260611380294362223312662910414463271075698626700051013373191925775625748128787013767938469443905259169888237716");
	ttest_ieval("q20*q21*q22*q23*q24*q25*q26",
	            "25600");
	ttest_ieval("q20!*q21!*q22!*q23!*q24!*q25!*q26!",
	            "3744121748875552526852535447127850026923882450188216789264575259227427900097600");
	ttest_ieval("(-1)!", "1");
	ttest_ieval("0!", "1");
	ttest_ieval("1!", "1");
	ttest_ieval("2!", "2");
	ttest_ieval("3!", "6");
	ttest_ieval("6!", "720");
	ttest_ieval("7!", "5040");
	ttest_ieval("(-1)#", "1");
	ttest_ieval("0#", "1");
	ttest_ieval("1#", "1");
	ttest_ieval("2#", "2");
	ttest_ieval("3#", "6");
	ttest_ieval("6#", "30");
	ttest_ieval("7#", "210");

	tprint("Check ilog, isqrt\n");
	imovd(a, "0");
	dd = ilog(a);
	dl = dd;
	imovk(b, dl);
	tcheckd(b, "0", "ilog", 0);
	imovd(a, "1");
	dd = ilog(a);
	dl = dd;
	imovk(b, dl);
	tcheckd(b, "0", "ilog", 1);
	imovd(a, "2");
	dd = ilog(a);
	dl = dd;
	imovk(b, dl);
	tcheckd(b, "0", "ilog", 2);
	imovd(a, "3");
	dd = ilog(a);
	dl = dd;
	imovk(b, dl);
	tcheckd(b, "1", "ilog", 3);
	imovd(a, "4");
	dd = ilog(a);
	dl = dd;
	imovk(b, dl);
	tcheckd(b, "1", "ilog", 4);
	imovd(a, "5");
	dd = ilog(a);
	dl = dd;
	imovk(b, dl);
	tcheckd(b, "1", "ilog", 5);
	imovd(a, "0");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "0", "isqrt", 0);
	tcheckd(c, "0", "isqrt res", 0);
	imovd(a, "1");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "1", "isqrt", 1);
	tcheckd(c, "0", "isqrt res", 1);
	imovd(a, "2");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "1", "isqrt", 2);
	tcheckd(c, "1", "isqrt res", 2);
	imovd(a, "3");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "1", "isqrt", 3);
	tcheckd(c, "2", "isqrt res", 3);
	imovd(a, "4");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "2", "isqrt", 4);
	tcheckd(c, "0", "isqrt res", 4);
	imovd(a, "5");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "2", "isqrt", 5);
	tcheckd(c, "1", "isqrt res", 5);
	imovd(a, "65535");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "255", "isqrt", 65535);
	tcheckd(c, "510", "isqrt res", 65535);
	imovd(a, "65536");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "256", "isqrt", 65536);
	tcheckd(c, "0", "isqrt res", 65536);
	imovd(a, "65537");
	isqrt(b, a);
	imov(c, res);
	tcheckd(b, "256", "isqrt", 65537);
	tcheckd(c, "1", "isqrt res", 65537);
	ttest_ilog_isqrt(10);
	ttest_ilog_isqrt(21);
	ttest_ilog_isqrt(32);
	ttest_ilog_isqrt(127);
	ttest_ilog_isqrt(128);
	ttest_ilog_isqrt(129);
	halt();

	imulk(v, 10);

	for (i = 1; i <= 12; i++)
	{
		isqu(v);
		tidisp(v);
		tprint(" v=10^2^%d, d(v)=%d\n", i, v->digits);
	}

	tprint("Check igcdext\n");
	ttestigcdext("2900", "33333333333333333333330");
	ttestigcdext("2901", "33333333333333333333330");
	ttestigcdext("2902", "33333333333333333333330");
	ttestigcdext("2903", "33333333333333333333330");
	ttestigcdext("2904", "33333333333333333333330");
	ttestigcdext("2905", "33333333333333333333330");

	tprint("Check iexp and iexpmod\n");
	ttestiexp("1", "333", "1000000000000000000000000000000000000");
	ttestiexp("2", "333", "1000000000000000000000000000000000000");
	ttestiexp("3", "333", "1000000000000000000000000000000000000");
	ttestiexp("10", "333", "1000000000000000000000000000000000000");
	ttestiexp("2901", "33", "1000000000000000000000000000000000000");

	tprint("Check igcd\n");
	ttestigcd("2900", "33333333333333333333330");
	ttestigcd("2901", "33333333333333333333330");
	ttestigcd("2902", "33333333333333333333330");
	ttestigcd("2903", "33333333333333333333330");
	ttestigcd("2904", "33333333333333333333330");
	ttestigcd("2905", "33333333333333333333330");

	ttest_xnxtprmk();

	tcheckr(a, a, "END", 0);
	tprint("All done\n");
	halt();
}

void ttestigcd(char* cx, char* cy)
{
	static long i, j, sx, sy;

	imovd(x, cx); imovd(y, cy);

	for (i = 1; i <= 10; i++)
	{
		for (sx = -1; sx <= 1; sx++)
		{
			for (sy = -1; sy <= 1; sy++)
			{
				j = 1000000 * (sx + 2) + 100000 * (sy + 2) + i;

				imov(a, x);
				imulk(a, sx);
				imov(b, y);
				imulk(b, sy);
				igcd(a, b);

				if (sx || sy)
				{
					imov(c, x);
					imulk(c, sx);
					idiv(c, a);
					imov(z, res);
					tcheckd(z, "0", "igcd res x/a", j);
					imov(d, y);
					imulk(d, sy);
					idiv(d, a);
					imov(z, res);
					tcheckd(z, "0", "igcd res y/a", j);
					igcd(c, d);
					tcheckd(c, "1", "igcd (c, d)", j);
				}
				else
				{
					tcheckd(a, "0", "igcd (0, 0)", j);
				}
			}
		}

		imulk(x, 200);
		imulk(y, 330);
	}
}

void ttestigcdext(char* cx, char* cy)
{
	static long i;
	static long j;
	static long sx;

	imovd(x, cx);
	imovd(y, cy);

	for (i = 1; i <= 100; i++)
	{
		for (sx = -1; sx <= 1; sx++)
		{
			j = 1000 * (sx + 2) + i;

			imov(a, x); imulk(a, sx);
			imov(b, y);
			igcdext(e, d, a, b);
			imov(z, x); imulk(z, sx);
			isub(z, a);
			tcheckd(z, "0", "igcdext a", j);
			imov(z, y);
			isub(z, b);
			tcheckd(z, "0", "igcdext b", j);
			imov(v, x); imulk(v, sx);
			idiv(v, d);
			imov(z, res);
			tcheckd(z, "0", "igcdext res a/d", j);
			imov(w, y);
			idiv(w, d);
			imov(z, res);
			tcheckd(z, "0", "igcdext res b/d", j);
			igcd(v, w);
			tcheckd(v, "1", "igcdext (a/d, b/d)", j);
			imov(v, a);
			imov(w, b);
			igcd(v, w);
			isub(v, d);
			tcheckd(v, "0", "igcdext (a, b) = d", j);
			imov(v, e);
			imul(v, a);
			isub(v, d);
			imod(v, b);
			tcheckd(v, "0", "igcdext u*a = d (mod b)", j);
		}

		imulk(x, 20);
		iaddk(x, 23);
		imulk(y, 33);
		iaddk(x, 47);
	}
}

void ttestiexp(char* cx, char* cy, char* cz)
{
	static long i;
	static long j;
	static long sx;
	static long sy;

	for (sx = -1; sx <= 1; sx++)
	{
		for (sy = -1; sy <= 1; sy++)
		{
			imovd(x, cx);
			imulk(x, sx);
			imovd(y, cy);
			imulk(y, sy);
			imovd(z, cz);
			iexp(a, x, y); /* a = x^y */
			imov(b, a);
			imod(b, z); /* b = x^y (mod z) */
			iexpmod(c, x, y, z); /* c = x^y (mod z) */
			imovk(d, 1);
			imovk(e, 1);
			i = 0;

			while (isgn(y) >= 1)
			{
				imul(d, x); /* d = x^y */
				imul(e, x);
				imod(e, z); /* e = x^y (mod z) */
				isubk(y, 1);
				i++;
			}

			j = 1000000 * (sx + 2) + 100000 * (sy + 2) + i;

			tcheckr(a, d, "iexp a d", j);
			tcheckr(b, c, "iexp b c", j);
			tcheckr(b, e, "iexp b e", j);
		}
	}
}

void ttest_xnxtprmk(void)
{
	static long i;
	static long k;

	k = 0;
	tprint("Check xnxtprmk ");

	for (i = 1; i <= 20000; i++)
	{
		k = xnxtprmk(k);
		tprint("%d ", k);
	}

	tprint("\n");
}

void ttest_ilog_isqrt(long k)
{
	static long i;
	static long dl;
	static long el;

	double dd;
	double ed;

	imovk(a, k);
	ed = log(k);

	for (i = 0; i < 10; i++)
	{
		if (test_quit())
			halt();

		dd = ilog(a);
		dl = dd;
		el = ed;
		imovk(b, dl - el);
		tcheckd(b, "0", "ilog", dl);
		imov(b, a);
		iaddk(b, -2);
		isqrt(c, b);
		imov(d, res);
		isqu(c);
		iadd(c, d);
		isub(c, b);
		tcheckd(c, "0", "isqrt -2", 100 * k + i);
		imov(b, a);
		iaddk(b, -1);
		isqrt(c, b);
		imov(d, res);
		isqu(c);
		iadd(c, d);
		isub(c, b);
		tcheckd(c, "0", "isqrt -1", 100 * k + i);
		imov(b, a);
		isqrt(c, b);
		imov(d, res);
		isqu(c);
		iadd(c, d);
		isub(c, b);
		tcheckd(c, "0", "isqrt 0 ", 100 * k + i);

		if (i > 1)
			tcheckd(d, "0", "isqrt res", 100 * k + i);

		imov(b, a);
		iaddk(b, 1);
		isqrt(c, b);
		imov(d, res);
		isqu(c);
		iadd(c, d);
		isub(c, b);
		tcheckd(c, "0", "isqrt +1", 100 * k + i);

		imov(b, a);
		iaddk(b, 2);
		isqrt(c, b);
		imov(d, res);
		isqu(c);
		iadd(c, d);
		isub(c, b);
		tcheckd(c, "0", "isqrt +2", 100 * k + i);

		isqu(a);
		ed *= 2.0;
	}
}

void ttest_ieval(char* e, char* b)
{
	imov(a, ieval(e));

	if (*eval_err != 0)
		tprint("Error: err=%s \n", eval_err);

	tcheckd(a, b, "ieval", 0);

	if (*eval_err != 0)
		halt();
}

void ttest_inxtprm_k(long p, long i)
{
	static long j;
	static long b;

	imovk(a, p);
	b = p;

	for (j = 0; j < i; j++)
	{
		inxtprm(a);
		b = xnxtprmk(b);
		imov(e, a);
		isubk(e, b);
		tcheckd(e, "0", "inxtprm", p);
	}
}

void ttest_inxtprm_i(char* a, char* c)
{
	imovd(b, a);
	imov(e, b);
	imodk(e, 1000000000);
	inxtprm(b);
	tcheckd(b, c, "inxtprm", iwd0(e));
}
