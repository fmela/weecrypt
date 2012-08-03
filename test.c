#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <time.h>

#include "weecrypt.h"

#if defined(__APPLE__)
typedef uint64_t timer_val;
# define TIMER_FMT		"%.03fms"
# define TIMER_VAL(t)	((t) * 1e-6)
#else
typedef double timer_val;
# define TIMER_FMT		"%.03fms"
# define TIMER_VAL(t)	((t) * 1e3)
#endif

timer_val hrtimer();

void time_binomial(void);
void time_gcd(void);
void test_mpq_acc(void);
void test_set_f(void);
void test_crt(void);
void test_invert_matrix(void);
void test_mpq(void);
void test_from_str(void);
void test_lehmer(void);
void test_mexp(void);
void test_mul_mod_powb(void);
void test_barrett(void);
void test_composite(void);
void test_modinv(void);
void test_gcdext(void);
void time_divexact(void);
void test_divexact(void);
void test_invert(void);
void tune_mul(void);
void test_div(void);
void test_add(void);
void time_copy(void);
void time_modi(void);
void test_dmod(void);
void test_sub(void);
void test_sqr(void);
void test_modexp_u64(void);
void time_modexp(void);
void test_mul(void);
void time_div(void);
void time_mul(void);
void time_mmul(void);
void time_dmul_add(void);
void test_dmuli(void);
void test_gcd(void);
void test_sqrt(void);
void time_sqrt(void);
void time_add(void);
void test_shift(void);
void run_gcd_test(void);
void print_fraction(mp_digit *u, mp_size size, unsigned base);
void twiddle(void);
void gen_fraction(void);
void time_invert(void);

extern int detect_cpuid(void);
extern int detect_mmx(void);
extern int detect_sse(void);
void mp_and_mmx(mp_digit *u, mp_size size, const mp_digit *v);

extern void _mp_mul_base(const mp_digit *, mp_size,
						 const mp_digit *, mp_size, mp_digit *);

int
main(void)
{
	time_gcd();
	time_binomial();
//	test_crt();
//	test_invert_matrix();
//	test_mpq_acc();
//	test_set_f();
//	test_mpq();
	test_modinv();
	test_gcdext();
//	test_from_str();
	test_mexp();
//	test_lehmer();
	test_composite();
//	test_mul_mod_powb();
//	time_copy();
	test_divexact();
	time_divexact();
//	test_invert();
//	time_invert();
//	gen_fraction();
//	time_dmul_add();
//	time_div();
//	tune_mul();
	test_div();
//	test_add();
//	test_sub();
	time_modi();
	test_sqr();

	test_modexp_u64();
	time_modexp();
//	test_barrett();

//	time_mul();
//	test_mul();
//	test_dmuli();
//	test_dmod();
	test_gcd();
//	test_shift();
//	time_mmul();
//	time_add();
	test_sqrt();
//	time_sqrt();
//	run_gcd_test();
	return 0;
}

static void
debug_print(char c, const mp_digit n[], mp_size size)
{
	printf("%c=", c);
	mp_print(n, size, 10);
	printf("\n");
}

void
test_div(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define N	(120)
#define D	(N/2)
#define Q	(N-D+1)
#define R	D
#define NTRIALS	100000
	mp_digit n[N], d[D], q[Q], r[R], t[N+1]; /* D+(N-D+1)=N+1 */
	int i, j, fail=0;

	twiddle();
	for (i=1; i<=NTRIALS; i++) {
		mp_max(n,N);
	//	mp_max(n,N);
		mp_rand(d,D);
		if (d[D-1]==0)
			d[D-1]=rand();
		mp_divrem(n,N,d,D,q,r);
		mp_mul(q,Q,d,D,t);
		if (t[N]) {
			printf("Trial %d failed! T[N]!=0\n", i);
			fail++;
			continue;
		}
		if (mp_addi(t,N,r,R)) {
			printf("Trial %d Failed! Remainder overflowed Q*D!\n", i);
			fail++;
			continue;
		}
		if ((j=mp_cmp(n,N,t,N))!=0) {
			printf("Trial %d failed! N %c Q*D+R\n", i, j<0?'<':'>');
			debug_print('N',n,N);
			debug_print('D',d,D);
			debug_print('Q',q,Q);
			debug_print('R',r,R);
			debug_print('T',t,N);
			fail++;
		}
		if ((i & 16383)==0)
			twiddle();
	}
	printf("%d passed, %d failed.\n", NTRIALS-fail, fail);
	if (fail)
		abort();
#undef N
#undef D
#undef Q
#undef R
#undef NTRIALS
}

void
time_modi(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define N	10
#define D	5
#define R	D
#define TRIALS	100000
	mp_digit n[N], b[N], d[D], r[R];
	timer_val modi_time = 0, mod_time = 0;

	mp_rand(n,N);
	mp_rand(d,D);

	for (int i=0; i<TRIALS; i++) {
		mp_copy(n,N,b);
		timer_val start = hrtimer();
		mp_modi(b,N,d,D);
		modi_time += hrtimer() - start;
	}

	for (int i=0; i<TRIALS; i++) {
		timer_val start = hrtimer();
		mp_mod(n,N,d,D,r);
		mod_time += hrtimer() - start;
	}

	printf(" mod=" TIMER_FMT "\n", TIMER_VAL(mod_time));
	printf("modi=" TIMER_FMT "\n", TIMER_VAL(modi_time));
#undef TRIALS
#undef N
#undef D
#undef R
}

void
test_sqr(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);
#define A		120
#define NTRIALS	2000
#define NITER	20
	mp_digit a[A], b[A * 2], c[A * 2];
	timer_val mul_time=0, sqr_time=0;
	int fail = 0;

	for (int i=0; i<NTRIALS; i++) {
		mp_rand(a,A);

		timer_val start = hrtimer();
		for (int j = 0; j < NITER; ++j)
			_mp_mul_base(a,A,a,A,b);
		mul_time += hrtimer() - start;

		start = hrtimer();
		for (int j = 0; j < NITER; ++j)
			mp_sqr(a,A,c);
		sqr_time += hrtimer() - start;

		if (mp_cmp_ne(b,A*2,c,A*2)) {
			debug_print('A',a,A);
			debug_print('B',b,A*2);
			debug_print('C',c,A*2);
			fail++;
		}
	}
	printf("%d passed, %d failed.\n", NTRIALS-fail, fail);
	printf("multiplication: " TIMER_FMT "\n", TIMER_VAL(mul_time));
	printf("      squaring: " TIMER_FMT " (%.2f%%)\n", TIMER_VAL(sqr_time),
		   100. * (double)sqr_time / (double)mul_time);
