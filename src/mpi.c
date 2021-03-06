/* mpi.c
 * Copyright (C) 2001-2012 Farooq Mela. All rights reserved. */

#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "weecrypt_memory.h"
#include "mpi.h"
#include "mpi_internal.h"

#define MPI_INIT_BYTES	8
#define MPI_INIT_DIGITS	((MPI_INIT_BYTES + MP_DIGIT_SIZE - 1) / MP_DIGIT_SIZE)

void
mpi_init(mpi *n)
{
    ASSERT(n != NULL);

    n->alloc = MPI_INIT_DIGITS;
    n->digits = mp_new0(MPI_INIT_DIGITS);
    n->size = 0;
    n->sign = 0;
}

void
mpi_init_u32(mpi *n, uint32_t ui)
{
    mpi_init(n);
    mpi_set_u32(n, ui);
}

void
mpi_init_s32(mpi *n, int32_t si)
{
    mpi_init(n);
    mpi_set_s32(n, si);
}

void
mpi_init_u64(mpi *n, uint64_t ul)
{
    mpi_init(n);
    mpi_set_u64(n, ul);
}

void
mpi_init_s64(mpi *n, int64_t sl)
{
    mpi_init(n);
    mpi_set_s64(n, sl);
}

void
mpi_init_mpi(mpi *p, const mpi *q)
{
    ASSERT(p != NULL);
    ASSERT(q != NULL);

    if (q->size) {
	p->digits = mp_dup(q->digits, q->size);
	p->size = p->alloc = q->size;
	p->sign = q->sign;
    } else {
	mpi_init(p);
    }
}

void
mpi_init_size(mpi *p, mp_size size)
{
    if (size == 0)
	size = 1;

    p->digits = mp_new(size);
    p->alloc = size;
    p->size = 0;
    p->sign = 0;
}

void
mpi_init_str(mpi *p, const char *str, unsigned base)
{
    while (*str && isspace(*str))
	++str;
    bool neg = false;
    if (*str == '+') {
	++str;
    } else if (*str == '-') {
	neg = true;
	++str;
    }
    mp_size size = 0;
    mp_digit *digits = mp_from_str(str, base, &size);
    if (digits) {
	mpi_init_mp(p, digits, size);
	p->sign = neg;
    } else {
	mpi_init(p);
    }
}

void
mpi_init_mp(mpi *p, mp_digit *n, mp_size size)
{
    p->digits = n;
    p->size = mp_rsize(n, size);
    p->alloc = size;
    p->sign = 0;
}

void
mpi_free(mpi *n)
{
    ASSERT(n != NULL);

    mp_free(n->digits);
}

void
mpi_free_zero(mpi *n)
{
    ASSERT(n != NULL);

    mp_zero(n->digits, n->alloc);
    mp_free(n->digits);
}

void
mpi_set_mpi(mpi *p, const mpi *q)
{
    ASSERT(p != NULL);
    ASSERT(q != NULL);

    if (p != q) {
	if (q->size == 0) {
	    mpi_zero(p);
	} else {
	    MPI_SIZE(p, q->size);
	    mp_copy(q->digits, q->size, p->digits);
	    p->sign = q->sign;
	}
    }
}

void
mpi_zero(mpi *n)
{
    ASSERT(n != NULL);

    n->sign = 0;
    n->size = 0;
}

void
mpi_neg(mpi *n)
{
    ASSERT(n != NULL);

    if (n->size) /* Don't flip sign if N = 0 */
	n->sign ^= 1;
}

void
mpi_abs(mpi *n)
{
    ASSERT(n != NULL);

    n->sign = 0;
}

void
mpi_set_u32(mpi *n, uint32_t m)
{
    n->sign = 0;
    if (m == 0) {
	n->size = 0;
	return;
    }
#if MP_DIGIT_MAX >= UINT32_MAX
    MPI_SIZE(n, 1);
    n->digits[0] = (mp_digit)m;
#else
    mp_size j;
    uint32_t t = m;

    mp_size j = 0;
    while (t) {
	t >>= MP_DIGIT_BITS;
	++j;
    }
    MPI_MIN_ALLOC(n, j);
    for (j = 0; m; ++j) {
	n->digits[j] = (mp_digit)m;
	m >>= MP_DIGIT_BITS;
    }
    n->size = j;
#endif
}

