/*
 * mpq.c
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "weecrypt_memory.h"
#include "mpq.h"

void
mpq_normalize_nogcd(mpq *q)
{
	ASSERT(!mpi_is_zero(q->den));

	/* We want to normalize the fraction N/D so that:
	 * (1) if N=0 then D=1
	 * (2) D never has sign (actually always positive since we cant have D=0)
	 * (3) gcd(N,D)=1 */

	if (mpi_is_zero(q->num)) {
		mpi_one(q->den);
		return;
	}

	/* Make denominator positive. */
	q->num->sign ^= q->den->sign;
	q->den->sign = 0;
}

void
mpq_normalize(mpq *q)
{
	mp_digit *tmp;
	mp_size tmp_size, tz;

	ASSERT(!mpi_is_zero(q->den));

	/* We want to normalize the fraction N/D so that:
	 * (1) if N=0 then D=1
	 * (2) D never has sign (actually always positive since we cant have D=0)
	 * (3) gcd(N,D)=1 */

	if (mpi_is_zero(q->num)) {
		mpi_one(q->den);
		return;
	}

	/* Make denominator positive. */
	q->num->sign ^= q->den->sign;
	q->den->sign = 0;

	if (mpi_is_one(q->den))
		return;

	/* Adjust so that GCD(N,D)=1, doing as little work as possible :-) */

	/* Count common trailing zero digits. */
	for (tz = 0; (q->num->digits[tz] | q->den->digits[tz]) == 0; tz++)
		/* void */;
	if (tz != 0) {
		/* (num->size - tz) and (den->size - tz) must be > 0,
		 * because tz < tmp_size and tmp_size <= min(num->size, den->size) */
		q->num->size -= tz;
		mp_copy(q->num->digits + tz, q->num->size, q->num->digits);
		q->den->size -= tz;
		mp_copy(q->den->digits + tz, q->den->size, q->den->digits);
	}

	tmp_size = MIN(q->num->size, q->den->size);
	ASSERT(tmp_size != 0);
	MP_TMP_ALLOC(tmp, tmp_size);
	mp_gcd(q->num->digits, q->num->size,
		   q->den->digits, q->den->size, tmp);
	tmp_size = mp_rsize(tmp, tmp_size);
	ASSERT(tmp_size != 0);

	if (tmp_size == 1) {
		if (tmp[0] != 1) {
			mp_digit r;

			r = tmp[0];
			if ((r & (r - 1)) == 0) {
				mp_digit lg = mp_digit_log2(r);

				r = mp_rshifti(q->num->digits, q->num->size, lg);
				ASSERT(r == 0);
				q->num->size -= (q->num->digits[q->num->size - 1] == 0);

				r = mp_rshifti(q->den->digits, q->den->size, lg);
				ASSERT(r == 0);
				q->den->size -= (q->den->digits[q->den->size - 1] == 0);
			} else {
				r = mp_ddivi(q->num->digits, q->num->size, tmp[0]);
				ASSERT(r == 0);
				q->num->size -= (q->num->digits[q->num->size - 1] == 0);

				r = mp_ddivi(q->den->digits, q->den->size, tmp[0]);
				ASSERT(r == 0);
				q->den->size -= (q->den->digits[q->den->size - 1] == 0);
			}
			ASSERT(q->num->size != 0);
			ASSERT(q->den->size != 0);
		}
	} else {
		mp_digit *tmp2 = NULL;
		mp_size tmp2_size = 0;

		if (mp_cmp_eq(q->num->digits, q->num->size, tmp, tmp_size)) {
			q->num->size = 1;
			q->num->digits[0] = 1;
		} else {
			tmp2_size = MAX(q->num->size, q->den->size) - tmp_size + 1;
			MP_TMP_ALLOC(tmp2, tmp2_size);

			mp_divexact(q->num->digits, q->num->size, tmp, tmp_size, tmp2);
			q->num->size = mp_rsize(tmp2, q->num->size - tmp_size + 1);
			mp_copy(tmp2, q->num->size, q->num->digits);
		}

		if (mp_cmp_eq(q->den->digits, q->den->size, tmp, tmp_size)) {
			q->den->size = 1;
			q->den->digits[0] = 1;
		} else {
			if (tmp2 == NULL) {
				tmp2_size = q->den->size - tmp_size + 1;
				MP_TMP_ALLOC(tmp2, tmp2_size);
			}

			mp_divexact(q->den->digits, q->den->size, tmp, tmp_size, tmp2);
			q->den->size = mp_rsize(tmp2, q->den->size - tmp_size + 1);
			mp_copy(tmp2, q->den->size, q->den->digits);
		}

		if (tmp2)
			MP_TMP_FREE(tmp2);
	}
	MP_TMP_FREE(tmp);
}