#undef A
#undef NTRIALS
#undef NITER
}

void
test_modexp_u64(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define A	20
#define M	10
#define N	10000
	mp_digit a[A], m[M], c[M], d[M];

//	srand((unsigned)time(0));
	timer_val modexp_time = 0, modexp_pow2_time = 0;
	for (int n=0; n<N; n++) {
		unsigned p = rand();
		mp_rand(a,A);
		mp_rand(m,M);

		timer_val start = hrtimer();
		mp_modexp_u64(a,A,p,m,M,c);
		modexp_time += hrtimer() - start;

		start = hrtimer();
		mp_modexp_pow2_u64(a,A,p,m,M,d);
		modexp_pow2_time += hrtimer() - start;
		if (mp_cmp(c,M,d,M)) {
			printf("Trial %d failed:\n", n);
			printf("A="), mp_print_dec(a,A), printf("\n");
			printf("P=%u\n", p);
			printf("M="), mp_print_dec(m,M), printf("\n");
			printf("C="), mp_print_dec(c,M), printf("\n");
			printf("D="), mp_print_dec(d,M), printf("\n");
		}
	}
	printf("     modexp_u64=" TIMER_FMT "\n", TIMER_VAL(modexp_time));
	printf("modexp_pow2_u64=" TIMER_FMT "\n", TIMER_VAL(modexp_pow2_time));
#undef A
#undef M
#undef N
}

void
test_barrett(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);
#define A 45
#define P 5
#define M 30
#define NTRIALS	100
	mp_digit a[A], m[M], p[P], c[M], d[M];
	mp_barrett_ctx ctx = MP_BARRETT_CTX_INITIALIZER;
	timer_val barrett_time = 0, modexp_time = 0;

	twiddle();
	for (int i=0; i<NTRIALS; i++) {
		mp_rand(m,M);
		timer_val start = hrtimer();
		mp_barrett_ctx_init(&ctx, m, M);
		barrett_time += hrtimer() - start;
		for (int j=0; j<10; ++j) {
			mp_rand(a,A);
			mp_rand(p,P);

			start = hrtimer();
			mp_modexp(a,A,p,P,m,M,c);
			modexp_time += hrtimer() - start;

			start = hrtimer();
			mp_barrett(a,A,p,P,&ctx,d);
			barrett_time += hrtimer() - start;

			if (mp_cmp_n(c,d,M)) {
				printf("A="), mp_print_hex(a,A), printf("\n");
				printf("P="), mp_print_hex(p,P), printf("\n");
				printf("M="), mp_print_hex(m,M), printf("\n");
				printf("C="), mp_print_hex(c,M), printf("\n");
				printf("D="), mp_print_hex(d,M), printf("\n");
			}
		}
		start = hrtimer();
		mp_barrett_ctx_free(&ctx);
		barrett_time += hrtimer() - start;

		if ((i&0xf)==0)
			twiddle();
	}
	printf(" modexp=" TIMER_FMT "\n", TIMER_VAL(modexp_time));
	printf("barrett=" TIMER_FMT "\n", TIMER_VAL(barrett_time));
#undef A
#undef P
#undef M
#undef NTRIALS
}

void
time_modexp(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);
#define A	15
#define B	10
#define P	10
#define C	B
	mp_digit a[A], b[B], p[P], c[C], d[C];
	timer_val s,e;
	int i;
	mp_barrett_ctx ctx = MP_BARRETT_CTX_INITIALIZER;

	mp_rand(a,A);
	mp_rand(b,B);
	mp_rand(p,P);

	s = hrtimer();
	for (i=0; i<100; i++)
		mp_modexp(a,A,p,P,b,B,c);
	e = hrtimer();
	printf(" modexp time=" TIMER_FMT "\n", TIMER_VAL(e-s));

	s = hrtimer();
	for (i=0; i<100; i++)
		mp_mexp(a,A,p,P,b,B,d);
	e = hrtimer();
	printf("   mexp time=" TIMER_FMT "\n", TIMER_VAL(e-s));

	if (mp_cmp_n(c,d,C))
		printf("modexp/mexp don't match!\n");

	mp_barrett_ctx_init(&ctx,b,B);
	s = hrtimer();
	for (i=0; i<100; i++)
		mp_barrett(a,A,p,P,&ctx,d);
	e = hrtimer();
	mp_barrett_ctx_free(&ctx);
	printf("barrett time=" TIMER_FMT "\n", TIMER_VAL(e-s));

	if (mp_cmp_n(c,d,C))
		printf("modexp/barrett don't match!\n");

#undef P
#undef C
#undef B
#undef A
}

void
test_add(void)
{
#define A 27
#define NTRIALS	1
	mp_digit a[A], b[A], c[A+1], d[A+1];
	unsigned i, fail=0;

	for (i=0; i<NTRIALS; i++) {
		mp_rand(a,A);
		mp_rand(b,A);
		c[A]=mp_add_n(a,b,A,c);
		mp_copy(a,A,d);
		d[A]=mp_addi_n(d,b,A);
		if (mp_cmp(d,A+1,c,A+1)) {
			fail++;
			printf("wrong!\n");
		}
			printf("   a="); mp_print_hex(a,A); printf("\n");
			printf("   b="); mp_print_hex(b,A); printf("\n");
			printf(" add="); mp_print_hex(c,A+1); printf("\n");
			printf("addi="); mp_print_hex(d,A+1); printf("\n");
	}
	printf("%u passed, %u failed\n", NTRIALS - fail, fail);
#undef A
#undef NTRIALS
}