void
mpi_set_s32(mpi *n, int32_t m)
{
    if (m >= 0) {
	mpi_set_u32(n, m);
    } else {
	mpi_set_u32(n, -m);
	n->sign = 1;
    }
}

void
mpi_set_u64(mpi *n, uint64_t m)
{
    if (m == 0) {
	mpi_zero(n);
	return;
    }

#if MP_DIGIT_MAX >= UINT64_MAX
    MPI_SIZE(n, 1);
    n->digits[0] = (mp_digit)m;
#else
    mp_size j;
    uint64_t t = m;

    for (j = 0; t; j++)
	t >>= MP_DIGIT_BITS;

    MPI_MIN_ALLOC(n, j);
    for (j = 0; m; j++) {
	n->digits[j] = (mp_digit)m;
	m >>= MP_DIGIT_BITS;
    }
    n->size = j;
#endif
    n->sign = 0;
}

void
mpi_set_s64(mpi *n, int64_t m)
{
    if (m >= 0) {
	mpi_set_u64(n, m);
    } else {
	mpi_set_u64(n, -m);
	n->sign = 1;
    }
}

bool
mpi_get_s32(const mpi *p, int32_t *q)
{
    const unsigned bits = mp_significant_bits(p->digits, p->size);
    if (bits >= (sizeof(*q) * CHAR_BIT) - 1)
	return false;

    /* TODO: rewrite this in a more clever way. */
    int32_t r = 0;
    for (unsigned i = 0; i < bits; i++)
	if (p->digits[i / MP_DIGIT_BITS] & (1U << (i % MP_DIGIT_BITS)))
	    r |= (1U << i);
    if (p->sign)
	r = -r;
    *q = r;
    return true;
}

bool
mpi_get_u32(const mpi *p, uint32_t *q)
{
    if (p->sign)
	return false;
    const unsigned bits = mp_significant_bits(p->digits, p->size);
    if (bits >= (sizeof(*q) * CHAR_BIT))
	return false;

    /* TODO: rewrite this in a more clever way. */
    uint32_t r = 0;
    for (unsigned i = 0; i < bits; i++)
	if (p->digits[i / MP_DIGIT_BITS] & (1U << (i % MP_DIGIT_BITS)))
	    r |= (1U << i);
    *q = r;
    return true;
}

bool
mpi_set_str(mpi *p, const char *str, unsigned base)
{
    ASSERT(p != NULL);
    ASSERT(str != NULL);

    while (*str && isspace(*str))
	++str;
    bool neg = false;
    if (*str == '+') {
	++str;
    } else if (*str == '-') {
	neg = true;
	++str;
    }
    mp_size size = 0;
    mp_digit *digits = mp_from_str(str, base, &size);
    if (digits) {
	mp_free(p->digits);
	p->digits = digits;
	p->size = p->alloc = size;
	p->sign = neg;
	return true;
    }
    return false;
}

float
mpi_get_f(const mpi *p)
{
    const float delta = MP_DIGIT_MAX;
    const float base = delta + 1.f;
    if (p->size == 0)
	return 0.f;

    mp_size j = p->size - 1;
    float q = p->digits[j];
    while (j) {
	q *= base;
	if (q == q + delta) {
	    while (--j)
		q *= base;
	    break;
	}
	q += p->digits[--j];
    }
    if (p->sign)
	q = -q;
    return q;
}

double
mpi_get_d(const mpi *p)
{
    if (p->size == 0)
	return 0.;

    const double delta = MP_DIGIT_MAX;
    const double base = delta + 1.;

    mp_size j = p->size - 1;
    double q = p->digits[j];
    while (j) {
	q *= base;
	if (q == q + delta) {
	    while (--j)
		q *= base;
	    break;
	}
	q += p->digits[--j];
    }
    if (p->sign)
	q = -q;
    return q;
}

void
mpi_inc(mpi *n)
{
    if (n->size == 0) {	/* Make it +1 */
	ASSERT(n->sign == 0);
	MPI_SIZE(n, 1);
	n->digits[0] = 1;
	return;
    }

    if (n->sign == 0) {
	if (mp_inc(n->digits, n->size)) {
	    MPI_MIN_ALLOC(n, n->size + 1);
	    n->digits[n->size++] = 1;
	}
    } else {
	mp_dec(n->digits, n->size);
	n->size -= (n->digits[n->size - 1] == 0);
	if (n->size == 0)
	    n->sign = 0;
    }
}

