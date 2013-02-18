/* mpq.c
 * Copyright (C) 2003-2012 Farooq Mela. All rights reserved. */

#include "weecrypt_memory.h"
#include "mpq.h"
#include "mpi_internal.h"

void
mpq_normalize_nogcd(mpq *q)
{
    ASSERT(!mpi_is_zero(q->den));

    /* We want to normalize the fraction N/D so that:
     * (1) if N=0 then D=1
     * (2) D never has sign (actually always positive since we cant have D=0)
     * (3) gcd(N,D)=1 */

    if (mpi_is_zero(q->num)) {
	mpi_set_u32(q->den, 1);
	return;
    }

    /* Make denominator positive. */
    q->num->sign ^= q->den->sign;
    q->den->sign = 0;
}

void
mpq_normalize(mpq *q)
{
    ASSERT(!mpi_is_zero(q->den));

    /* We want to normalize the fraction N/D so that:
     * (1) if N=0 then D=1
     * (2) D never has sign (actually always positive since we cant have D=0)
     * (3) gcd(N,D)=1 */

    if (mpi_is_zero(q->num)) {
	mpi_set_u32(q->den, 1);
	return;
    }

    /* Make denominator positive. */
    q->num->sign ^= q->den->sign;
    q->den->sign = 0;

    if (mpi_is_one(q->den))
	return;

    /* Adjust so that GCD(N,D)=1, doing as little work as possible :-) */

    /* Count common trailing zero digits. */
    unsigned trailing_zeros = 0;
    while (!(q->num->digits[trailing_zeros] | q->den->digits[trailing_zeros]))
	++trailing_zeros;
    if (trailing_zeros != 0) {
	/* num->size - trailing_zeros and den->size - trailing_zeros are > 0,
	 * since trailing_zeros < gcd_size ≤ min(num->size, den->size) */
	q->num->size -= trailing_zeros;
	mp_copy(q->num->digits + trailing_zeros, q->num->size, q->num->digits);

	q->den->size -= trailing_zeros;
	mp_copy(q->den->digits + trailing_zeros, q->den->size, q->den->digits);
    }

    mp_size gcd_size = MIN(q->num->size, q->den->size);
    ASSERT(gcd_size > 0);
    mp_digit *gcd = MP_TMP_ALLOC(gcd_size);
    mp_gcd(q->num->digits, q->num->size,
	   q->den->digits, q->den->size, gcd);
    MP_NORMALIZE(gcd, gcd_size);
    ASSERT(gcd_size > 0);

    if (gcd_size == 1) {
	if (gcd[0] != 1) {
	    mp_digit r = gcd[0];
	    if ((r & (r - 1)) == 0) {
		mp_digit lg = mp_digit_lsb_shift(r);

		r = mp_rshifti(q->num->digits, q->num->size, lg);
		ASSERT(r == 0);
		q->num->size -= (q->num->digits[q->num->size - 1] == 0);

		r = mp_rshifti(q->den->digits, q->den->size, lg);
		ASSERT(r == 0);
		q->den->size -= (q->den->digits[q->den->size - 1] == 0);
	    } else {
		r = mp_ddivi(q->num->digits, q->num->size, gcd[0]);
		ASSERT(r == 0);
		q->num->size -= (q->num->digits[q->num->size - 1] == 0);

		r = mp_ddivi(q->den->digits, q->den->size, gcd[0]);
		ASSERT(r == 0);
		q->den->size -= (q->den->digits[q->den->size - 1] == 0);
	    }
	    ASSERT(q->num->size != 0);
	    ASSERT(q->den->size != 0);
	}
    } else {
	mp_digit *tmp2 = NULL;
	mp_size tmp2_size = 0;

	if (mp_cmp_eq(q->num->digits, q->num->size, gcd, gcd_size)) {
	    q->num->size = 1;
	    q->num->digits[0] = 1;
	} else {
	    tmp2_size = MAX(q->num->size, q->den->size) - gcd_size + 1;
	    tmp2 = MP_TMP_ALLOC(tmp2_size);

	    mp_divexact(q->num->digits, q->num->size, gcd, gcd_size, tmp2);
	    q->num->size = mp_rsize(tmp2, q->num->size - gcd_size + 1);
	    mp_copy(tmp2, q->num->size, q->num->digits);
	}

	if (mp_cmp_eq(q->den->digits, q->den->size, gcd, gcd_size)) {
	    q->den->size = 1;
	    q->den->digits[0] = 1;
	} else {
	    if (tmp2 == NULL) {
		tmp2_size = q->den->size - gcd_size + 1;
		tmp2 = MP_TMP_ALLOC(tmp2_size);
	    }

	    mp_divexact(q->den->digits, q->den->size, gcd, gcd_size, tmp2);
	    q->den->size = mp_rsize(tmp2, q->den->size - gcd_size + 1);
	    mp_copy(tmp2, q->den->size, q->den->digits);
	}

	if (tmp2)
	    MP_TMP_FREE(tmp2);
    }
    MP_TMP_FREE(gcd);
}