void
test_sub(void)
{
#define A 7
	mp_digit a[A], b[A], c[A];
	int i, n;

	for (i=0; i<1; i++) {
		mp_rand(a,A); mp_rand(b,A);
		printf("  a="), mp_print_dec(a,A), printf("\n");
		printf("  b="), mp_print_dec(b,A), printf("\n");
		n=mp_sub_n(a,b,A,c);
		printf("a-b=%c",n?'-':'+');
		mp_print_dec(c,A); printf("\n");

		n=mp_sub_n(b,a,A,c);
		printf("b-a=%c",n?'-':'+');
		mp_print_dec(c,A); printf("\n");
	}
#undef A
}

void
test_gcd(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define A		20
#define B		10
#define G		10
#define NTRIALS	100000
	mp_digit a[A], ad[A];
	mp_digit b[B], bd[A];
	mp_digit g[G];
	int j, g_size;

	timer_val timer = hrtimer();
	twiddle();
	for (j=0;j<NTRIALS;j++) {
		mp_rand(a,A);
		mp_rand(b,B);
		a[0] = b[0] = a[1] = b[1] = 0;
		mp_lshifti(a,A,4);
		mp_lshifti(b,B,4);
		mp_gcd(a,A,b,B,g);
		g_size=mp_rsize(g,G);
		mp_divexact(a,A,g,g_size,ad);
		mp_divexact(b,B,g,g_size,bd);
		mp_zero(g,G);
		mp_gcd(ad,A-g_size+1,bd,B-g_size+1,g);
		if (!mp_is_one(g,G))
			break;
		if ((j&1023)==0)
			twiddle();
	}
	if (j<NTRIALS) {
		debug_print('A',a,A);
		debug_print('B',b,B);
		debug_print('G',g,G);
	} else {
		timer = hrtimer() - timer;
		printf("No GCD errors in %d trials (took " TIMER_FMT ")\n",
			   NTRIALS, TIMER_VAL(timer));
	}
#undef A
#undef B
#undef G
#undef NTRIALS
}

void
test_shift(void)
{
#define A	4
	mp_digit r;
	mp_digit a[A+1];

	mp_rand(a,A);
	printf("a="), mp_print_dec(a,A), printf("\n");
	r = mp_rshifti(a,A,7);
	printf("a>>7="), mp_print_dec(a,A), printf(", r=" MP_FORMAT "\n", r);
	a[A]=mp_lshifti(a,A,31);
	printf("(a>>7)<<31="), mp_print_dec(a,A+1), printf("\n");
#undef A
}

void
test_sqrt(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define A	150
#define B	(A*2)
#define C	((B+1)/2)
#define NTRIALS	10000
	mp_digit a[A], b[B], c[C];

	timer_val sqrt_time = 0;
	for (int i=0; i<NTRIALS; i++) {
		mp_rand(a,A);			// A <- random
	//	mp_max(a,A);
		mp_sqr(a,A,b);			// B <- A^2
		/* It would require B>=(A^2+2A+1) for floor(sqrt(B)) > A. */
		mp_addi(b,B,a,A);		// B += A
		mp_addi(b,B,a,A);		// B += A
		mp_inc(b,B);			// B += 1
		timer_val timer = hrtimer();
		mp_sqrt(b,B,c);			// C = sqrt(B)
		sqrt_time += hrtimer() - timer;
		mp_dec(c,C);			// C = C - 1
		if (mp_cmp(a,A,c,C) != 0) {
			printf("a="), mp_print_dec(a,A), printf("\n");
			printf("b="), mp_print_dec(b,B), printf("\n");
			printf("c="), mp_print_dec(c,C), printf("\n");
		}
	}
	printf("Computed %u %u-digit square roots in " TIMER_FMT "\n",
		   NTRIALS, A, TIMER_VAL(sqrt_time));
#undef C
#undef B
#undef A
#undef NTRIALS
}

void
time_sqrt(void)
{
#define A	50
#define B	((A+1)/2)
	mp_digit a[A], b[B];
	timer_val total = 0;
	int i;

	for (i=0; i<10000; i++) {
		mp_max(a,A);
		timer_val start = hrtimer();
		mp_sqrt(a,A,b);
		total += hrtimer() - start;
	}
	printf("time: " TIMER_FMT "\n", TIMER_VAL(total));
#undef B
#undef A
}

void
test_mul(void)
{
#define A	300
#define C	(A*2)
#define NTRIALS	50000

	mp_digit a[A], b[A], c[C], d[C];
	unsigned fail=0;

	for (int i=0; i<NTRIALS; ++i) {
		mp_rand(a,A); mp_rand(b,A);
		_mp_mul_base(a,A,b,A,c);
		mp_mul_n(a,b,A,d);
		if (mp_cmp_n(c,d,C)) {
			fail++;
			printf("wrong\n");
			printf("a="); mp_print_hex(a,A); printf("\n");
			printf("b="); mp_print_hex(b,A); printf("\n\n");
			mp_print_hex(c,C); printf("\n\n");
			mp_print_hex(d,C); printf("\n");
			for (int j=0; j<C; ++j)
				if (c[j]!=d[j])
					printf("differ at position %u\n", j);
		}
	}
	printf("%u passed, %u failed\n", NTRIALS-fail, fail);

#undef A
#undef B
#undef C
#undef NTRIALS
}

#if 0
void
tune_mul(void)
{
#define A	200
#define B	200
#define C	(A+B)
	mp_digit a[A], b[B], c[C];
	uint64_t s, e;
	mp_size cutoff;
	unsigned i;
	extern mp_size KARATSUBA_MUL_THRESHOLD;

	for (cutoff = 8; cutoff <= 256; cutoff += 4) {
		KARATSUBA_MUL_THRESHOLD = cutoff;
		s = hrtimer();
		for (i=0; i<100; i++)
			mp_mul(a,A,b,B,c);
		e = hrtimer();
		printf("Cutoff=%3u cycles=%llu\n", cutoff, e - s);
	}
#undef C
#undef B
#undef A
}
#endif

void
time_div(void)
{
#define N	200
#define D	60
#define Q	(N-D+1)
#define R	D
	mp_digit n[N], d[D], q[Q], r[R];
	timer_val total=0;

	for (int i=0; i<10000; i++) {
		mp_rand(n,N);
		mp_rand(d,D);
		timer_val start = hrtimer();
		mp_divrem(n,N,d,D,q,r);
		total += hrtimer() - start;
	}
	printf("total: " TIMER_FMT "\n", TIMER_VAL(total));
#undef R
#undef Q
#undef D
#undef N
}