mpq *
mpq_new(void)
{
	mpq *p;

	p = MALLOC(sizeof(*p));
	mpi_init(p->num);
	mpi_init_ui(p->den, 1);
	return p;
}

mpq *
mpq_dup(const mpq *q)
{
	mpq *p;

	p = MALLOC(sizeof(*p));
	mpq_init_mpq(p, q);
	return p;
}

void
mpq_init(mpq *p)
{
	/* initialize to 0/1 */
	mpi_init(p->num);
	mpi_init_ui(p->den, 1);
}

void
mpq_init_mpq(mpq *p, const mpq *q)
{
	mpi_init_mpi(p->num, q->num);
	mpi_init_mpi(p->den, q->den);
}

void
mpq_init_mpi(mpq *p, const mpi *q)
{
	/* initialize to q/1 */
	mpi_init_mpi(p->num, q);
	mpi_init_ui(p->den, 1);
}

void
mpq_init_ui(mpq *p, unsigned int q)
{
	mpi_init_ui(p->num, q);
	mpi_init_ui(p->den, 1);
}

void
mpq_init_si(mpq *p, signed int q)
{
	mpi_init_si(p->num, q);
	mpi_init_ui(p->den, 1);
}

void
mpq_init_ul(mpq *p, unsigned long q)
{
	mpi_init_ul(p->num, q);
	mpi_init_ul(p->den, 1);
}

void
mpq_init_sl(mpq *p, signed long q)
{
	mpi_init_sl(p->num, q);
	mpi_init_ul(p->den, 1);
}

void
mpq_init_ui_ui(mpq *p, unsigned int n, unsigned int d)
{
	ASSERT(d != 0);

	mpi_init_ui(p->num, n);
	mpi_init_ui(p->den, d);

	mpq_normalize(p);
}

void
mpq_init_si_si(mpq *p, signed int n, signed int d)
{
	ASSERT(d != 0);

	mpi_init_si(p->num, n);
	mpi_init_si(p->den, d);

	mpq_normalize(p);
}

void
mpq_init_ul_ul(mpq *p, unsigned long n, unsigned long d)
{
	ASSERT(d != 0);

	mpi_init_ul(p->num, n);
	mpi_init_ul(p->den, d);

	mpq_normalize(p);
}

void
mpq_init_sl_sl(mpq *p, signed long n, signed long d)
{
	ASSERT(d != 0);

	mpi_init_sl(p->num, n);
	mpi_init_sl(p->den, d);

	mpq_normalize(p);
}

void
mpq_init_f(mpq *p, float f)
{
	mpq_init(p);
	mpq_set_f(p, f);
}

void
mpq_init_d(mpq *p, double d)
{
	mpq_init(p);
	mpq_set_d(p, d);
}

void
mpq_set_f(mpq *p, float f)
{
	int sign, exp;
	mp32_t ff;

	if (f == 0.0) {
		mpq_zero(p);
		return;
	}

	/* sign = f_31
	 *  exp = f_23...30, excess 127
	 * frac = f_0...22 + 2^23 */
	ff = *(mp32_t *)&f;
	sign = ff >> 31;
	exp = (ff >> 23) & 0xff;
	ff |= (1U << 23);
	ff &= (1U << 24) - 1;

	mpi_set_ul(p->num, ff);

	p->num->sign = sign;
	mpi_set_ui(p->den, 1);
	if ((exp -= 150) != 0) { /* 127+23 */
		if (exp < 0) {
			mpi_lshift(p->den, -exp, p->den);
			mpq_normalize(p);
		} else {
			mpi_lshift(p->num,  exp, p->num);
		}
	}
}