mpq *
mpq_new(void)
{
    mpq *p;

    p = MALLOC(sizeof(*p));
    mpi_init(p->num);
    mpi_init_u32(p->den, 1);
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
    mpi_init_u32(p->den, 1);
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
    mpi_init_u32(p->den, 1);
}

void
mpq_init_u32(mpq *p, uint32_t q)
{
    mpi_init_u32(p->num, q);
    mpi_init_u32(p->den, 1);
}

void
mpq_init_s32(mpq *p, int32_t q)
{
    mpi_init_s32(p->num, q);
    mpi_init_u32(p->den, 1);
}

void
mpq_init_u64(mpq *p, uint64_t q)
{
    mpi_init_u64(p->num, q);
    mpi_init_u32(p->den, 1);
}

void
mpq_init_s64(mpq *p, int64_t q)
{
    mpi_init_s64(p->num, q);
    mpi_init_u32(p->den, 1);
}

void
mpq_init_u32_u32(mpq *p, uint32_t n, uint32_t d)
{
    ASSERT(d != 0);

    mpi_init_u32(p->num, n);
    mpi_init_u32(p->den, d);

    mpq_normalize(p);
}

void
mpq_init_s32_s32(mpq *p, int32_t n, int32_t d)
{
    ASSERT(d != 0);

    mpi_init_s32(p->num, n);
    mpi_init_s32(p->den, d);

    mpq_normalize(p);
}

void
mpq_init_u64_u64(mpq *p, uint64_t n, uint64_t d)
{
    ASSERT(d != 0);

    mpi_init_u64(p->num, n);
    mpi_init_u64(p->den, d);

    mpq_normalize(p);
}