void
time_mul(void)
{
#define A	512
#define N	100000
	mp_digit a[A], b[A], c[A*2];
	unsigned i;
	timer_val total = 0;

	mp_rand(a,A);
	mp_rand(b,A);
	timer_val start = hrtimer();
	for (i=0; i<N; i++) {
		mp_mul_n(a, b, A, c);
	}
	total = hrtimer() - start;
	printf("%u digits: " TIMER_FMT "/multiply\n", A, TIMER_VAL(total)/N);
#undef A
#undef N
}

void
time_mmul(void)
{
#define A 455
#define B 137
#define N 10000
	mp_digit a[A], b[B], c[A+B];
	int i;

	mp_rand(a,A);
	mp_rand(b,B);
	for (i=0; i<N; i++)
		mp_mul(a,A,b,B,c);
#undef N
#undef B
#undef A
}

void
time_add(void)
{
#define A	60
#define	N	1000000
	unsigned i;
	mp_digit a[A], b[A], c[A];

	mp_rand(a,A);
	mp_rand(b,A);
	mp_rand(c,A); /* shaddup gcc */

#if 0
	s = hrtimer(); mp_add_n(a,b,A,c); e = hrtimer();
	printf("add  = %llu cycles\n", e - s);
	s = hrtimer(); mp_addi_n(a,b,A); e = hrtimer();
	printf("addi = %llu cycles\n", e - s);
#else
	timer_val timer = hrtimer();
	for (i=0; i<N; i++)
	//	mp_add_n(a,b,A,c);
		mp_addi_n(a,b,A);
	//	mp_subi_n(a,b,A);
	timer = hrtimer() - timer;
	printf("addi=" TIMER_FMT "\n", TIMER_VAL(timer));
#endif

#undef A
#undef N
}

void
test_dmod(void)
{
#if MP_DIGIT_SIZE >= 4
#define A	100
#define B	2398283U
	mp_digit a[A], r;

	mp_rand(a,A);
	mp_print_dec(a,A); printf("\n");
	timer_val timer = hrtimer();
	r = mp_dmod(a,A,B);
	timer = hrtimer() - timer;
	printf("%u %% " MP_FORMAT "\n", B, r);
	printf("time: " TIMER_FMT "\n", TIMER_VAL(timer));
#undef A
#undef B
#endif
}

void
run_gcd_test(void)
{
#define A		20
#define B		20
#define G		(A<B?A:B)
#define TRIALS	10000

	mp_digit a[A], b[B], gcd[G];
	int i, cp = 0;
	double r;

	for (i = 0; i < TRIALS; i++) {
		mp_rand(a,A);
		mp_rand(b,B);
		mp_gcd(a,A,b,B,gcd);
		if (mp_rsize(gcd,G) == 1 && gcd[0] == 1)
			cp++;
	}

	r = cp / (double)TRIALS;
	printf("%d coprimes from %d trials (ratio=%.10f)\n", cp, TRIALS, r);
#undef TRIALS
#undef A
#undef B
#undef G
}

void
twiddle(void)
{
	static int t=-1;

	printf("%c\b", "|/-\\"[(++t)&3]);
	fflush(stdout);
}

void
test_dmuli(void)
{
#define A 7
	mp_digit a[A + 1], b;

	mp_rand(a,A);
	mp_rand(&b,1);
	/* To test power of 2 code uncomment below */
//	b=1<<(b&31);
	printf("a="), mp_print_dec(a,A), printf("\n");
	a[A] = mp_dmuli(a,A,b);
	printf("a * " MP_FORMAT " = ", b), mp_print_dec(a, A + 1), printf("\n");
#undef A
}

void
time_dmul_add(void)
{
#define A	100
	mp_digit a[A], b[A], c;

	mp_rand(a,A);
	mp_rand(b,A);
	mp_rand(&c,1);
	timer_val timer = hrtimer();
	for (int i=0; i<1000; i++)
		mp_dmul_sub(b, A, c, a);
	timer = hrtimer() - timer;
	printf("time: " TIMER_FMT "\n", TIMER_VAL(timer));
#undef A
}

void
gen_fraction(void)
{
#define A	100
#define B	((A + 2) / 2)
	mp_digit a[A + 1];
	mp_digit b[B];
	mp_digit r;

#if 0
	mp_zero(a, A);
	a[A] = 1;
	mp_ddivi(a, A + 1, 5);
	mp_inc(a, A);
	putchar(a[A] + '0');
	i = mp_string_size(A, 10);
	putchar('.');
	while (i--) {
		r = mp_dmuli(a, A, 10);
		putchar(r + '0');
	}
	putchar('\n');
#else
	mp_zero(a, A);
	a[A] = 89;

	mp_sqrt(a, A + 1, b);
	putchar('\n');
	putchar(b[B - 1] + '0');
	putchar('.');
	size_t i = mp_string_size(B - 1, 10);
	while (i--) {
		r = mp_dmuli(b, B - 1, 10);
		putchar(r + '0');
	}
	putchar('\n');
#endif
#undef B
#undef A
}

void
test_invert(void)
{
	mp_digit i, n, inv;

	for (i = 0; i < 10; i++) {
		mp_rand(&n,1);
		n |= 1; /* ensure it's odd. */
		printf("n=" MP_FORMAT "\n", n);
		inv = mp_digit_invert(n);
		printf("inv=" MP_FORMAT "\n", inv);
		printf("n*inv=" MP_FORMAT "\n", (mp_digit)(n * inv));
	}
}

void
pr_hex(char c, const mp_digit *u, mp_size size)
{
	putchar(c);
	putchar('=');
	while (size--)
		printf(MP_HEX_FORMAT "%c", u[size], size ? ' ' : '\n');
}

void
test_divexact(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define A 60
#define B (A/2)
#define C (A+B)
#define D (C-A+1)
#define N 10000
	mp_digit a[A], b[B], c[C], d[D];
	int i;

	for (i=0; i<N; i++) {
		mp_rand(a,A);
		a[A-1]+=!a[A-1];
		mp_rand(b,B);
		mp_mul(a,A,b,B,c);
		mp_divexact(c,C,a,A,d);
		mp_div(c,C,a,A,d);
		if (d[D-1] || mp_cmp_n(d,b,B) != 0) {
			pr_hex('a',a,A);
			pr_hex('b',b,B);
			pr_hex('c',c,C);
			pr_hex('d',d,D);
		}
	}
#undef N
#undef D
#undef C
#undef B
#undef A
}