void
mpi_dec(mpi *n)
{
    if (n->size == 0) {	/* Make it -1 */
	ASSERT(n->sign == 0);
	MPI_SIZE(n, 1);
	n->digits[0] = 1;
	n->sign = 1;
	return;
    }

    if (n->sign) {
	if (mp_inc(n->digits, n->size)) {
	    MPI_MIN_ALLOC(n, n->size + 1);
	    n->digits[n->size++] = 1;
	}
    } else {
	mp_dec(n->digits, n->size);
	n->size -= (n->digits[n->size - 1] == 0);
    }
}

void
mpi_rand_ctx(mpi *n, unsigned bits, mt64_context *ctx)
{
    ASSERT(n != NULL);

    n->sign = 0; /* XXX */
    if (!bits) {
	n->size = 0;
	return;
    }

    const unsigned hbits = bits % MP_DIGIT_BITS;
    const unsigned digits = (bits / MP_DIGIT_BITS) + (hbits != 0);
    MPI_SIZE(n, digits);
    /* Let mp_rand_digits worry about whether ctx is NULL or not. */
    mp_rand_digits(ctx, n->digits, digits);
    if (hbits != 0) {
	n->digits[n->size - 1] &= (((mp_digit)1 << hbits) - 1);
	n->digits[n->size - 1] |= ((mp_digit)1) << (hbits-1);
    } else {
	n->digits[n->size - 1] |= MP_DIGIT_MSB;
    }
    ASSERT(mpi_significant_bits(n) == bits);
}

void
mpi_rand(mpi *n, unsigned bits)
{
    return mpi_rand_ctx(n, bits, NULL);
}

void
mpi_swap(mpi *a, mpi *b)
{
    mpi tm;

    tm = *a;
    *a = *b;
    *b = tm;
}

int
mpi_cmp(const mpi *p, const mpi *q)
{
    int r;

    if (p == q)
	return 0;

    if (p->sign ^ q->sign)
	return p->sign ? -1 : +1;
    r = mp_cmp(p->digits, p->size, q->digits, q->size);
    return p->sign ? -r : +r;
}

int
mpi_cmp_u32(const mpi *p, uint32_t q)
{
    int r;
    mpi_t qq;

    mpi_init_u32(qq, q);
    r = mpi_cmp(p, qq);
    mpi_free(qq);
    return r;
}

int
mpi_cmp_s32(const mpi *p, int32_t q)
{
    int r;
    mpi_t qq;

    mpi_init_s32(qq, q);
    r = mpi_cmp(p, qq);
    mpi_free(qq);
    return r;
}

int
mpi_cmp_u64(const mpi *p, uint64_t q)
{
    int r;
    mpi_t qq;

    mpi_init_u64(qq, q);
    r = mpi_cmp(p, qq);
    mpi_free(qq);
    return r;
}

int
mpi_cmp_s64(const mpi *p, int64_t q)
{
    int r;
    mpi_t qq;

    mpi_init_s64(qq, q);
    r = mpi_cmp(p, qq);
    mpi_free(qq);
    return r;
}

void
mpi_add(const mpi *a, const mpi *b, mpi *c)
{
    if (a->size == 0) {
	if (b->size == 0)
	    c->size = 0;
	else
	    mpi_set_mpi(c, b);
	return;
    } else if (b->size == 0) {
	mpi_set_mpi(c, a);
	return;
    }

    if (a == b) {
	mp_digit cy;
	if (a == c) {
	    cy = mp_lshifti(c->digits, c->size, 1);
	} else {
	    MPI_SIZE(c, a->size);
	    cy = mp_lshift(a->digits, a->size, 1, c->digits);
	}
	if (cy) {
	    MPI_MIN_ALLOC(c, c->size + 1);
	    c->digits[c->size++] = cy;
	}
	return;
    }

    /* Note: this code is careful so it works for A == C or B == C!! */
    mp_size size;
    if (a->sign == b->sign) {	/* Both positive or negative. */
	size = MAX(a->size, b->size);
	MPI_MIN_ALLOC(c, size + 1);
	mp_digit cy = mp_add(a->digits, a->size, b->digits, b->size, c->digits);
	if (cy)
	    c->digits[size++] = cy;
	else
	    MP_NORMALIZE(c->digits, size);
	c->sign = a->sign;
    } else {					/* Differing signs. */
	if (a->sign)
	    SWAP(a, b, const mpi *);
	ASSERT(a->sign == 0);
	ASSERT(b->sign == 1);

	int cmp = mp_cmp(a->digits, a->size, b->digits, b->size);
	if (cmp > 0) {							/* |A| > |B| */
	    /* If B < 0 and |A| > |B|, then C = A - |B| */
	    MPI_MIN_ALLOC(c, a->size);
	    ASSERT(mp_sub(a->digits, a->size,
			  b->digits, b->size, c->digits) == 0);
	    c->sign = 0;
	    size = mp_rsize(c->digits, a->size);
	} else if (cmp < 0) {					/* |A| < |B| */
	    /* If B < 0 and |A| < |B|, then C = -(|B| - |A|) */
	    MPI_MIN_ALLOC(c, b->size);
	    ASSERT(mp_sub(b->digits, b->size,
			  a->digits, a->size, c->digits) == 0);
	    c->sign = 1;
	    size = mp_rsize(c->digits, b->size);
	} else {								/* |A| = |B| */
	    c->sign = 0;
	    size = 0;
	}
    }
    c->size = size;
}