void
mpq_init_s64_s64(mpq *p, int64_t n, int64_t d)
{
    ASSERT(d != 0);

    mpi_init_s64(p->num, n);
    mpi_init_s64(p->den, d);

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
    /* TODO: handle denormals, NaN, +/- infinity */
    if (f == 0.0) {
	mpq_zero(p);
	return;
    }

    /* sign = f_31
     *  exp = f_23...30, excess 127
     * frac = f_0...22 + 2^23 */
    union {
	float f;
	uint32_t u32;
    } ff;
    ff.f = f;
    int sign = ff.u32 >> 31;
    int exp = (ff.u32 >> 23) & 0xff;
    ff.u32 |= (1U << 23);
    ff.u32 &= (1U << 24) - 1;

    mpi_set_u32(p->num, ff.u32);

    p->num->sign = sign;
    mpi_set_u32(p->den, 1);
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
    if (mpq_is_zero(p))
	return 0.0;

    if (mpi_is_one(p->den))
	return mpi_get_f(p->num);

    /* Single precision floats effectively have a 24-bit significand. Dividing
     * an N-bit number by an M-bit number yields either an (N-M)-bit number or
     * an (N-M+1)-bit number. Since we need to guarantee at LEAST 24 bits, we
     * compute (U*2^K)/V for some K such that N+K-M ≥ 53. If we get an extra
     * bit, we just throw it away. Since this means N+K ≥ M+24, the minimum
     * possible value for K = M-N+24. */
    const int N = mpi_significant_bits(p->num);
    const int M = mpi_significant_bits(p->den);
    const int K = M - N + 24;
    const int sd = (K > 0) ? (K / MP_DIGIT_BITS) : ((-K) / MP_DIGIT_BITS);
    const int sb = (K > 0) ? (K % MP_DIGIT_BITS) : ((-K) % MP_DIGIT_BITS);

    mp_digit *frac = NULL;
    mp_size fsize;
    if (N-M < 24) {	/* K>0 */
	mp_size tsize = p->num->size + 1 + sd;
	mp_digit *tmp = MP_TMP_ALLOC(tsize);
	if (sd != 0)
	    mp_zero(tmp, sd);
	tmp[tsize - 1] = mp_lshift(p->num->digits, p->num->size, sb, tmp + sd);
	tsize -= (tmp[tsize - 1] == 0);
	/* Now we have U*2^(M-N+24). Divide it by V */
	fsize = tsize - p->den->size + 1;
	frac = MP_TMP_ALLOC(fsize);
	mp_div(tmp, tsize, p->den->digits, p->den->size, frac);
	MP_TMP_FREE(tmp);
    } else { /* N-M ≥ 24, K ≤ 0 */
	fsize = p->num->size - sd - p->den->size + 1;
	frac = MP_TMP_ALLOC(fsize);
	mp_div(p->num->digits - sd, p->num->size + sd,
	       p->den->digits, p->den->size, frac);
	if (sb != 0)
	    mp_rshifti(frac, fsize, sb);
    }
    MP_NORMALIZE(frac, fsize);
    int exp = mp_significant_bits(frac, fsize) - 24;
    if (exp) {
	ASSERT(exp == 1);
	mp_rshifti(frac, fsize, 1); /* XXX throws away a bit, round? */
	fsize -= (frac[fsize-1] == 0);
    }
    ASSERT(mp_significant_bits(frac, fsize) == 24);

    uint64_t q;
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

    ASSERT((q & ((uint32_t)1 << 23)) != 0);
    q -= (uint32_t)1 << 23;
    exp += 127 + 23 - K;	/* [bias] + [min exp to satisfy (q^-exp)<1] - K */
    if (exp < 0 || exp > 0xff) {
	fprintf(stderr, "floating-point %s in %s\n",
		(exp < 0) ? "underflow" : "overflow", __PRETTY_FUNCTION__);
	return 0.0;
    }
    q |= (uint32_t)exp << 23;
    if (p->num->sign)
	q |= (uint32_t)1 << 31;
    union {
	float f;
	uint32_t u32;
    } ff;
    ff.u32 = q;
    return ff.f;
}