void
time_divexact(void)
{
#define A	30
#define B	15
#define C	(A+B)
#define D	(C-B+1)
#define N	100000
	mp_digit a[A],b[B],c[C],d[D];
	timer_val div_time = 0, divexact_time = 0;

	for (int i=0; i<N; i++) {
		mp_rand(a,A);
		mp_rand(b,B);
		mp_mul(a,A,b,B,c);

		timer_val start = hrtimer();
		mp_divexact(c,C,b,B,d);
		divexact_time += hrtimer() - start;

		start = hrtimer();
		mp_div(c,C,b,B,d);
		div_time += hrtimer() - start;
	}
	printf("     div=" TIMER_FMT "\n", TIMER_VAL(div_time));
	printf("divexact=" TIMER_FMT "\n", TIMER_VAL(divexact_time));
#undef A
#undef B
#undef C
#undef D
#undef N
}

void
gcdext_int(int a, int b, int *u, int *v, int *g)
{
	if (a < b)
		SWAP(a, b, int);

	*u = 1;
	*g = a;

	if (b == 0) {
		*v = 0;
		return;
	}

	int v1 = 0;
	int v3 = b;
	while (v3 != 0) {
		int q = *g / v3;
		int t3 = *g % v3;
		int t1 = *u - q * v1;
		*u = v1;
		*g = v3;
		v1 = t1;
		v3 = t3;
	}
	*v = (*g - a * *u) / b;
}

void
test_gcdext(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

	mpi_t a, b, g, u, v, d;

	mpi_init(a);
	mpi_init(b);
	mpi_init(u);
	mpi_init(v);
	mpi_init(d);
	mpi_init(g);

	for (int i=0; i<100000; i++) {
		mpi_rand(a, 100);
		mpi_rand(b, 100);
		mpi_gcd(a, b, g);
		/*
		printf("A="), mpi_print_dec(a), printf(" B="), mpi_print_dec(b);
		printf(" G="), mpi_print_dec(g), printf("\n");
		*/
		mpi_gcdext(a, b, u, v, d);
		if (!mpi_cmp_eq(g, d)) {
			printf("[%d] warning: gcd/gcdext not equal.\n", i);
			printf("A="), mpi_print_dec(a), printf("\n");
			printf("B="), mpi_print_dec(b), printf("\n");
			printf("G="), mpi_print_dec(g), printf("\n");
			printf("U="), mpi_print_dec(u), printf("\n");
			printf("V="), mpi_print_dec(v), printf("\n");
			printf("D="), mpi_print_dec(d), printf("\n");
		}
		if (a->size == 1) {
			int iu, iv, ig;
			gcdext_int(a->digits[0], b->digits[0], &iu, &iv, &ig);
			printf("IU=%d IV=%d IG=%d\n", iu, iv, ig);
			if (!mpi_cmp_s32(u, iu))
				printf("IU does not match\n");
			if (!mpi_cmp_s32(v, iv))
				printf("IV does not match\n");
			if (!mpi_cmp_s32(g, ig))
				printf("IG does not match\n");
		}
		mpi_mul(a, u, u);	/* u=a*u */
		mpi_mul(b, v, v);	/* v=b*v */
		mpi_add(u, v, g);	/* g=a*u+b*v */
		if (!mpi_cmp_eq(g, d)) {
			printf("[%d] warning: gcd/a*u+b*v not equal.\n", i);
			printf("A="), mpi_print_dec(a), printf("\n");
			printf("B="), mpi_print_dec(b), printf("\n");
			printf("G="), mpi_print_dec(g), printf("\n");
			printf("U="), mpi_print_dec(u), printf("\n");
			printf("V="), mpi_print_dec(v), printf("\n");
			printf("D="), mpi_print_dec(d), printf("\n");
		}
	}

	mpi_free(a);
	mpi_free(b);
	mpi_free(g);
	mpi_free(u);
	mpi_free(v);
	mpi_free(d);
}

void
test_modinv(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

	const int iters=10000;
	mpi_t b, m, inv, p;

	mpi_init(b);
	mpi_init(m);
	mpi_init(inv);
	mpi_init(p);

	for (int i=0; i<iters; i++) {
		do {
			mpi_rand(m, 40);
			do {
				mpi_rand(b, 39);
			} while (!(mpi_cmp(b, m) < 0));
		} while (!mpi_modinv(m,b,inv));

		mpi_mul(b, inv, p);
		mpi_mod(p, m, p);
		if (!mpi_is_one(p))
			printf("trial %d failed\n", i);
	}

	mpi_free(p);
	mpi_free(inv);
	mpi_free(m);
	mpi_free(b);
}

void
test_composite(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define A (1024/(MP_DIGIT_SIZE*CHAR_BIT))
#define ROUNDS 8
	mp_digit a[A];

	mp_rand(a,A);
	a[A-1]|=1UL<<31;
	a[0]|=1;
	printf("Searching for prime... ");
	int trials = 0, sieved = 0;
	timer_val sieve_time = 0, composite_time = 0;
	for (;;) {
		if (mp_daddi(a,A,2)) {
			mp_rand(a,A);
			a[A-1]|=1UL<<31;
			a[0]|=1;
		}
		if ((trials&0x7)==0)
			twiddle();
		trials++;

		timer_val start = hrtimer();
		int r = mp_sieve(a,A,400);
		sieve_time += hrtimer() - start;
		if (r) {
			sieved++;
			continue;
		}
		start = hrtimer();
		r = mp_composite(a,A,ROUNDS);
		composite_time += hrtimer() - start;

		if (!r)
			break;
	}
	printf("\n");
	mp_print_dec(a,A);
	printf(" (%u bits) is prime with high probability\n",
		   mp_significant_bits(a,A));
	printf("tested %d numbers (%d [%.03f%%] rejected by sieve).\n",
		   trials, sieved, sieved / (float)trials * 100.0);
	printf("          sieving took " TIMER_FMT "\n", TIMER_VAL(sieve_time));
	printf("primality testing took " TIMER_FMT "\n", TIMER_VAL(composite_time));
#undef A
#undef ROUNDS
}