float
mpq_get_f(const mpq *p)
{
	int N, M, K, exp, sd, sb;
	mp_digit *frac;
	mp_size fsize;
	mp32_t q;

	if (mpq_is_zero(p))
		return 0.0;

	if (mpi_is_one(p->den))
		return mpi_get_f(p->num);

	/* Single precision floats effectively have a 24-bit significand. Dividing
	 * an N-bit number by an M-bit number yields either an (N-M)-bit number or
	 * an (N-M+1)-bit number. Since we need to guarantee at LEAST 24 bits, we
	 * compute (U*2^K)/V for some K such that N+K-M >= 53. If we an extra
	 * bit, we just throw it away. Since this means N+K >= M+24, the minimum
	 * possible value for K = M-N+24. */
	N = mpi_sig_bits(p->num);
	M = mpi_sig_bits(p->den);
	K = M - N + 24;
	if (K > 0) {
		sd = K / MP_DIGIT_BITS;
		sb = K % MP_DIGIT_BITS;
	} else {
		sd = (-K) / MP_DIGIT_BITS;
		sb = (-K) % MP_DIGIT_BITS;
	}

	if (N-M < 24) {	/* K>0 */
		mp_digit *tmp;
		mp_size tsize;

		tsize = p->num->size + 1 + sd;
		MP_TMP_ALLOC(tmp, tsize);
		if (sd != 0)
			mp_zero(tmp, sd);
		tmp[tsize - 1] = mp_lshift(p->num->digits, p->num->size, sb, tmp + sd);
		tsize -= (tmp[tsize - 1] == 0);
		/* Now we have U*2^(M-N+24). Divide it by V */
		fsize = tsize - p->den->size + 1;
		MP_TMP_ALLOC(frac, fsize);
		mp_div(tmp, tsize, p->den->digits, p->den->size, frac);
		MP_TMP_FREE(tmp);
	} else { /* N-M>=24, K<=0 */
		fsize = p->num->size - sd - p->den->size + 1;
		MP_TMP_ALLOC(frac, fsize);
		mp_div(p->num->digits - sd, p->num->size + sd,
			   p->den->digits, p->den->size, frac);
		if (sb != 0)
			mp_rshifti(frac, fsize, sb);
	}
	fsize = mp_rsize(frac, fsize);
	exp = mp_significant_bits(frac, fsize) - 24;
	if (exp) {
		ASSERT(exp == 1);
		mp_rshifti(frac, fsize, 1); /* XXX throws away a bit, round? */
		fsize -= (frac[fsize-1] == 0);
	}
	ASSERT(mp_significant_bits(frac, fsize) == 24);
#if MP_DIGIT_BITS == 8
	ASSERT(fsize == 3);
	q  = frac[2]; q <<= 8;
	q |= frac[1]; q <<= 8;
	q |= frac[0];
#elif MP_DIGIT_BITS == 16
	ASSERT(fsize == 2);
	q  = frac[1]; q <<= 16;
	q |= frac[0];
#else
	ASSERT(fsize == 1);
	q = frac[0];
#endif
	MP_TMP_FREE(frac);

	ASSERT((q & ((mp32_t)1 << 23)) != 0);
	q -= (mp32_t)1 << 23;
	exp += 127 + 23 - K;	/* [bias] + [min exp to satisfy (q^-exp)<1] - K */
	if (exp < 0 || exp > 0xff) {
		fprintf(stderr, "floating-point %s in mpq_get_f()\n",
				(exp < 0) ? "underflow" : "overflow");
		return 0.0;
	}
	q |= (mp32_t)exp << 23;
	if (p->num->sign)
		q |= (mp32_t)1 << 31;
	return *(float *)&q;
}