void
mpq_set_d(mpq *p, double d)
{
    /* TODO: handle denormals, NaN, +/- infinity */
    if (d == 0.0) {
	mpq_zero(p);
	return;
    }

    /* sign = f_63
     *  exp = f_52..62, excess 1023
     * frac = f_0..51 + 2^52 */
    union {
	double d;
	uint64_t u64;
    } dd;
    dd.d = d;
    bool sign = (dd.u64 >> 63);
    int exp = (int)(dd.u64 >> 52);
    exp &= ((1U << 11) - 1);
    dd.u64 &= (UINT64_C(1) << 53) - 1;

    mp_size digits = (53 + MP_DIGIT_BITS - 1) / MP_DIGIT_BITS;
    MPI_SIZE(p->num, digits);
#if MP_DIGIT_SIZE == 8
    p->num->digits[0] = dd.u64;
#else
    for (mp_size j = 0; j < digits; j++) {
	p->num->digits[j] = (mp_digit)dd.u64;
	dd.u64 >>= MP_DIGIT_BITS;
    }
#endif
    p->num->digits[52 / MP_DIGIT_BITS] |= (mp_digit)1 << (52 % MP_DIGIT_BITS);

    p->num->sign = sign;
    mpi_set_u32(p->den, 1);
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
    /* This code and mpq_set_d() unfortunately uses the uint64_t data type,
     * which we could avoid if we had a mechanism for knowing or detecting the
     * endian-ness of the host machine. */
    if (mpq_is_zero(p))
	return 0.0;

    if (mpi_is_one(p->den))
	return mpi_get_d(p->num);

    /* Double precision floats effectively have a 53-bit significand. Dividing
     * an N-bit number by an M-bit number yields either an (N-M)-bit number or
     * an (N-M+1)-bit number. Since we need to guarantee at LEAST 53 bits, we
     * compute (U*2^K)/V for some K such that N+K-M ≥ 53. If we get an extra
     * bit, we just throw it away. Since this means N+K ≥ M+53, the minimum
     * possible value for K = M-N+53. */
    const int N = mpi_significant_bits(p->num);
    const int M = mpi_significant_bits(p->den);
    const int K = M - N + 53;
    const int sd = (K > 0) ? (K / MP_DIGIT_BITS) : ((-K) / MP_DIGIT_BITS);
    const int sb = (K > 0) ? (K % MP_DIGIT_BITS) : ((-K) % MP_DIGIT_BITS);

    mp_digit *frac = NULL;
    mp_size fsize;
    if (N-M < 53) {	/* K>0 */
	mp_digit *tmp;
	mp_size tsize;

	tsize = p->num->size + 1 + sd;
	tmp = MP_TMP_ALLOC(tsize);
	if (sd != 0)
	    mp_zero(tmp, sd);
	tmp[tsize - 1] = mp_lshift(p->num->digits, p->num->size, sb, tmp + sd);
	tsize -= (tmp[tsize - 1] == 0);
	/* Now we have U*2^(M-N+53). Divide it by V */
	fsize = tsize - p->den->size + 1;
	frac = MP_TMP_ALLOC(fsize);
	mp_div(tmp, tsize, p->den->digits, p->den->size, frac);
	MP_TMP_FREE(tmp);
    } else { /* N-M ≥ 53, K ≤ 0 */
	fsize = p->num->size - sd - p->den->size + 1;
	frac = MP_TMP_ALLOC(fsize);
	mp_div(p->num->digits + sd, p->num->size - sd,
	       p->den->digits, p->den->size, frac);
	if (sb != 0)
	    mp_rshifti(frac, fsize, sb);
    }
    MP_NORMALIZE(frac, fsize);
    int exp = mp_significant_bits(frac, fsize) - 53;
    if (exp) {
	ASSERT(exp == 1);
	mp_rshifti(frac, fsize, 1);	/* XXX: throws away a bit, round? */
	fsize -= (frac[fsize - 1] == 0);
    }
    ASSERT(mp_significant_bits(frac, fsize) == 53);

    uint64_t q;
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

    ASSERT((q & ((uint64_t)1 << 52)) != 0);
    q -= (uint64_t)1 << 52;
    exp += 1023 + 52 - K;	/* [bias] + [min exp to satisfy (q^-exp)<1] - K */
    if (exp < 0 || exp > 0x7ff) {
	fprintf(stderr, "floating-point %s in %s\n",
		(exp < 0) ? "underflow" : "overflow", __PRETTY_FUNCTION__);
	return 0.0;
    }
    q |= (uint64_t)exp << 52;
    if (p->num->sign)
	q |= (uint64_t)1 << 63;
    union {
	double d;
	uint64_t u64;
    } dd;
    dd.u64 = q;
    return dd.d;
}

void
mpq_free(mpq *p)
{
    ASSERT(p != NULL);

    mpi_free(p->num);
    mpi_free(p->den);
}

void
mpq_set_mpq(mpq *p, const mpq *q)
{
    ASSERT(p != NULL);
    ASSERT(q != NULL);

    if (p != q) {
	mpi_set_mpi(p->num, q->num);
	mpi_set_mpi(p->den, q->den);
    }
}

void
mpq_set_u32(mpq *p, uint32_t q)
{
    ASSERT(p != NULL);

    mpi_set_u32(p->num, q);
    mpi_set_u32(p->den, 1);
}

void
mpq_set_s32(mpq *p, int32_t q)
{
    ASSERT(p != NULL);

    mpi_set_s32(p->num, q);
    mpi_set_u32(p->den, 1);
}

void
mpq_set_u64(mpq *p, uint64_t q)
{
    ASSERT(p != NULL);

    mpi_set_u64(p->num, q);
    mpi_set_u32(p->den, 1);
}

void
mpq_set_s64(mpq *p, int64_t q)
{
    ASSERT(p != NULL);

    mpi_set_s64(p->num, q);
    mpi_set_u32(p->den, 1);
}

void
mpq_set_u32_u32(mpq *p, uint32_t n, uint32_t d)
{
    ASSERT(p != NULL);
    ASSERT(d != 0);

    mpi_set_u32(p->num, n);
    mpi_set_u32(p->den, d);
    mpq_normalize(p);
}