void
time_copy(void)
{
#define A 237
	mp_digit a[A], b[A];

	mp_rand(a,A);

	timer_val timer = hrtimer();
	mp_copy(a,A,b);
	timer = hrtimer() - timer;
	if (mp_cmp_n(a,b,A))
		abort();
	printf("copy: " TIMER_FMT "\n", TIMER_VAL(timer));

	timer = hrtimer();
	mp_copy(a,A,b);
	timer = hrtimer() - timer;
	if (mp_cmp_n(a,b,A))
		abort();
	printf("copy: " TIMER_FMT "\n", TIMER_VAL(timer));

	timer = hrtimer();
	mp_copy(a,A,b);
	timer = hrtimer() - timer;
	if (mp_cmp_n(a,b,A))
		abort();
	printf("copy: " TIMER_FMT "\n", TIMER_VAL(timer));
#undef A
}

void
test_mul_mod_powb(void)
{
#define A 5
#define B 5
#define C 8
#define D (A+B)
	mp_digit a[A], b[B], c[C], d[D];
	int i;

	mp_rand(a,A);
	mp_rand(b,B);
	printf("A="), mp_print_hex(a,A), printf("\n");
	printf("B="), mp_print_hex(b,B), printf("\n");
	mp_mul_mod_powb(a,A,b,B,c,C);
	mp_mul(a,A,b,B,d);

	for (i=0; i<C; i++)
		if (c[i] != d[i])
			printf("wrong at %u\n", i);
	printf("C="), mp_print_hex(c,C), printf("\n");
	printf("D="), mp_print_hex(d,C), printf("\n");
#undef D
#undef C
#undef B
#undef A
}

void
test_mexp(void)
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define A 32
#define P 20
#define M 16
#define NTRIALS 1000
	mp_digit a[A], p[P], m[M], m0[M], m1[M];
	timer_val modexp_time = 0, modexp_pow2_time = 0, mexp_time = 0;

	for (int i=0; i<NTRIALS; i++) {
		mp_rand(a,A);
		mp_rand(p,P);
		mp_rand(m,M);
		m[0]|=1;

		timer_val start = hrtimer();
		mp_modexp(a,A,p,P,m,M,m0);
		modexp_time += hrtimer() - start;

		start = hrtimer();
		mp_modexp_pow2(a,A,p,P,m,M,m1);
		modexp_pow2_time += hrtimer() - start;

		if (mp_cmp_n(m0,m1,M)) {
			printf("modexp != modexp_pow2!\n");
			printf(" a="), mp_print_hex(a,A), printf("\n");
			printf(" p="), mp_print_hex(p,P), printf("\n");
			printf(" m="), mp_print_hex(m,M), printf("\n");
			printf("modexp="), mp_print_hex(m0,M), printf("\n");
			printf("modexp_pow2="), mp_print_hex(m1,M), printf("\n");
		}

		start = hrtimer();
		mp_mexp(a,A,p,P,m,M,m1);
		mexp_time += hrtimer() - start;

		if (mp_cmp_n(m0,m1,M)) {
			printf("modexp != mexp!\n");
			printf(" a="), mp_print_hex(a,A), printf("\n");
			printf(" p="), mp_print_hex(p,P), printf("\n");
			printf(" m="), mp_print_hex(m,M), printf("\n");
			printf("modexp="), mp_print_hex(m0,M), printf("\n");
			printf("mexp="), mp_print_hex(m1,M), printf("\n");
		}
	}
	printf(" modexp=" TIMER_FMT "\n", TIMER_VAL(modexp_time));
	printf("modexp2=" TIMER_FMT "\n", TIMER_VAL(modexp_pow2_time));
	printf("   mexp=" TIMER_FMT "\n", TIMER_VAL(mexp_time));
#undef A
#undef P
#undef M
#undef NTRIALS
}

void
test_lehmer(void)
{
#define A 5
#define B 5
#define G MIN(A,B)
	mp_digit a[A], b[B], g1[G], g2[G];
	extern void mp_lehmer(const mp_digit *, mp_size,
						  const mp_digit *, mp_size, mp_digit *);

	mp_rand(a,A);
	mp_rand(b,B);
	mp_gcd(a,A,b,B,g1);
	mp_lehmer(a,A,b,B,g2);

	if (mp_cmp_n(g1,g2,G)) {
		printf(" a="), mp_print_hex(a,A), printf("\n");
		printf(" b="), mp_print_hex(b,B), printf("\n");
		printf("g1="), mp_print_hex(g1,G), printf("\n");
		printf("g2="), mp_print_hex(g2,G), printf("\n");
	}
#undef A
#undef B
#undef G
}

void
test_from_str(void)
{
	mp_digit *p, tf;
	mp_size psize;
	char buf[1024] = { 0 };

	while (fgets(buf, sizeof(buf), stdin)) {
		p = mp_from_str_dec(buf, &psize);
		mp_print_dec(p, psize);
		if ((tf = mp_sieve(p, psize, 1000)) != 0)
			printf(" has trivial factor " MP_FORMAT ".\n", tf);
		else if (mp_composite(p, psize, 10))
			printf(" is composite.\n");
		else
			printf(" is probably prime.\n");
		mp_free(p);
	}
}

void
time_invert(void)
{
	timer_val timer = hrtimer();
	for (unsigned i = 0; i < 1000; i++) {
		mp_digit p;
		mp_rand(&p, 1);
		mp_digit q = mp_digit_invert(p | 1);
		(void)q;
	}
	timer = hrtimer() - timer;
	printf("time: " TIMER_FMT "\n", TIMER_VAL(timer));
}

uint32_t r() { return (rand()<<16)|rand(); }

void
test_mpq()
{
	mpq_t a, b, c, d, e;

	srand((unsigned)time(0));
	mpq_init_s32_s32(a, r(), r());
	mpq_init_s32_s32(b, r(), r());
	mpq_init_s32_s32(c, r(), r());
	mpq_init_s32_s32(d, r(), r());
	mpq_init(e);

	printf("a="), mpq_print_dec(a), printf("\n");
	printf("b="), mpq_print_dec(b), printf("\n");
	printf("c="), mpq_print_dec(c), printf("\n");
	printf("d="), mpq_print_dec(d), printf("\n");

	mpq_add(a, b, e);
	mpq_add(e, c, e);
	mpq_add(e, d, e);

	printf("e="), mpq_print_dec(e), printf("\n");

	mpq_free(a);
	mpq_free(b);
	mpq_free(c);
	mpq_free(d);
	mpq_free(e);
}