void
mpi_add_u32(const mpi *a, uint32_t b, mpi *s)
{
    /* TODO: optimize. */
    mpi_t bb;

    mpi_init_u32(bb, b);
    mpi_add(a, bb, s);
    mpi_free(bb);
}

/* FIXME: I don't think this works for A==C or B==C .. */
void
mpi_sub(const mpi *a, const mpi *b, mpi *c)
{
    /* A - A = 0 */
    if (mpi_cmp_eq(a, b)) {
	mpi_zero(c);
	return;
    }
    /* A - 0 = A */
    if (b->size == 0) {
	mpi_set_mpi(c, a);
	return;
    }
    if (b == c) {
	c->sign ^= 1;
	mpi_add(a, b, c);
    } else {
	/* here, a could = c */
	((mpi *)b)->sign ^= 1;
	mpi_add(a, b, c);
	((mpi *)b)->sign ^= 1;
    }
}

void
mpi_mul(const mpi *a, const mpi *b, mpi *c)
{
    if (a->size == 0 || b->size == 0) {
	mpi_zero(c);
	return;
    }

    if (a == b) {
	mpi_sqr(a, c);
	return;
    }

    mp_size csize = a->size + b->size;
    if (a == c || b == c) {
	mp_digit *prod = MP_TMP_ALLOC(csize);
	mp_mul(a->digits, a->size, b->digits, b->size, prod);
	csize -= (prod[csize - 1] == 0);
	MPI_SIZE(c, csize);
	mp_copy(prod, csize, c->digits);
	MP_TMP_FREE(prod);
    } else {
	ASSERT(a->digits[a->size - 1] != 0);
	ASSERT(b->digits[b->size - 1] != 0);
	MPI_MIN_ALLOC(c, csize);
	mp_mul(a->digits, a->size, b->digits, b->size, c->digits);
	c->size = csize - (c->digits[csize - 1] == 0);
    }
    c->sign = a->sign ^ b->sign;
}

void
mpi_mul_u32(const mpi *a, uint32_t b, mpi *p)
{
    if (mpi_is_zero(a) || b == 0) {
	mpi_zero(p);
	return;
    } else if (b == 1) {
	if (a != p)
	    mpi_set_mpi(p, a);
	return;
    } else if ((b & (b-1)) == 0) {	/* B is a power of 2 */
	mpi_lshift(a, __builtin_ctz(b), p);
	return;
    } else if (b == (mp_digit)b) {	/* B fits in an mp_digit */
	if (a == p) {
	    mp_digit cy = mp_dmuli(p->digits, p->size, (mp_digit)b);
	    if (cy) {
		MPI_MIN_ALLOC(p, p->size + 1);
		p->digits[p->size++] = cy;
	    }
	} else {
	    MPI_MIN_ALLOC(p, a->size);
	    mp_digit cy = mp_dmul(a->digits, a->size, (mp_digit)b, p->digits);
	    if (cy) {
		MPI_MIN_ALLOC(p, a->size + 1);
		p->digits[a->size] = cy;
		p->size = a->size + 1;
	    } else {
		p->size = a->size;
	    }
	}
    } else {
	unsigned bits = CHAR_BIT * sizeof(uint32_t) - __builtin_clz(b);
	mp_size size = (bits + MP_DIGIT_BITS - 1) / MP_DIGIT_BITS;
	mp_digit *bp = MP_TMP_ALLOC(size);
#if MP_DIGIT_BITS >= 32
	bp[0] = b;
#else
	for (mp_size j=0; j<size; j++) {
	    bp[j] = (mp_digit)b;
	    b >>= MP_DIGIT_BITS;
	}
#endif
	if (a == p) {
	    mp_digit *tmp = MP_TMP_ALLOC(p->size + size);
	    mp_mul(p->digits, p->size, bp, size, tmp);
	    MPI_MIN_ALLOC(p, p->size + size);
	    mp_copy(tmp, p->size + size, p->digits);
	    p->size = mp_rsize(p->digits, p->size + size);
	    MP_TMP_FREE(tmp);
	} else {
	    MPI_MIN_ALLOC(p, a->size + size);
	    mp_mul(a->digits, a->size, bp, size, p->digits);
	    p->size = mp_rsize(p->digits, a->size + size);
	}
	MP_TMP_FREE(bp);
    }
}