void
mpq_set_d(mpq *p, double d)
{
	int sign, exp;
	mp_size j, digits;
	mp64_t dd;

	if (d == 0.0) {
		mpq_zero(p);
		return;
	}

	/* sign = f_63
	 *  exp = f_52..62, excess 1023
	 * frac = f_0..51 + 2^52 */
	dd = *(mp64_t *)&d;
	sign = (dd & CONST64(0x8000000000000000)) ? 1 : 0;
	exp = (int)(dd >> 52);
	exp &= ((1U << 11) - 1);
	dd &= (CONST64(1) << 53) - 1;
	digits = (53 + MP_DIGIT_BITS - 1) / MP_DIGIT_BITS;
	if (p->num->alloc < digits) {
		p->num->digits = mp_resize(p->num->digits, digits);
		p->num->alloc = digits;
	}
	p->num->size = digits;
	for (j = 0; j < digits; j++) {
		p->num->digits[j] = (mp_digit)dd;
		dd >>= MP_DIGIT_BITS;
	}
	p->num->digits[52 / MP_DIGIT_BITS] |= (mp_digit)1 << (52 % MP_DIGIT_BITS);

	p->num->sign = sign;
	mpi_set_ui(p->den, 1);
	if ((exp -= 1075) != 0) { /* 1023+52 */
		if (exp < 0) {
			mpi_lshift(p->den, -exp, p->den);
			mpq_normalize(p);
		} else {
			mpi_lshift(p->num,  exp, p->num);
		}
	}
}

double
mpq_get_d(const mpq *p)
{
	/* Ehh, this code and mpq_set_d() unfortunately uses the mp64_t data type,
	 * which we could avoid if we had a mechanism for knowing or detecting the
	 * endian-ness of the host machine... */
	int N, M, K, exp, sd, sb;
	mp_digit *frac;
	mp_size fsize;
	mp64_t q;

	if (mpq_is_zero(p))
		return 0.0;

	if (mpi_is_one(p->den))
		return mpi_get_d(p->num);

	/* Double precision floats effectively have a 53-bit significand. Dividing
	 * an N-bit number by an M-bit number yields either an (N-M)-bit number or
	 * an (N-M+1)-bit number. Since we need to guarantee at LEAST 53 bits, we
	 * compute (U*2^K)/V for some K such that N+K-M >= 53. If we get an extra
	 * bit, we just throw it away. Since this means N+K >= M+53, the minimum
	 * possible value for K = M-N+53. */
	N = mpi_sig_bits(p->num);
	M = mpi_sig_bits(p->den);
	K = M - N + 53;
	if (K > 0) {
		sd = K / MP_DIGIT_BITS;
		sb = K % MP_DIGIT_BITS;
	} else {
		sd = (-K) / MP_DIGIT_BITS;
		sb = (-K) % MP_DIGIT_BITS;
	}
	if (N-M < 53) {	/* K>0 */
		mp_digit *tmp;
		mp_size tsize;

		tsize = p->num->size + 1 + sd;
		MP_TMP_ALLOC(tmp, tsize);
		if (sd != 0)
			mp_zero(tmp, sd);
		tmp[tsize - 1] = mp_lshift(p->num->digits, p->num->size, sb, tmp + sd);
		tsize -= (tmp[tsize - 1] == 0);
		/* Now we have U*2^(M-N+53). Divide it by V */
		fsize = tsize - p->den->size + 1;
		MP_TMP_ALLOC(frac, fsize);
		mp_div(tmp, tsize, p->den->digits, p->den->size, frac);
		MP_TMP_FREE(tmp);
	} else { /* N-M>=53, K<=0 */
		fsize = p->num->size - sd - p->den->size + 1;
		MP_TMP_ALLOC(frac, fsize);
		mp_div(p->num->digits + sd, p->num->size - sd,
			   p->den->digits, p->den->size, frac);
		if (sb != 0)
			mp_rshifti(frac, fsize, sb);
	}
	fsize = mp_rsize(frac, fsize);
	exp = mp_significant_bits(frac, fsize) - 53;
	if (exp) {
		ASSERT(exp == 1);
		mp_rshifti(frac, fsize, 1);	/* XXX: throws away a bit, round? */
		fsize -= (frac[fsize-1] == 0);
	}
	ASSERT(mp_significant_bits(frac, fsize) == 53);
#if MP_DIGIT_BITS == 8
	ASSERT(fsize == 7);
	q  = frac[6]; q <<= 8;
	q |= frac[5]; q <<= 8;
	q |= frac[4]; q <<= 8;
	q |= frac[3]; q <<= 8;
	q |= frac[2]; q <<= 8;
	q |= frac[1]; q <<= 8;
	q |= frac[0];
#elif MP_DIGIT_BITS == 16
	ASSERT(fsize == 4);
	q  = frac[3]; q <<= 16;
	q |= frac[2]; q <<= 16;
	q |= frac[1]; q <<= 16;
	q |= frac[0];
#elif MP_DIGIT_BITS == 32
	ASSERT(fsize == 2);
	q  = frac[1]; q <<= 32;
	q |= frac[0];
#else
	ASSERT(fsize == 1);
	q = frac[0];
#endif
	MP_TMP_FREE(frac);

	ASSERT((q & ((mp64_t)1 << 52)) != 0);
	q -= (mp64_t)1 << 52;
	exp += 1023 + 52 - K;	/* [bias] + [min exp to satisfy (q^-exp)<1] - K */
	if (exp < 0 || exp > 0x7ff) {
		fprintf(stderr, "floating-point %s in mpq_get_d()\n",
				(exp < 0) ? "underflow" : "overflow");
		return 0.0;
	}
	q |= (mp64_t)exp << 52;
	if (p->num->sign)
		q |= (mp64_t)1 << 63;
	return *(double *)&q;
}