void
mpq_set_s32_s32(mpq *p, int32_t n, int32_t d)
{
    ASSERT(p != NULL);
    ASSERT(d != 0);

    mpi_set_s32(p->num, n);
    mpi_set_s32(p->den, d);
    mpq_normalize(p);
}

void
mpq_set_u64_u64(mpq *p, uint64_t n, uint64_t d)
{
    ASSERT(p != NULL);
    ASSERT(d != 0);

    mpi_set_u64(p->num, n);
    mpi_set_u64(p->den, d);
    mpq_normalize(p);
}

void
mpq_set_s64_s64(mpq *p, int64_t n, int64_t d)
{
    ASSERT(p != NULL);
    ASSERT(d != 0);

    mpi_set_s64(p->num, n);
    mpi_set_s64(p->den, d);
    mpq_normalize(p);
}

void
mpq_zero(mpq *q)
{
    ASSERT(q != NULL);

    mpi_zero(q->num);
    mpi_set_u32(q->den, 1);
}

void
mpq_one(mpq *q)
{
    ASSERT(q != NULL);

    mpi_set_u32(q->num, 1);
    mpi_set_u32(q->den, 1);
}

#if 0
void
mpq_pinf(mpq *q)
{
    mpi_set_u32(q->num, 1);
    mpi_zero(q->den);
}

void
mpq_ninf(mpq *q)
{
    mpi_set_u32(q->num, 1);
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
    ASSERT(q != NULL);

    if (q->num->size)
	q->num->sign ^= 1;
}

void
mpq_abs(mpq *q)
{
    ASSERT(q != NULL);

    q->num->sign = 0;
}