mpq_t **
matrix_n_by_n(int n)
{
	int i,j;
	mpq_t **m;

	m = MALLOC(sizeof(*m) * n);
	for (i = 0; i < n; i++) {
		m[i] = MALLOC(sizeof(**m) * n);
		for (j = 0; j < n; j++)
			mpq_init_u32(m[i][j], i == j);
	}
	return m;
}

/* invert_matrix -- invert a matrix using rational arithmetic */
int
invert_matrix(int n, mpq_t **mtx, mpq_t **inv)
{
	int i, j, k;
	mpq_t t,m;

	if (n == 1) {
		if (mpq_is_zero(mtx[0][0]))
			return -1;
		mpq_set_mpq(inv[0][0], mtx[0][0]);
		mpq_invert(inv[0][0]);
		return 0;
	}

	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			mpq_set_u32(inv[i][j], i == j);

	mpq_init(t);
	mpq_init(m);

	for (i = 0; i < n; i++) {
		/* Scan for row with a non-zero i'th component. */
		for (j = i; j < n; j++)
			if (!mpq_is_zero(mtx[j][i]))
				break;
		if (j == n) { /* singular */
			mpq_free(t);
			mpq_free(m);
			return -1;
		}
		if (j != i) { /* swap row if needed */
			SWAP(mtx[i], mtx[j], mpq_t *);
			SWAP(inv[i], inv[j], mpq_t *);
		}
		for (j = i + 1; j < n; j++) { /* elim i'th var from rest of eq's */
			mpq_div(mtx[j][i], mtx[i][i], m);
			for (k = i; k < n; k++) {
				mpq_mul(mtx[i][k], m, t);
				mpq_sub(mtx[j][k], t, mtx[j][k]);
			}
			for (k = 0; k < n; k++) {
				mpq_mul(inv[i][k], m, t);
				mpq_sub(inv[j][k], t, inv[j][k]);
			}
		}
	}

	/* now matrix should be upper triangular. back-substitute */
	for (i = n-1; i >= 0; i--) {
		/* scale row so first component is 1. */
		for (k = 0; k < n; k++)
			mpq_div(inv[i][k], mtx[i][i], inv[i][k]);
		mpq_set_u32(mtx[i][i], 1);
		/* now back-substitute row */
		for (j = i-1; j >= 0; j--) {
			for (k = 0; k < n; k++) {
				mpq_mul(mtx[j][i], inv[i][k], m);
				mpq_sub(inv[j][k], m, inv[j][k]);
			}
			mpq_set_u32(mtx[j][i], 0);
		}
	}

	mpq_free(t);
	mpq_free(m);
	return 0;
}

void
test_invert_matrix(void)
{
	int j,k;
	int n=4;

	mpq_t **m;
	mpq_t **i;

	m = matrix_n_by_n(n);
	i = matrix_n_by_n(n);

	mpq_set_u32(m[0][0], 0);
	mpq_set_u32(m[0][1], 0);
	mpq_set_u32(m[0][2], 0);
	mpq_set_u32(m[0][3], 1);

	mpq_set_u32(m[1][0], 1);
	mpq_set_u32(m[1][1], 1);
	mpq_set_u32(m[1][2], 1);
	mpq_set_u32(m[1][3], 1);

	mpq_set_u32(m[2][0], 7);
	mpq_set_u32(m[2][1], 0);
	mpq_set_u32(m[2][2], 1);
	mpq_set_u32(m[2][3], 0);

	mpq_set_u32(m[3][0], 3);
	mpq_set_u32(m[3][1], 2);
	mpq_set_u32(m[3][2], 1);
	mpq_set_u32(m[3][3], 0);

	printf("M:\n");
	for (j=0; j<n; j++) {
		for (k=0; k<n; k++) {
		//	printf("m[%d,%d]=",j,k);
		//	mpq_print_dec(m[j][k]);
			printf("% .03f", mpq_get_f(m[j][k]));
			printf("%c", (k+1==n)?'\n':' ');
		}
	}
	printf("\n");

	if (invert_matrix(n, m, i)) {
		printf("singular\n");
		return;
	}

	for (j=0; j<n; j++) {
		for (k=0; k<n; k++) {
			if (mpq_cmp_u32(m[j][k], j == k)) {
				printf("inversion failed: m[%u,%u]=%.03f", j, k, mpq_get_f(m[j][k]));
				return;
			}
		}
	}

	printf("M^-1:\n");
	for (j=0; j<n; j++) {
		for (k=0; k<n; k++) {
		//	printf("i[%d,%d]=",j,k);
		//	mpq_print_dec(i[j][k]);
			printf("% .03f", mpq_get_f(i[j][k]));
			printf("%c", (k+1==n)?'\n':' ');
		}
	}
}