void
mpq_free(mpq *p)
{
	mpi_free(p->num);
	mpi_free(p->den);
}

void
mpq_set_mpq(mpq *p, const mpq *q)
{
	if (p != q) {
		mpi_set_mpi(p->num, q->num);
		mpi_set_mpi(p->den, q->den);
	}
}

void
mpq_set_ui(mpq *p, unsigned q)
{
	mpi_set_ui(p->num, q);
	mpi_set_ui(p->den, 1);
}

void
mpq_set_si(mpq *p, signed q)
{
	mpi_set_si(p->num, q);
	mpi_set_ui(p->den, 1);
}

void
mpq_set_ul(mpq *p, unsigned long q)
{
	mpi_set_ul(p->num, q);
	mpi_set_ui(p->den, 1);
}

void
mpq_set_sl(mpq *p, signed long q)
{
	mpi_set_sl(p->num, q);
	mpi_set_ui(p->den, 1);
}

void
mpq_set_ui_ui(mpq *p, unsigned int n, unsigned int d)
{
	ASSERT(d != 0);

	mpi_set_ui(p->num, n);
	mpi_set_ui(p->den, d);
	mpq_normalize(p);
}

void
mpq_set_si_si(mpq *p, signed int n, signed int d)
{
	ASSERT(d != 0);

	mpi_set_si(p->num, n);
	mpi_set_si(p->den, d);
	mpq_normalize(p);
}

void
mpq_set_ul_ul(mpq *p, unsigned long n, unsigned long d)
{
	ASSERT(d != 0);

	mpi_set_ul(p->num, n);
	mpi_set_ul(p->den, d);
	mpq_normalize(p);
}

void
mpq_set_sl_sl(mpq *p, signed long n, signed long d)
{
	ASSERT(d != 0);

	mpi_set_sl(p->num, n);
	mpi_set_sl(p->den, d);
	mpq_normalize(p);
}

void
mpq_zero(mpq *q)
{
	mpi_zero(q->num);
	mpi_one(q->den);
}

void
mpq_one(mpq *q)
{
	mpi_one(q->num);
	mpi_one(q->den);
}

#if 0
void
mpq_pinf(mpq *q)
{
	mpi_one(q->num);
	mpi_zero(q->den);
}