void
mpq_swap(mpq *p, mpq *q)
{
    ASSERT(p != NULL);
    ASSERT(q != NULL);

    if (p != q) {
	mpi_swap(p->num, q->num);
	mpi_swap(p->den, q->den);
    }
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
#if 0
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
	    mp_size d_size = MIN(u->den->size, v->den->size);
	    mp_digit *d = MP_TMP_ALLOC(d_size);
	    mp_gcd(u->den->digits, u->den->size,
		   v->den->digits, v->den->size, d);
	    MP_NORMALIZE(d, d_size);
	    if (d_size == 1 && d[0] == 1) {
		/* If d_1 = 1 (~61% of the time), then W = (uv' + u'v)/(u'v')
		 * and doesn't require normalization. */
		mpi_t t1;

		MP_TMP_FREE(d);
		mpi_init_size(t1, u->num->size + v->den->size);
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
		mpi_init_size(t1, u->num->size + v->den->size - d1->size + 1);
		mpi_divexact(v->den, d1, t1);
		mpi_mul(t1, u->num, t1);

		/* t2 = v(u'/d_1) */
		mpi_init_size(t2, v->num->size + u->den->size - d1->size + 1);
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
    ASSERT(u != NULL);
    ASSERT(v != NULL);
    ASSERT(w != NULL);

    if (u == v) {
	mpq_zero(w);
    } else if (mpq_is_zero(v)) {
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
#if 0
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
	     * Let d_1 = gcd(u',v') */
	    mp_size d_size = MIN(u->den->size, v->den->size);
	    mp_digit *d = MP_TMP_ALLOC(d_size);
	    mp_gcd(u->den->digits, u->den->size,
		   v->den->digits, v->den->size, d);
	    MP_NORMALIZE(d, d_size);
	    if (d_size == 1 && d[0] == 1) {
		/* If d_1 = 1 (~61% of the time), then W = (uv' - u'v)/(u'v')
		 * and doesn't require normalization. */
		mpi_t t1;

		MP_TMP_FREE(d);
		mpi_init_size(t1, u->num->size + v->den->size);
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
		mpi_init_size(t1, u->num->size + v->den->size - d1->size + 1);
		mpi_divexact(v->den, d1, t1);
		mpi_mul(t1, u->num, t1);

		/* t2 = v(u'/d_1) */
		mpi_init_size(t2, v->num->size + u->den->size - d1->size + 1);
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
    /* If U or V is zero, then the product is zero. */
    if (mpq_is_zero(u) || mpq_is_zero(v)) {
	mpq_zero(w);
	return;
    }

    /* We don't need to worry about normalizing because U coprime to U'
     * implies U^2 coprime to U'^2 */
    if (u == v) {
	mpi_sqr(u->num, w->num);
	mpi_sqr(u->den, w->den);
    } else {
	/* We know that U and U' are coprime, and that V and V' are coprime.
	 * Since we want W and W' to be coprime, we only need to find common
	 * factors between U and V', and V and U'.
	 *
	 * Let G1 = gcd(U,V') and G2 = gcd(V,U').
	 * Then, the normalized product (U*V)/(U'*V') is can be written as
	 *
	 *  (U/G1)(V/G2)
	 * --------------
	 * (U'/G2)(V'/G1)
	 *
	 * where are all divisions by G1 or G2 are exact. */

	mpi_t g1;
	mpi_init_size(g1, MIN(u->num->size, v->den->size));

	mpi_gcd(u->num, v->den, g1);
	if (mpi_is_one(g1)) {
	    mpi_gcd(v->num, u->den, g1);
	    if (mpi_is_one(g1)) {	/* GCD(U,V') = 1 and GCD(U',V) = 1 */
		/* No common factors. Just multiply. */
		mpi_mul(u->num, v->num, w->num);
		mpi_mul(u->den, v->den, w->den);
	    } else {				/* GCD(U,V') = 1 and GCD(U',V) ≠ 1 */
		/* Numerator is U*(V/G), denominator is (U'/G)*V' */
		mpi_t t1;
		mpi_init_size(t1, v->num->size - g1->size + 1);

		mpi_divexact(v->num, g1, t1);	/* T1 = V/G */
		mpi_mul(u->num, t1, w->num);	/* W = U * T1 */

		mpi_divexact(u->den, g1, t1);	/* T1 = U'/G */
		mpi_mul(v->den, t1, w->den);	/* W' = V' * T1 */

		mpi_free(t1);
	    }
	} else {
	    mpi_t g2, t1;
	    mpi_init_size(g2, MIN(v->num->size, u->den->size));

	    mpi_gcd(v->num, u->den, g2);
	    if (mpi_is_one(g2)) {	/* GCD(U,V') ≠ 1 and GCD(U',V) = 1 */
		mpi_init_size(t1, u->num->size - g1->size + 1);

		/* Numerator is (U/G)*V, denominator is U'*(V'/G) */
		mpi_divexact(u->num, g1, t1);	/* T1 = U/G */
		mpi_mul(v->num, t1, w->num);	/* W = V * T1 */

		mpi_divexact(v->den, g1, t1);	/* T1 = V'/G */
		mpi_mul(u->den, t1, w->den);	/* W' = U' * T1 */
	    } else {				/* GCD(U,V') ≠ 1 and GCD(U',V) ≠ 1 */
		/* Numerator is (U/G1)*(V/G2), denominator is (U'/G2)*(V'/G1) */
		mpi_t t2;

		mpi_init_size(t1, u->num->size - g1->size + 1);
		mpi_init_size(t2, v->num->size - g2->size + 1);

		mpi_divexact(u->num, g1, t1);	/* T1 = U/G1 */
		mpi_divexact(v->num, g2, t2);	/* T1 = V/G2 */
		mpi_mul(t1, t2, w->num);		/* W = T1 * T2 */

		mpi_divexact(v->den, g1, t1);	/* T1 = V'/G1 */
		mpi_divexact(u->den, g2, t2);	/* T2 = U'/G2 */
		mpi_mul(t1, t2, w->den);		/* W' = T1 * T2 */

		mpi_free(t2);
	    }
	    mpi_free(t1);
	    mpi_free(g2);
	}
	mpi_free(g1);
    }

    mpq_normalize_nogcd(w);
}

void
mpq_muli(mpq *w, const mpi *v)
{
    if (mpi_is_zero(v)) {
	mpq_zero(w);
    } else if (mpi_is_one(v)) {
	/* Do nothing. */
    } else if (mpi_is_negone(v)) {
	mpq_neg(w);
    } else {
	/* TODO: GCD optimization. */
	mpi_mul(w->num, v, w->num);
	mpq_normalize(w);
    }
}

void
mpq_mul_u32(const mpq *u, uint32_t v, mpq *w)
{
    if (mpq_is_zero(u) || v == 0) {
	mpq_zero(w);
    } else if (v == 1) {
	mpq_set_mpq(w, u);
    } else {
	/* TODO: GCD optimization. */
	mpi_mul_u32(u->num, v, w->num);
	mpi_set_mpi(w->den, u->den);
	mpq_normalize(w);
    }
}

void
mpq_mul_s32(const mpq *u, int32_t v, mpq *w)
{
    if (mpq_is_zero(u) || v == 0) {
	mpq_zero(w);
    } else if (v == 1 || v == -1) {
	mpq_set_mpq(w, u);
	if (v == -1)
	    mpi_neg(w->num);
    } else {
	/* TODO: GCD optimization.
	 * Compute g = gcd(u', v)
	 * Then answer is (v/g*u)/(u'/g) - exact divisions and no GCD after. */
	mpi_mul_s32(u->num, v, w->num);
	mpi_set_mpi(w->den, u->den);
	mpq_normalize(w);
    }
}

void
mpq_div_u32(const mpq *u, uint32_t v, mpq *w)
{
    ASSERT(v != 0);

    if (mpq_is_zero(u)) {
	mpq_zero(w);
    } else if (v == 1) {
	mpq_set_mpq(w, u);
    } else {
	/* TODO: GCD optimization. */
	mpi_set_mpi(w->num, u->num);
	mpi_mul_u32(u->den, v, w->den);
	mpq_normalize(w);
    }
}

void
mpq_div_s32(const mpq *u, int32_t v, mpq *w)
{
    ASSERT(v != 0);

    if (mpq_is_zero(u)) {
	mpq_zero(w);
    } else if (v == 1) {
	mpq_set_mpq(w, u);
    } else {
	/* TODO: GCD optimization. */
	mpi_set_mpi(w->num, u->num);
	mpi_mul_s32(u->den, v, w->den);
	mpq_normalize(w);
    }
}

void
mpq_divi(mpq *w, const mpi *v)
{
    ASSERT(!mpi_is_zero(v));

    if (mpi_is_one(v)) {
	/* Do nothing. */
    } else if (mpi_is_negone(v)) {
	mpq_neg(w);
    } else {
	/* TODO: GCD optimization. */
	mpi_mul(w->den, v, w->den);
	mpq_normalize(w);
    }
}

void
mpq_div(const mpq *u, const mpq *v, mpq *w)
{
    ASSERT(!mpi_is_zero(v->num));

    if (mpi_is_zero(u->num)) {
	mpq_zero(w);
	return;
    }

    if (u == v) {
	mpq_one(w);
	return;
    }

    /* TODO: GCD optimization. */
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
    if (p->num->sign != q->num->sign)
	return p->num->sign ? -1 : +1;

    int pbits = (int)mpi_significant_bits(p->num) -
	(int)mpi_significant_bits(p->den);
    int qbits = (int)mpi_significant_bits(q->num) -
	(int)mpi_significant_bits(q->den);
    if (pbits != qbits) {
	int cmp = (pbits < qbits) ? -1 : +1;
	return (p->num->sign) ? -cmp : cmp;
    }

    /* Ok so we can't tell just by looking at size of numbers.
     * We want to compare A/B to C/D, so compute A*D and B*C and compare. */
    const mp_size t1size = p->num->size + q->den->size;
    const mp_size t2size = p->den->size + q->num->size;
    mp_digit *t1 = MP_TMP_ALLOC(t1size + t2size);
    mp_digit *t2 = t1 + t1size;
    mp_mul(p->num->digits, p->num->size, q->den->digits, q->den->size, t1);
    mp_mul(p->den->digits, p->den->size, q->num->digits, q->num->size, t2);
    int cmp = mp_cmp(t1, t1size, t2, t2size);
    MP_TMP_FREE(t1);

    return (p->num->sign) ? -cmp : cmp;
}

int
mpq_cmp_u32(const mpq *p, uint32_t q)
{
    int r;
    mpq_t qq;

    mpq_init_u32(qq, q);
    r = mpq_cmp(p, qq);
    mpq_free(qq);
    return r;
}

int
mpq_cmp_s32(const mpq *p, int32_t q)
{
    int r;
    mpq_t qq;

    mpq_init_s32(qq, q);
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
    if (!mpi_is_one(p->den)) {
	fputc('/', fp);
	mpi_fprint(p->den, base, fp);
    }
}