void
mpi_mul_s32(const mpi *a, int32_t b, mpi *p)
{
    if (a->size == 0 || b == 0)
	mpi_zero(p);
    else {
	if (b < 0) {
	    mpi_mul_u32(a, -b, p);
	    p->sign ^= 1;
	} else {
	    mpi_mul_u32(a, b, p);
	}
    }
}

void
mpi_mul_u64(const mpi *a, uint64_t b, mpi *p)
{
    if (mpi_is_zero(a) || b == 0) {
	mpi_zero(p);
	return;
    } else if (b == 1) {
	if (a != p)
	    mpi_set_mpi(p, a);
	return;
    } else if ((b & (b-1)) == 0) {	/* B is a power of 2 */
	mpi_lshift(a, __builtin_ctzll(b), p);
	return;
    } else if (b == (mp_digit)b) {	/* B fits in an mp_digit */
	if (a == p) {
	    mp_digit cy = mp_dmuli(p->digits, p->size, (mp_digit)b);
	    if (cy) {
		MPI_MIN_ALLOC(p, p->size + 1);
		p->digits[p->size++] = cy;
	    }
	} else {
	    MPI_MIN_ALLOC(p, a->size);
	    mp_digit cy = mp_dmul(a->digits, a->size, (mp_digit)b, p->digits);
	    if (cy) {
		MPI_MIN_ALLOC(p, a->size + 1);
		p->digits[a->size] = cy;
		p->size = a->size + 1;
	    } else {
		p->size = a->size;
	    }
	}
    } else {
	unsigned bits = CHAR_BIT * sizeof(uint64_t) - __builtin_clzll(b);
	mp_size size = (bits + MP_DIGIT_BITS - 1) / MP_DIGIT_BITS;
	mp_digit *bp = MP_TMP_ALLOC(size);
#if MP_DIGIT_BITS >= 64
	bp[0] = b;
#else
	for (mp_size j=0; j<size; j++) {
	    bp[j] = (mp_digit)b;
	    b >>= MP_DIGIT_BITS;
	}
#endif
	if (a == p) {
	    mp_digit *tmp = MP_TMP_ALLOC(p->size + size);
	    mp_mul(p->digits, p->size, bp, size, tmp);
	    MPI_MIN_ALLOC(p, p->size + size);
	    mp_copy(tmp, p->size + size, p->digits);
	    p->size = mp_rsize(p->digits, p->size + size);
	    MP_TMP_FREE(tmp);
	} else {
	    MPI_MIN_ALLOC(p, a->size + size);
	    mp_mul(a->digits, a->size, bp, size, p->digits);
	    p->size = mp_rsize(p->digits, a->size + size);
	}
	MP_TMP_FREE(bp);
    }
}

void
mpi_mul_s64(const mpi *a, int64_t b, mpi *p)
{
    if (a->size == 0 || b == 0)
	mpi_zero(p);
    else {
	if (b < 0) {
	    mpi_mul_u64(a, -b, p);
	    p->sign ^= 1;
	} else {
	    mpi_mul_u64(a, b, p);
	}
    }
}