void
test_crt(void)
{
	mpi_t a_1, m_1;
	mpi_t a_2, m_2;
	mpi_t a_3, m_3;
	mpi_t x, t;
	mpi_crt_ctx ctx;

	mpi_init_u32(a_1, 1); mpi_init_u32(m_1,  7);
	mpi_init_u32(a_2, 6); mpi_init_u32(m_2, 11);
	mpi_init_u32(a_3, 5); mpi_init_u32(m_3, 13);
	mpi_init(x);
	mpi_init(t);

	mpi_crt_init(&ctx);
	do {
		if (mpi_crt_step(&ctx, a_1, m_1)) { printf("failed at 1\n"); break; }
		if (mpi_crt_step(&ctx, a_2, m_2)) { printf("failed at 2\n"); break; }
		if (mpi_crt_step(&ctx, a_3, m_3)) { printf("failed at 3\n"); break; }
		if (mpi_crt_finish(&ctx, x)) { printf("failed at finish\n"); break; }
		printf("OK, x="), mpi_print_dec(x), printf("\n");
		mpi_mod(x, m_1, t);
		if (!mpi_cmp_eq(t, a_1)) {
			printf("noteq at 1\n");
			printf("  T: "), mpi_print_dec(t), printf("\n");
			printf("M_1: "), mpi_print_dec(m_1), printf("\n");
			printf("A_1: "), mpi_print_dec(a_1), printf("\n");
			break;
		}
		mpi_mod(x, m_2, t);
		if (!mpi_cmp_eq(t, a_2)) {
			printf("noteq at 2\n");
			printf("  T: "), mpi_print_dec(t), printf("\n");
			printf("M_2: "), mpi_print_dec(m_2), printf("\n");
			printf("A_2: "), mpi_print_dec(a_2), printf("\n");
			break;
		}
		mpi_mod(x, m_3, t);
		if (!mpi_cmp_eq(t, a_3)) {
			printf("noteq at 3\n");
			printf("  T: "), mpi_print_dec(t), printf("\n");
			printf("M_3: "), mpi_print_dec(m_3), printf("\n");
			printf("A_3: "), mpi_print_dec(a_3), printf("\n");
			break;
		}
		printf("checked ok!\n");
	} while (0);
	mpi_free(a_1); mpi_free(m_1);
	mpi_free(a_2); mpi_free(m_2);
	mpi_free(a_3); mpi_free(m_3);
	mpi_free(x);
	mpi_free(t);
}

void
test_set_f(void)
{
	mpq_t t;
#define N	100000

	mpq_init(t);
	srand((unsigned)time(0));

	timer_val get_time = 0, set_time = 0;
	for (int j = 0; j < N; j++) {
#if 1
		double d, d2;
		d = rand() / (double)RAND_MAX;
	//	d *= RAND_MAX; d += rand();
	//	d *= RAND_MAX; d += rand();
	//	d *= RAND_MAX; d += rand();
	//	d *= RAND_MAX; d += rand();

		timer_val start = hrtimer();
		mpq_set_d(t, d);
		set_time += hrtimer() - start;

		start = hrtimer();
		d2 = mpq_get_d(t);
		get_time += hrtimer() - start;
		if (d!=d2)
			printf("d=%.*f d2=%.*f diff=%.*f\n",
				   DBL_DIG+3, d, DBL_DIG+3, d2, DBL_DIG+3, (d>d2)?d2-d:d-d2);

		d = 1.0 / d;
		mpq_set_d(t, d);
		d2 = mpq_get_d(t);
		if (d!=d2)
			printf("d=%.*f d2=%.*f diff=%.*f\n",
				   DBL_DIG+3, d, DBL_DIG+3, d2, DBL_DIG+3, (d>d2)?d2-d:d-d2);

#else
		float f, f2;
		f = rand() / (float)RAND_MAX;
	//	f *= RAND_MAX; f += rand();
	//	f *= RAND_MAX; f += rand();
	//	f *= RAND_MAX; f += rand();

		timer_val start = hrtimer();
		mpq_set_f(t, f);
		set_time += hrtimer() - start;

		start = hrtimer();
		f2 = mpq_get_f(t);
		get_time += hrtimer() - start;

		if (f!=f2)
			printf("f1=%.*f f2=%.*f\n|f/f2|=%.*g\n",
				   FLT_DIG+2, f, FLT_DIG+2, f2, FLT_DIG+2, (f/f2));

		f = 1.0f / f;
		mpq_set_f(t, f);
		f2 = mpq_get_f(t);
		if (f!=f2)
			printf("f=%.*g f2=%.*g |f/f2|=%.*g\n",
				   FLT_DIG+2, f, FLT_DIG+2, f2, FLT_DIG+2, (f/f2));
#endif
	}
	printf("avg time for mpq_set: " TIMER_FMT "\n", TIMER_VAL(set_time) / N);
	printf("avg time for mpq_get: " TIMER_FMT "\n", TIMER_VAL(get_time) / N);
#undef N

	mpq_free(t);
}

void
test_mpq_acc(void)
{
	mpq_t t,t2,t3;
	volatile float f,f2,f3;

	f = 1000.43f;
	f2 = f - 1000.0f;
	f3 = f2 + 10000000.0f;
	printf("f3=%.*f\n", FLT_DIG+1, f3);

	mpq_init_d(t, 1000.43);
	mpq_init_d(t2, 1000.0);
	mpq_sub(t, t2, t2);
	mpq_init_d(t3, 10000000.0);
	mpq_add(t3, t2, t3);
	printf("t3=%.*f\n", FLT_DIG+1, mpq_get_d(t3));

	mpq_free(t);
	mpq_free(t2);
	mpq_free(t3);
}

void
time_gcd()
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define A 30
#define B 30
#define C MIN(A,B)
#define NTRIALS	1000
	mp_digit a[A], b[B], c[C];

	for (mp_size aa=A; aa<=A; aa+=3) {
		for (mp_size bb=B; bb<=aa; bb+=3) {
			timer_val total_time = 0;
			for (int i=0; i<NTRIALS; i++) {
				mp_rand(a, aa);
				mp_rand(b, bb);

				timer_val start = hrtimer();
				mp_gcd(a, aa, b, bb, c);
				total_time += hrtimer() - start;
			}
			printf("Computed %u %u/%u-bit GCDs in " TIMER_FMT "\n",
				   NTRIALS, aa*MP_DIGIT_BITS, bb*MP_DIGIT_BITS,
				   TIMER_VAL(total_time));
		}
	}

#undef A
#undef B
#undef C
#undef NTRIALS
}

void
time_binomial()
{
	printf("--> %s\n", __PRETTY_FUNCTION__);

#define N 500
	mpi_t binomial;
	mpi_init(binomial);

	timer_val total_time = 0;
	for (int n = 1; n <= N; ++n) {
		for (int k = 0; k <= n; ++k) {
			timer_val timer = hrtimer();
			mpi_binomial(n, k, binomial);
			total_time += hrtimer() - timer;
		}
	}
	printf("%u Binomial time: " TIMER_FMT "\n", N, TIMER_VAL(total_time));
	mpi_free(binomial);
#undef N
}

#if defined(__APPLE__)

#include <mach/mach_time.h>

uint64_t hrtimer() {
	return mach_absolute_time();
}

#else

#include <sys/types.h>
#include <sys/time.h>

double hrtimer() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec * 1e-6;
}

#endif