void
mpq_ninf(mpq *q)
{
	mpi_one(q->num);
	q->num->sign = 1;
	mpi_zero(q->den);
}

void
mpq_undef(mpq *q)
{
	mpi_zero(q->num);
	mpi_zero(q->den);
}
#endif

void
mpq_neg(mpq *q)
{
	if (q->num->size)
		q->num->sign ^= 1;
}

void
mpq_abs(mpq *q)
{
	q->num->sign = 0;
}

void
mpq_swap(mpq *p, mpq *q)
{
	mpi_swap(p->num, q->num);
	mpi_swap(p->den, q->den);
}

// U   V   V'U    U'V    UV' + VU'
// - + - = ---- + ---- = ---------
// U'  V'  V'U'   U'V'     U'V'
void
mpq_add(const mpq *u, const mpq *v, mpq *w)
{
	if (mpq_is_zero(v)) {
		mpq_set_mpq(w, u);
	} else if (mpq_is_zero(u)) {
		mpq_set_mpq(w, v);
	} else {
		if (mpi_cmp_eq(u->den, v->den)) {
			mpi_add(u->num, v->num, w->num);
			mpi_set_mpi(w->den, u->den);
			mpq_normalize(w);
		} else {
#if 1
			/* Slow way */
			mpi_t t1;
			mpi_init(t1);
			mpi_mul(u->num, v->den, t1);
			mpi_mul(u->den, v->num, w->num);
			mpi_add(t1, w->num, w->num);
			mpi_free(t1);
			mpi_mul(u->den, v->den, w->den);
			mpq_normalize(w);
#else
			/* Following Knuth 4.5.1, pp330-331
			 *
			 * Let U = u/u' and V = v/v'
			 * Let d_1 = gcd(u', v') */
			mp_digit *d;
			mp_size d_size;
			d_size = MIN(u->den->size, v->den->size);
			MP_TMP_ALLOC(d, d_size);
			mp_gcd(u->, u->den->size,
				   v->, v->den->size, d);
			d_size = mp_rsize(d, d_size);
			if (d_size == 1 && d[0] == 1) {
				/* If d_1 = 1 (~61% of the time), then W = (uv' + u'v)/(u'v')
				 * and doesn't require normalization. */
				mpi_t t1;

				MP_TMP_FREE(d);
				mpi_init(t1);
				mpi_mul(u->num, v->den, t1);
				mpi_mul(u->den, v->num, w->num);
				mpi_add(t1, w->num, w->num);
				mpi_free(t1);
				mpi_mul(u->den, v->den, w->den);
			} else {
				/* Otherwise, let t = u(v'/d_1)+v(u'/d_1) and d_2 = gcd(t,d_1)
				 * Then the answer is (t/d_2)/((u'/d_1)(v'/d_2)) which is a
				 * normalized fraction. */
				mpi_t t1, t2, d1;

				mpi_init_mp(d1, d, d_size);

				/* t1 = u(v'/d_1) */
				mpi_init(t1);
				mpi_divexact(v->den, d1, t1);
				mpi_mul(t1, u->num, t1);

				/* t2 = v(u'/d_1) */
				mpi_init(t2);
				mpi_divexact(u->den, d1, t2);
				mpi_mul(t2, v->num, t2);

				mpi_add(t1, t2, t1);

				mpi_gcd(t1, d1, t2);

				mpi_divexact(t1, t2, w->num);
				mpi_free(t1);

				mpi_divexact(u->den, d1, w->den);

				mpi_divexact(v->den, t2, t2);
				mpi_mul(w->den, t2, w->den);
				mpi_free(t2);
			}
			mpq_normalize_nogcd(w);
#endif
		}
	}
}