void
mpi_sqr(const mpi *a, mpi *b)
{
    if (a->size == 0) {
	mpi_zero(b);
	return;
    }

    mp_size bsize = a->size * 2;
    if (a == b) {
	mp_digit *prod = MP_TMP_ALLOC(bsize);
	mp_sqr(a->digits, a->size, prod);
	bsize -= (prod[bsize - 1] == 0);
	MPI_SIZE(b, bsize);
	mp_copy(prod, bsize, b->digits);
	MP_TMP_FREE(prod);
    } else {
	MPI_MIN_ALLOC(b, bsize);
	mp_sqr(a->digits, a->size, b->digits);
	b->size = bsize - (b->digits[bsize - 1] == 0);
    }
    b->sign = 0;
}


void
mpi_divrem(const mpi *a, const mpi *b, mpi *q, mpi *r)
{
    ASSERT(a != q);
    ASSERT(a != r);
    ASSERT(b != q);
    ASSERT(b != r);
    ASSERT(q != r);

    ASSERT(b->size != 0);

    if (a->size == 0 || a->size < b->size) {
	mpi_zero(q);
	mpi_set_mpi(r, a);
	return;
    }
    if (a == b) {
	mpi_set_u32(q, 1);
	mpi_zero(r);
	return;
    }

    const mp_size qsize = a->size - b->size + 1;
    MPI_MIN_ALLOC(q, qsize);
    MPI_MIN_ALLOC(r, b->size);

    mp_divrem(a->digits, a->size,
	      b->digits, b->size, q->digits, r->digits);

    q->size = qsize - (q->digits[qsize - 1] == 0);
    q->sign = a->sign ^ b->sign;
    r->size = mp_rsize(r->digits, b->size);
    r->sign = 0; /* XXX */
}

void
mpi_div(const mpi *a, const mpi *b, mpi *q)
{
    mp_size qsize;

    ASSERT(b->size != 0);

    if (a->size == 0 ||
	a->size < b->size) {
	mpi_zero(q);
	return;
    }
    if (a == b) {
	mpi_set_u32(q, 1);
	return;
    }

    qsize = a->size - b->size + 1;
    if (a == q || b == q) {
	mp_digit *quot = MP_TMP_ALLOC(qsize);
	mp_div(a->digits, a->size,
	       b->digits, b->size, quot);
	qsize -= (quot[qsize - 1] == 0);
	MPI_SIZE(q, qsize);
	mp_copy(quot, qsize, q->digits);
	MP_TMP_FREE(quot);
    } else {
	MPI_MIN_ALLOC(q, qsize);
	mp_div(a->digits, a->size,
	       b->digits, b->size, q->digits);
	MP_NORMALIZE(q->digits, q->size);
    }
    q->sign = a->sign ^ b->sign;
}

void
mpi_divexact(const mpi *a, const mpi *b, mpi *q)
{
    ASSERT(b->size != 0);

    if (a->size == 0 ||
	a->size < b->size) {
	/* FIXME: raise an error here? */
	mpi_zero(q);
	return;
    }
    if (a == b) {
	mpi_set_u32(q, 1);
	return;
    }

    mp_size qsize = a->size - b->size + 1;
    if (a == q || b == q) {
	mp_digit *quot = MP_TMP_ALLOC(qsize);
	mp_divexact(a->digits, a->size,
		    b->digits, b->size, quot);
	MP_NORMALIZE(quot, qsize);
	MPI_SIZE(q, qsize);
	mp_copy(quot, qsize, q->digits);
	MP_TMP_FREE(quot);
    } else {
	MPI_MIN_ALLOC(q, qsize);
	mp_divexact(a->digits, a->size,
		    b->digits, b->size, q->digits);
	MP_NORMALIZE(q->digits, qsize);
    }
    q->size = qsize;
    q->sign = a->sign ^ b->sign;
}

/* XXX ignore sign of arguments and sets sign of result to sign of A */
void
mpi_mod(const mpi *a, const mpi *m, mpi *r)
{
    ASSERT(a != m);
    ASSERT(m != r);
    ASSERT(m->size != 0);

    /* If A = 0 or A = M, then A % M = 0 */
    if (a->size == 0 || a == m) {
	mpi_zero(r);
	return;
    }

    /* If A < M, R = 0 */
    /* FIXME: there is a bug here when A is negative. */
    if (mpi_cmp(a, m) < 0) {
	mpi_set_mpi(r, a);
	return;
    }

    if (a == r) {
	/* A = R, so compute the modulus in-place on R. */
	mp_modi(r->digits, r->size, m->digits, m->size);
    } else {
	MPI_MIN_ALLOC(r, m->size);
	mp_mod(a->digits, a->size, m->digits, m->size, r->digits);
    }
    r->size = mp_rsize(r->digits, m->size);
    r->sign = a->sign; /* XXX */
}

void
mpi_sqrt(const mpi *a, mpi *r)
{
    ASSERT(a != r);

    if (a->size == 0) {
	mpi_zero(r);
	return;
    }
    const mp_size rsize = (a->size + 1) / 2;
    MPI_MIN_ALLOC(r, rsize);
    mp_sqrt(a->digits, a->size, r->digits);
    r->size = mp_rsize(r->digits, rsize);
    r->sign = a->sign;
}

void
mpi_gcd(const mpi *a, const mpi *b, mpi *g)
{
    ASSERT(a != NULL);
    ASSERT(b != NULL);
    ASSERT(g != NULL);
    ASSERT(a != g);
    ASSERT(b != g);

    if (a == b) {					/* GCD(A,A) = A */
	mpi_set_mpi(g, a);
	g->sign = 0;
	return;
    }

    if (a->size == 0) {				/* GCD(0,B) = B */
	mpi_set_mpi(g, b);
    } else if (b->size == 0) {		/* GCD(A,0) = A */
	mpi_set_mpi(g, a);
    } else {
	const mp_size min_size = MIN(a->size, b->size);
	MPI_SIZE(g, min_size);
	mp_gcd(a->digits, a->size, b->digits, b->size, g->digits);
	MPI_NORMALIZE(g);
    }
    g->sign = 0;
}

bool
mpi_coprime(const mpi *a, const mpi *b)
{
    return mp_coprime(a->digits, a->size, b->digits, b->size);
}

void
mpi_rshift(const mpi *p, unsigned bits, mpi *q)
{
    if (!bits)
	return;

    const unsigned digits = bits / MP_DIGIT_BITS;
    bits %= MP_DIGIT_BITS;

    const unsigned pbits = mpi_significant_bits(p);
    if (pbits <= bits) {
	mpi_zero(q);
	return;
    }

    if (p == q) {
	if (digits) {
	    q->size -= digits;
	    mp_copy(q->digits + digits, q->size, q->digits);
	}
	mp_rshifti(q->digits, q->size, bits);
    } else {
	const mp_size qsize = p->size - digits;
	MPI_SIZE(q, qsize);
	mp_rshift(p->digits + digits, qsize, bits, q->digits);
    }
    q->size -= (q->digits[q->size - 1] == 0);
    ASSERT(q->size != 0);
}

void
mpi_lshift(const mpi *p, unsigned bits, mpi *q)
{
    if (bits == 0 || mpi_is_zero(p)) {
	if (bits == 0)
	    mpi_set_mpi(q, p);
	else
	    mpi_zero(q);
	return;
    }

    const unsigned digits = bits / MP_DIGIT_BITS;
    bits %= MP_DIGIT_BITS;

    mp_digit cy;
    if (p == q) {
	cy = mp_lshifti(q->digits, q->size, bits);
	if (digits != 0) {
	    MPI_MIN_ALLOC(q, q->size + digits);
	    for (int j = q->size - 1; j >= 0; j--)
		q->digits[j + digits] = q->digits[j];
	    q->size += digits;
	}
    } else {
	MPI_SIZE(q, p->size + digits);
	cy = mp_lshift(p->digits, p->size, bits, q->digits + digits);
    }

    mp_zero(q->digits, digits);
    if (cy) {
	MPI_SIZE(q, q->size + 1);
	q->digits[q->size - 1] = cy;
    }
}

/* < 0 means right shift, > 0 means left shift */
void
mpi_shift(const mpi *p, int bits, mpi *q)
{
    if (bits < 0)
	mpi_rshift(p, -bits, q);
    else if (bits > 0)
	mpi_lshift(p, +bits, q);
}

void
mpi_fprint(const mpi *n, unsigned base, FILE *fp)
{
    if (!fp)
	fp = stdout;
    if (n->size == 0) {
	fputc('0', fp);
	return;
    }
    if (n->sign)
	fputc('-', fp);
    mp_fprint(n->digits, n->size, base, fp);
}