void
mpq_sub(const mpq *u, const mpq *v, mpq *w)
{
	if (mpq_is_zero(v)) {
		mpq_set_mpq(w, u);
	} else if (mpq_is_zero(u)) {
		mpq_set_mpq(w, v);
		mpq_neg(w);
	} else {
		if (mpi_cmp_eq(u->den, v->den)) {
			mpi_sub(u->num, v->num, w->num);
			mpi_set_mpi(w->den, u->den);
			mpq_normalize(w);
		} else {
#if 1
			/* Slow way */
			mpi_t t1;
			mpi_init(t1);
			mpi_mul(u->num, v->den, t1);
			mpi_mul(u->den, v->num, w->num);
			mpi_sub(t1, w->num, w->num);
			mpi_free(t1);
			mpi_mul(u->den, v->den, w->den);
			mpq_normalize(w);
#else
			/* Following Knuth 4.5.1, pp330-331
			 *
			 * Let U = u/u' and V = v/v'
			 * Let d_1 = gcd(u', v') */
			mp_digit *d;
			mp_size d_size;
			d_size = MIN(u->den->size, v->den->size);
			MP_TMP_ALLOC(d, d_size);
			mp_gcd(u->, u->den->size,
				   v->, v->den->size, d);
			d_size = mp_rsize(d, d_size);
			if (d_size == 1 && d[0] == 1) {
				/* If d_1 = 1 (~61% of the time), then W = (uv' - u'v)/(u'v')
				 * and doesn't require normalization. */
				mpi_t t1;

				MP_TMP_FREE(d);
				mpi_init(t1);
				mpi_mul(u->num, v->den, t1);
				mpi_mul(u->den, v->num, w->num);
				mpi_sub(t1, w->num, w->num);
				mpi_free(t1);
				mpi_mul(u->den, v->den, w->den);
			} else {
				/* Otherwise, let t = u(v'/d_1)-v(u'/d_1) and d_2 = gcd(t,d_1)
				 * Then the answer is (t/d_2)/((u'/d_1)(v'/d_2)) which is a
				 * normalized fraction. */
				mpi_t t1, t2, d1;

				mpi_init_mp(d1, d, d_size);

				/* t1 = u(v'/d_1) */
				mpi_init(t1);
				mpi_divexact(v->den, d1, t1);
				mpi_mul(t1, u->num, t1);

				/* t2 = v(u'/d_1) */
				mpi_init(t2);
				mpi_divexact(u->den, d1, t2);
				mpi_mul(t2, v->num, t2);

				mpi_sub(t1, t2, t1);

				mpi_gcd(t1, d1, t2);

				mpi_divexact(t1, t2, w->num);
				mpi_free(t1);

				mpi_divexact(u->den, d1, w->den);

				mpi_divexact(v->den, t2, t2);
				mpi_mul(w->den, t2, w->den);
				mpi_free(t2);
			}
			mpq_normalize_nogcd(w);
#endif
		}
	}
}

void
mpq_mul(const mpq *u, const mpq *v, mpq *w)
{
	int s;

	if (mpq_is_zero(u) || mpq_is_zero(v)) {
		mpq_zero(w);
		return;
	}

	s = u->num->sign ^ v->num->sign;
	mpi_mul(u->num, v->num, w->num);
	mpi_mul(u->den, v->den, w->den);
	mpq_normalize(w);
	w->num->sign = s;
	w->den->sign = 0;
}

void
mpq_mul_ui(const mpq *u, unsigned int v, mpq *w)
{
	if (mpq_is_zero(u) || v == 0) {
		mpq_zero(w);
	} else if (v == 1) {
		mpq_set_mpq(w, u);
	} else {
		mpq_t vv;
		mpq_init_ui(vv, v);
		mpq_mul(u, vv, w);
		mpq_free(vv);
	}
}

void
mpq_mul_si(const mpq *u, signed int v, mpq *w)
{
	if (mpq_is_zero(u) || v == 0) {
		mpq_zero(w);
	} else if (v == 1 || v == -1) {
		mpq_set_mpq(w, u);
		if (v == -1)
			mpi_neg(w->num);
	} else {
		/* FIXME. Compute g = gcd of u->den and v.
		 * Then answer is (v/g*u)/(u'/g) */
		mpi_mul_si(u->num, v, w->num);
		mpi_set_mpi(w->den, u->den);
		mpq_normalize(w);
	}
}

void
mpq_div_ui(const mpq *u, unsigned int v, mpq *w)
{
	if (v == 0)
		abort();

	if (mpq_is_zero(u)) {
		mpq_zero(w);
	} else if (v == 1) {
		mpq_set_mpq(w, u);
	} else {
		mpq_t vv;
		mpq_init_ui(vv, v);
		mpq_div(u, vv, w);
		mpq_free(vv);
	}
}

void
mpq_div_si(const mpq *u, signed int v, mpq *w)
{
	if (v == 0)
		abort();

	if (mpq_is_zero(u)) {
		mpq_zero(w);
	} else if (v == 1) {
		mpq_set_mpq(w, u);
	} else {
		mpq_t vv;
		mpq_init_si(vv, v);
		mpq_div(u, vv, w);
		mpq_free(vv);
	}
}

void
mpq_div(const mpq *u, const mpq *v, mpq *w)
{
	if (mpi_is_zero(v->num))
		abort();

	if (mpi_is_zero(u->num)) {
		mpq_zero(w);
		return;
	}

	if (u == v) {
		mpq_one(w);
		return;
	}

	mpi_mul(u->num, v->den, w->num);
	mpi_mul(u->den, v->num, w->den);
	mpq_normalize(w);
}

void
mpq_invert(mpq *q)
{
	ASSERT(!mpi_is_zero(q->den));
	mpi_swap(q->num, q->den);
	q->num->sign ^= q->den->sign;
	q->den->sign = 0;
}

int
mpq_cmp(const mpq *p, const mpq *q)
{
	int r;
	int psize, qsize;
//	int pbits, qbits;

	mp_digit *t1, *t2;
	mp_size t1size, t2size;

	if (p->num->sign != q->num->sign) {
		if (p->num->sign)
			return -1;
		else
			return +1;
	}

	/* XXX use mp_significant_bits() here.. */
	psize = (int)p->num->size - (int)p->den->size;
	qsize = (int)q->num->size - (int)q->den->size;
	if (psize != qsize) {
		r = (psize < qsize) ? -1 : +1;
		if (p->num->sign)
			r = -r;
		return r;
	}

	/* Ok so we can't tell just by looking at size of numbers.
	 * We want to compare A/B to C/D, so compute A*D and B*C and compare. */
	t1size = p->num->size + q->den->size;
	t2size = p->den->size + q->num->size;
	MP_TMP_ALLOC(t1, t1size + t2size);
	t2 = t1 + t1size;
	mp_mul(p->num->digits, p->num->size, q->den->digits, q->den->size, t1);
	mp_mul(p->den->digits, p->den->size, q->num->digits, q->num->size, t2);
	r = mp_cmp(t1, t1size, t2, t2size);
	MP_TMP_FREE(t1);
	if (p->num->sign)
		r = -r;
	return r;
}

int
mpq_cmp_ui(const mpq *p, unsigned int q)
{
	int r;
	mpq_t qq;

	mpq_init_ui(qq, q);
	r = mpq_cmp(p, qq);
	mpq_free(qq);
	return r;
}

int
mpq_cmp_si(const mpq *p,   signed int q)
{
	int r;
	mpq_t qq;

	mpq_init_si(qq, q);
	r = mpq_cmp(p, qq);
	mpq_free(qq);
	return r;
}

int
mpq_cmp_f(const mpq *p,  float q)
{
	int r;
	mpq_t qq;

	mpq_init_f(qq, q);
	r = mpq_cmp(p, qq);
	mpq_free(qq);
	return r;
}

int
mpq_cmp_d(const mpq *p, double q)
{
	int r;
	mpq_t qq;

	mpq_init_d(qq, q);
	r = mpq_cmp(p, qq);
	mpq_free(qq);
	return r;
}

void
mpq_fprint(const mpq *p, unsigned base, FILE *fp)
{
	if (fp == 0)
		fp = stdout;
	mpi_fprint(p->num, base, fp);
	fputc('/', fp);
	mpi_fprint(p->den, base, fp);
}
