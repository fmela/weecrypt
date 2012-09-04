/* unit_tests.c
 * Copyright (C) 2012 Farooq Mela. All rights reserved. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <time.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "weecrypt.h"

#define TEST_FUNC(func) { #func, func }

void test_mp_digit_mul();
void test_mp_digit_div();
void test_mp_digit_invert();
void test_mp_digit_log2();
void test_mp_digit_gcd();

CU_TestInfo mp_digit_tests[] = {
    TEST_FUNC(test_mp_digit_mul),
    TEST_FUNC(test_mp_digit_div),
    TEST_FUNC(test_mp_digit_invert),
    TEST_FUNC(test_mp_digit_log2),
    TEST_FUNC(test_mp_digit_gcd),
    CU_TEST_INFO_NULL
};

void test_mp_from_to_str();

CU_TestInfo mp_conversion_tests[] = {
    TEST_FUNC(test_mp_from_to_str),
    CU_TEST_INFO_NULL
};

void test_mp_add_n();
void test_mp_sub_n();
void test_mp_mul();
void test_mp_mul_bug();
void test_mp_div();
void test_mp_lshift();
void test_mp_rshift();
void test_mp_sieve();
void test_mp_gcd_bug();

CU_TestInfo mp_basic_tests[] = {
    TEST_FUNC(test_mp_add_n),
    TEST_FUNC(test_mp_sub_n),
    TEST_FUNC(test_mp_mul),
    TEST_FUNC(test_mp_mul_bug),
    TEST_FUNC(test_mp_div),
    TEST_FUNC(test_mp_lshift),
    TEST_FUNC(test_mp_rshift),
    TEST_FUNC(test_mp_sieve),
    TEST_FUNC(test_mp_gcd_bug),
    CU_TEST_INFO_NULL
};

void test_mpi_rand();
void test_mpi_cmp();
void test_mpi_fibonacci();
void test_mpi_factorial();
void test_mpi_binomial();

CU_TestInfo mpi_basic_tests[] = {
    TEST_FUNC(test_mpi_rand),
    TEST_FUNC(test_mpi_cmp),
    TEST_FUNC(test_mpi_fibonacci),
    TEST_FUNC(test_mpi_factorial),
    TEST_FUNC(test_mpi_binomial),
    CU_TEST_INFO_NULL
};

void test_base64_encode();
void test_base64_decode();
void test_base64_encode_decode();

CU_TestInfo base64_tests[] = {
    TEST_FUNC(test_base64_encode),
    TEST_FUNC(test_base64_decode),
    TEST_FUNC(test_base64_encode_decode),
    CU_TEST_INFO_NULL
};

void test_mt64_init_u64();
void test_mt64_init_u64_array();

CU_TestInfo mt64_tests[] = {
    TEST_FUNC(test_mt64_init_u64),
    TEST_FUNC(test_mt64_init_u64_array),
    CU_TEST_INFO_NULL
};

#define TEST_SUITE(suite) { #suite, NULL, NULL, suite }

CU_SuiteInfo test_suites[] = {
    TEST_SUITE(mp_digit_tests),
    TEST_SUITE(mp_conversion_tests),
    TEST_SUITE(mp_basic_tests),
    TEST_SUITE(mpi_basic_tests),
    TEST_SUITE(base64_tests),
    TEST_SUITE(mt64_tests),
    CU_SUITE_INFO_NULL
};

int
main(void)
{
    CU_initialize_registry();
    CU_register_suites(test_suites);
    CU_basic_set_mode(CU_BRM_NORMAL);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return 0;
}

void test_mp_digit_mul()
{
    mp_digit hi, lo;
    _mp_digit_mul(0, 0, &hi, &lo);
    CU_ASSERT_EQUAL(hi, 0);
    CU_ASSERT_EQUAL(lo, 0);
    _mp_digit_mul(MP_DIGIT_MAX, MP_DIGIT_MAX, &hi, &lo);
    CU_ASSERT_EQUAL(hi, MP_DIGIT_MAX - 1);
    CU_ASSERT_EQUAL(lo, 1);
#if MP_DIGIT_SIZE == 4
    _mp_digit_mul(4000000000U, 4000000000U, &hi, &lo);
    CU_ASSERT_EQUAL(hi, 3725290298);
    CU_ASSERT_EQUAL(lo, 1983905792);
#elif MP_DIGIT_SIZE == 8
    _mp_digit_mul(16000000000000000000ULL, 16000000000000000000ULL, &hi, &lo);
    CU_ASSERT_EQUAL(hi, 13877787807814456755ULL);
    CU_ASSERT_EQUAL(lo, 5449091666327633920ULL);
#endif
}

void test_mp_digit_div()
{
#if MP_DIGIT_SIZE >= 4
    mp_digit q, r;
    _mp_digit_div(47, 129416588, 1000000000, &q, &r);
#if MP_DIGIT_SIZE == 4
    CU_ASSERT_EQUAL(q, 201U);
    CU_ASSERT_EQUAL(r, 992879500U);
#elif MP_DIGIT_SIZE == 8
    CU_ASSERT_EQUAL(q, 866996971464ULL);
    CU_ASSERT_EQUAL(r, 478342540ULL);
#endif
#endif
}

void test_mp_digit_invert()
{
    for (int i = 0; i < 10000; ++i) {
	mp_digit r;
	mp_rand(&r, 1);
	r |= 1; /* Ensure r is odd. */
	mp_digit inverse = mp_digit_invert(r);
	CU_ASSERT_EQUAL((mp_digit)(r * inverse), (mp_digit)1);
    }
}

void test_mp_digit_log2()
{
    for (unsigned i = 0; i < 10000; ++i) {
	mp_digit r;
	mp_rand(&r, 1);
	r |= ((mp_digit)1) << (MP_DIGIT_BITS - 1); /* Ensure MSB is set. */
	unsigned i_log2 = i % MP_DIGIT_BITS;
	r >>= i_log2;
	CU_ASSERT_EQUAL(mp_digit_log2(r), MP_DIGIT_BITS - 1 - i_log2);
    }
}

static mp_digit digit_gcd(mp_digit u, mp_digit v)
{
    if (u < v)
	SWAP(u, v, mp_digit);
    /* Now u >= v. */
    if (v == 0)
	return u;
    for (;;) {
	mp_digit r = u % v;
	if (!r)
	    return v;
	u = v;
	v = r;
    }
}

void test_mp_digit_gcd()
{
    CU_ASSERT_EQUAL(mp_digit_gcd(100, 0), 100);
    CU_ASSERT_EQUAL(mp_digit_gcd(0, 100), 100);
    CU_ASSERT_EQUAL(mp_digit_gcd(0, 0), 0);
    for (unsigned i = 0; i < 10000; ++i) {
	mp_digit u[2];
	mp_rand(u, 2);
	CU_ASSERT_EQUAL(mp_digit_gcd(u[0], u[1]), digit_gcd(u[0], u[1]));
    }
}

void
test_mp_from_to_str()
{
    const char dec_str[] = "1298129834734872981982091298237348734982039483457692874102039898435387465198723";
    const char hex_str[] = "B35FB72F9E4F53B59C9A38C1B32095C503DB4B487ED9E0DD10845F9D3C97CEC83";

    mp_size digits = mp_string_digits(dec_str, 10);
    mp_size rdigits;
    mp_digit *m = mp_from_str(dec_str, 10, &rdigits);
    CU_ASSERT_PTR_NOT_NULL(m);
    CU_ASSERT(rdigits <= digits);

    size_t str_size = mp_string_size(digits, 16);
    CU_ASSERT(strlen(hex_str) <= str_size);
    char *str = mp_get_str(m, digits, 16, NULL);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str, hex_str);
    free(m);
}

void
test_mp_add_n()
{
    const mp_size N = 100;
    mp_digit A[N], B[N], C[N];

    /* Initialize summands to zero value. */
    for (mp_size i = 0; i < N; ++i) {
	A[i] = B[i] = 0;
    }

    for (mp_size i = 0; i < N; ++i) {
	mp_digit carry = mp_add_n(A, B, i, C);
	CU_ASSERT_EQUAL(carry, 0);
	for (mp_size j = 0; j < i; ++j) {
	    CU_ASSERT_EQUAL(C[j], 0);
	}
    }

    /* Initialize summands to maximum value. */
    for (mp_size i = 0; i < N; ++i) {
	A[i] = B[i] = MP_DIGIT_MAX;
    }

    for (mp_size i = 0; i < N; ++i) {
	mp_digit carry = mp_add_n(A, B, i, C);
	if (i == 0) {
	    CU_ASSERT_EQUAL(carry, 0);
	} else {
	    CU_ASSERT_EQUAL(carry, 1);
	}
	for (mp_size j = 0; j < i; ++j) {
	    if (j == 0) {
		CU_ASSERT_EQUAL(C[j], MP_DIGIT_MAX - 1);
	    } else {
		CU_ASSERT_EQUAL(C[j], MP_DIGIT_MAX);
	    }
	}
    }

    /* Initialize A to random, B to maximum value minus A. */
    mp_rand(A, N);
    for (mp_size i = 0; i < N; ++i) {
	B[i] = MP_DIGIT_MAX - A[i];
    }
    for (mp_size i = 0; i < N; ++i) {
	mp_digit carry = mp_add_n(A, B, i, C);
	CU_ASSERT_EQUAL(carry, 0);
	for (mp_size j = 0; j < i; ++j) {
	    CU_ASSERT_EQUAL(C[j], MP_DIGIT_MAX);
	}
    }
    /* Now increment B by one, causing the addition to overflow. */
    CU_ASSERT_EQUAL(mp_inc(B, N), 0);
    for (mp_size i = 0; i < N; ++i) {
	mp_digit carry = mp_add_n(A, B, i, C);
	if (i == 0) {
	    CU_ASSERT_EQUAL(carry, 0);
	} else {
	    CU_ASSERT_EQUAL(carry, 1);
	}
	for (mp_size j = 0; j < i; ++j) {
	    CU_ASSERT_EQUAL(C[j], 0);
	}
    }
}

void test_mp_sub_n()
{
    const mp_size N = 100;
    mp_digit A[N], B[N], C[N];

    /* Initialize to zero value. */
    for (mp_size i = 0; i < N; ++i) {
	A[i] = B[i] = 0;
    }

    for (mp_size i = 0; i < N; ++i) {
	mp_digit carry = mp_sub_n(A, B, i, C);
	CU_ASSERT_EQUAL(carry, 0);
	for (mp_size j = 0; j < i; ++j) {
	    CU_ASSERT_EQUAL(C[j], 0);
	}
    }

    /* Initialize to maximum value. */
    for (mp_size i = 0; i < N; ++i) {
	A[i] = B[i] = MP_DIGIT_MAX;
    }

    for (mp_size i = 0; i < N; ++i) {
	mp_digit borrow = mp_sub_n(A, B, i, C);
	CU_ASSERT_EQUAL(borrow, 0);
	for (mp_size j = 0; j < i; ++j) {
	    CU_ASSERT_EQUAL(C[j], 0);
	}
    }

    /* Initialize A to maximum value, B to zero. */
    for (mp_size i = 0; i < N; ++i) {
	A[i] = MP_DIGIT_MAX;
	B[i] = 0;
    }

    for (mp_size i = 0; i < N; ++i) {
	mp_digit borrow = mp_sub_n(A, B, i, C);
	CU_ASSERT_EQUAL(borrow, 0);
	for (mp_size j = 0; j < i; ++j) {
	    CU_ASSERT_EQUAL(C[j], MP_DIGIT_MAX);
	}
    }

    /* Initialize A to zero, B to maximum value. */
    for (mp_size i = 0; i < N; ++i) {
	A[i] = 0;
	B[i] = MP_DIGIT_MAX;
    }

    for (mp_size i = 0; i < N; ++i) {
	mp_digit borrow = mp_sub_n(A, B, i, C);
	if (i > 0) {
	    CU_ASSERT_EQUAL(borrow, 1);
	}
	for (mp_size j = 0; j < i; ++j) {
	    if (j == 0) {
		CU_ASSERT_EQUAL(C[j], 1);
	    } else {
		CU_ASSERT_EQUAL(C[j], 0);
	    }
	}
    }
}

void test_mp_mul()
{
    const mp_size N = 100;
    mp_digit A[N], B[N], C[N*2];

    /* Initialize to zero value. */
    for (mp_size i = 0; i < N; ++i) {
	A[i] = B[i] = 0;
    }

    for (mp_size i = 0; i < N; ++i) {
	mp_mul(A, i, B, i, C);
	for (mp_size j = 0; j < i*2; ++j) {
	    CU_ASSERT_EQUAL(C[j], 0);
	}
    }

    /* Initialize to maximum value. */
    for (mp_size i = 0; i < N; ++i) {
	A[i] = B[i] = MP_DIGIT_MAX;
    }

    for (mp_size i = 0; i < N; ++i) {
	mp_mul(A, i, B, i, C);
	for (mp_size j = 0; j < i*2; ++j) {
	    if (j == 0) {
		CU_ASSERT_EQUAL(C[j], 1);
	    } else if (j < i) {
		CU_ASSERT_EQUAL(C[j], 0);
	    } else if (j == i) {
		CU_ASSERT_EQUAL(C[j], MP_DIGIT_MAX - 1);
	    } else {
		CU_ASSERT_EQUAL(C[j], MP_DIGIT_MAX);
	    }
	}
    }
}

void test_mp_mul_bug()
{
    mp_size usize, vsize, epsize;
    mp_digit *u = mp_from_str("131811807990", 10, &usize);
    CU_ASSERT_PTR_NOT_NULL(u);
    CU_ASSERT_TRUE(usize > 0);

    mp_digit *v = mp_from_str("5565870955", 10, &vsize);
    CU_ASSERT_PTR_NOT_NULL(v);
    CU_ASSERT_TRUE(vsize > 0);

    mp_digit *ep =
	mp_from_str("733647513617577930450", 10, &epsize);
    CU_ASSERT_PTR_NOT_NULL(ep);
    CU_ASSERT_TRUE(epsize > 0);

    mp_digit *p = mp_new(usize + vsize);
    CU_ASSERT_PTR_NOT_NULL(p);
    mp_mul(u, usize, v, vsize, p);
    CU_ASSERT_TRUE(mp_cmp_eq(p, usize + vsize, ep, epsize));

    mp_free(u);
    mp_free(v);
    mp_free(ep);
    mp_free(p);
}

void test_mp_div()
{
    mp_size usize, vsize, eqsize, ersize;
    mp_digit *u =
	mp_from_str("2165485635546218891283495873487923820391912395483248234873"
		    "45754984536351684", 10, &usize);
    CU_ASSERT_PTR_NOT_NULL(u);
    CU_ASSERT_TRUE(usize > 0);

    mp_digit *v =
	mp_from_str("189182384738298326889351584863126568874876532656898765",
		    10, &vsize);
    CU_ASSERT_PTR_NOT_NULL(v);
    CU_ASSERT_TRUE(vsize > 0);

    mp_digit *eq =
	mp_from_str("1144655005032207512764", 10, &eqsize);
    CU_ASSERT_PTR_NOT_NULL(eq);
    CU_ASSERT_TRUE(eqsize > 0);

    mp_digit *er =
	mp_from_str("144512367603679442923819525574593307305800178143015224",
		    10, &ersize);
    CU_ASSERT_PTR_NOT_NULL(er);
    CU_ASSERT_TRUE(ersize > 0);

    mp_digit *q = mp_new(usize - vsize + 1);
    CU_ASSERT_PTR_NOT_NULL(q);
    mp_digit *r = mp_new(vsize);
    CU_ASSERT_PTR_NOT_NULL(r);
    mp_divrem(u, usize, v, vsize, q, r);
    CU_ASSERT_TRUE(mp_cmp_eq(q, usize - vsize + 1, eq, eqsize));
    CU_ASSERT_TRUE(mp_cmp_eq(r, vsize, er, ersize));
}

void test_mp_lshift()
{
}

void test_mp_rshift()
{
}

static mp_digit smallest_factor(mp_digit p) {
    if ((p & 1) == 0) {
	if (p == 2)
	    return 0;
	return 2;
    }
    for (mp_digit factor = 3; ; factor += 2) {
	if (factor * factor >= p) {
	    if (factor * factor == p)
		return factor;
	    return 0;
	}
	if (p % factor == 0)
	    return factor;
    }
}

void test_mp_sieve()
{
#if MP_DIGIT_SIZE == 1
    const unsigned max_n = 254;
#elif MP_DIGIT_SIZE == 2
    const unsigned max_n = 65000;
#else
    const unsigned max_n = 100000;
#endif
    mp_digit u;

    u = 0;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 0);
    u = 1;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 0);
    u = 2;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 0);
    u = 3;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 0);
    u = 5;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 0);
    u = 7;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 0);
    u = 11;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 0);
    u = 13;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 0);

    u = 4;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 2);
    u = 9;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 3);
    u = 16;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 2);
    u = 25;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 5);
    u = 36;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 2);
    u = 49;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 7);
    u = 121;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 11);
    u = 143;
    CU_ASSERT_EQUAL(mp_sieve(&u, 1), 11);

    for (u = 2; u <= max_n; ++u) {
	mp_digit v = u;
	mp_digit factor;
	while ((factor = mp_sieve(&v, 1)) != 0) {
	    CU_ASSERT_TRUE(factor >= 2);
	    CU_ASSERT_TRUE(v % factor == 0);
	    CU_ASSERT_EQUAL(factor, smallest_factor(v));
	    v /= factor;
	}
	CU_ASSERT_TRUE(smallest_factor(v) == 0);
    }
}

void test_mpi_rand() {
    mpi_t t;

    mpi_init(t);
    for (unsigned bits = 0; bits <= 1024; ++bits) {
	mpi_rand(t, bits);
	CU_ASSERT_EQUAL(mpi_significant_bits(t), bits);
    }
    mpi_free(t);
}

void test_mpi_cmp() {
    mpi_t u, v;

    mpi_init(u);
    mpi_init(v);
}

void test_mpi_fibonacci() {
    mpi_t u;

    mpi_init(u);

    mpi_fibonacci(0, u);
    CU_ASSERT_TRUE(mpi_is_zero(u));

    mpi_fibonacci(1, u);
    CU_ASSERT_TRUE(mpi_is_one(u));

    mpi_fibonacci(2, u);
    CU_ASSERT_TRUE(mpi_is_one(u));

    mpi_fibonacci(3, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 2) == 0);

    mpi_fibonacci(4, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 3) == 0);

    mpi_fibonacci(5, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 5) == 0);

    mpi_fibonacci(36, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 14930352) == 0);

    mpi_fibonacci(37, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 24157817) == 0);

    mpi_fibonacci(38, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 39088169) == 0);

    mpi_fibonacci(84, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(160500643816367088)) == 0);

    mpi_fibonacci(85, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(259695496911122585)) == 0);

    mpi_fibonacci(86, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(420196140727489673)) == 0);

    mpi_fibonacci(87, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(679891637638612258)) == 0);

    mpi_fibonacci(88, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(1100087778366101931)) == 0);

    mpi_fibonacci(89, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(1779979416004714189)) == 0);

    mpi_fibonacci(90, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(2880067194370816120)) == 0);

    mpi_fibonacci(91, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(4660046610375530309)) == 0);

    mpi_fibonacci(92, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(7540113804746346429)) == 0);

    char *str;
    mpi_fibonacci(100, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str, "354224848179261915075");
    free(str);

    mpi_fibonacci(175, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str, "1672445759041379840132227567949787325");
    free(str);

    mpi_fibonacci(333, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"175145587744443809540894028220838354911578178491208578950667797112537"
	"8");
    free(str);

    mpi_fibonacci(517, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"49788037395300812810131073986569908831619942913932907263973181610030"
	"1319690449020710377940634148387209607097");
    free(str);

    mpi_fibonacci(1000, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"43466557686937456435688527675040625802564660517371780402481729089536"
	"55541794905189040387984007925516929592259308032263477520968962323987"
	"33224711616429964409065331879382989696499285160037044761377951668492"
	"28875");
    free(str);

    mpi_fibonacci(2000, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"42246963333923048787067256023414827825798528402506810980102801373143"
	"08584370130707224123599639141511088446087538909603607640194711643596"
	"02927198331259873732625355580260699158591522949245390499872225679531"
	"69828744824729922639018337167780606070116154978867198798583114688708"
	"76264597369086722884023654422295243347964480139515349562972087652656"
	"06952980649984197744872015561280266540455417171788193032402520431208"
	"2516817125");
    free(str);

    mpi_fibonacci(3333, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"16081255460093030137520921726464547314678822673624885373824153483852"
	"08628672939517805286498219192576157095660455066069931191758021851248"
	"22488138771112394657758608333362364685060143859499917562034725977852"
	"04185967343731601096512811529527591977560692167217276780029708731519"
	"53909325673223364865532178711127303150321674121179072036629669724160"
	"13495121319476886150431134710920563192032639897674539031040226207564"
	"71488121330454007305564788034488918758638978710173483027002237738780"
	"66406591138798193375407310370057996152084870780871436707799925597506"
	"95780820340613920741679037453595465730010531432914505654314494108795"
	"29319440686555701709467375710653511452992648701281809864666401772826"
	"55314724005133378");
    free(str);

    mpi_fibonacci(7777, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"88591738025179441813883141681374872406686676360172518113096774213688"
	"22164949842641923995439360511625720108360264784180196507096254762973"
	"44063243198381844255406991780651394969331567189208152127002425946427"
	"98954303650289845362495424053558736761673931375520959483295372949269"
	"10410568590834778515655145353493776290909800114992347496916662676112"
	"32144656987286696738808878388391365156028214492372934341199320475105"
	"68624270962487875092772756442201701974549899226971652625907461847654"
	"05837055894766412045341022897072614542255173559907118847828590351585"
	"84002627527983599585913495786867285900749909368799912211895177762267"
	"21953948432225472866982427028233890645608625875345256990804802095790"
	"07994125261694279896390365346055711421685210305859780906992242307721"
	"47642166158049549864338570585692701141892854987366399910447792376655"
	"64370786219220952640205380160975255912506304388835640413377112652221"
	"10903098919929933505294624705209887192659876132986466911023891352155"
	"91898718266172603736617994205472305661677168828414638753247940894416"
	"25919172410087707622825009774234234252439835906441785135730907797247"
	"83351670100125724035563643959885567598665596968923767070730063143972"
	"84107537980951179629253260929594705411524298486335292051732048133789"
	"99698605388080762672458124906480895824529608571534907129384237632146"
	"19845396822025525624718045690629421055407733561890610220682907788050"
	"55469409800815279642612902360965705896859062089217791520899867949457"
	"02538252311668052154591541614821256884356326769909189234520532065491"
	"61404522646686050825324512251312312589660249411401400492235227426660"
	"1794630271782095422298924708557612162562807261703877769979457");
    free(str);

    mpi_free(u);
}

void test_mpi_factorial()
{
    mpi_t u;

    mpi_init(u);

    mpi_factorial(0, u);
    CU_ASSERT_TRUE(mpi_is_zero(u));

    mpi_factorial(1, u);
    CU_ASSERT_TRUE(mpi_is_one(u));

    mpi_factorial(2, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 2) == 0);

    mpi_factorial(3, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 6) == 0);

    mpi_factorial(4, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 24) == 0);

    mpi_factorial(5, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 120) == 0);

    mpi_factorial(6, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 720) == 0);

    mpi_factorial(7, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 5040) == 0);

    mpi_factorial(8, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 40320) == 0);

    mpi_factorial(9, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 362880) == 0);

    mpi_factorial(10, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 3628800) == 0);

    mpi_factorial(12, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 479001600) == 0);

    mpi_factorial(14, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(87178291200)) == 0);

    mpi_factorial(16, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(20922789888000)) == 0);

    mpi_factorial(18, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(6402373705728000)) == 0);

    mpi_factorial(20, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(2432902008176640000)) == 0);

    char *str;
    mpi_factorial(100, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"93326215443944152681699238856266700490715968264381621468592963895217"
	"59999322991560894146397615651828625369792082722375825118521091686400"
	"0000000000000000000000");
    free(str);

    mpi_factorial(200, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"78865786736479050355236321393218506229513597768717326329474253324435"
	"94499634033429203042840119846239041772121389196388302576427902426371"
	"05061926624952829931113462857270763317237396988943922445621451664240"
	"25403329186413122742829485327752424240757390324032125740557956866022"
	"60319041703240623517008587961789222227896237038973747200000000000000"
	"00000000000000000000000000000000000");
    free(str);

    mpi_factorial(300, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"30605751221644063603537046129726862938858880417357699941677674125947"
	"65331767168674655152914224775733499391478887017263688642639077590031"
	"54226842927906974559841225476930271954604008012215776252176854255965"
	"35690350678872526432189626429936520457644883038890975394348962543605"
	"32259807765212708224376394491201286786753683057122936819436499564604"
	"98166450227716500185176546469340112226034729724066333258583506870150"
	"16979416885035375213755491028912640715715483028228493795263658014523"
	"52331569364822334367992545940952768206080622328123873838808170496000"
	"00000000000000000000000000000000000000000000000000000000000000000000"
	"000");
    free(str);

    mpi_free(u);
}

void test_mpi_binomial()
{
    mpi_t u;

    mpi_init(u);

    mpi_binomial(1, 0, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(2, 0, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(2, 1, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 2) == 0);

    mpi_binomial(2, 2, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(3, 0, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(3, 1, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 3) == 0);

    mpi_binomial(3, 2, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 3) == 0);

    mpi_binomial(3, 3, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(4, 0, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(4, 1, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 4) == 0);

    mpi_binomial(4, 2, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 6) == 0);

    mpi_binomial(4, 3, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 4) == 0);

    mpi_binomial(4, 4, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(5, 0, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(5, 1, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 5) == 0);

    mpi_binomial(5, 2, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 10) == 0);

    mpi_binomial(5, 3, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 10) == 0);

    mpi_binomial(5, 4, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 5) == 0);

    mpi_binomial(5, 5, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(6, 0, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(6, 1, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 6) == 0);

    mpi_binomial(6, 2, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 15) == 0);

    mpi_binomial(6, 3, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 20) == 0);

    mpi_binomial(6, 4, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 15) == 0);

    mpi_binomial(6, 5, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 6) == 0);

    mpi_binomial(6, 6, u);
    CU_ASSERT_TRUE(mpi_cmp_u32(u, 1) == 0);

    mpi_binomial(50, 20, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(47129212243960)) == 0);

    mpi_binomial(50, 25, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(126410606437752)) == 0);

    mpi_binomial(60, 30, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(118264581564861424)) == 0);

    mpi_binomial(64, 32, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(1832624140942590534)) == 0);

    mpi_binomial(66, 33, u);
    CU_ASSERT_TRUE(mpi_cmp_u64(u, UINT64_C(7219428434016265740)) == 0);

    char *str;
    mpi_binomial(777, 333, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"80579854222698574207949367924478929918542861313197359202755168769617"
	"25182346602911846185391991696099503349189293284521928329099678939968"
	"12356695506928949274135260924783105459116823641559733537859484799527"
	"8384653319929753171120000");
    free(str);

    mpi_binomial(800, 400, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"18804244186835312700958607615195351332156581822914058344448099146747"
	"40467605503830469440334249701204699685595199584721583932629679799448"
	"91749546979275254641795355100303638201097639806192817763278612475325"
	"547257889054431744718116643407041640");
    free(str);

    mpi_binomial(8000, 4000, u);
    str = mpi_to_str(u, 10);
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str,
	"15500539285098807098553823815373989425935694551957309333656157668519"
	"92062544420544103955918264377395637097084598281775289330733983771265"
	"14851708409915051900486003823740416798323515396321687177366467000118"
	"44419542829309681549489395747557927689202010000456521758181433437143"
	"80460383694171636094440529066047925622076748751116147985613015866415"
	"45665571689415271615912645657092657518330048933324553294337819737989"
	"77720762238513091465728252641802619573485042778591831254947665832095"
	"76123979845446725608809450700726926914981420502735663404522864153603"
	"28557903543360847098743641772244951227832921049655580891715730152515"
	"21195447147918599301480939451809289307384475275543807739881761339490"
	"52116274996023188097515958667402407390034415493233886960307684874353"
	"49671245163081836318366710027275790069679959028140169234341958393990"
	"18995666989297069101573405247431581984561166504514312539918526030021"
	"90129593278340624267518292071897150715974217017810151213601254895599"
	"76073098506716090978776623597863283895379946728819002816247270320390"
	"46436955057640158278809470104463615296180188201424134618479231275852"
	"10746957466130435979605759522424770950486249740157098382102316804041"
	"56743167898442723680431020918408644769682009719163016825121361566749"
	"60969956621715995365837822655754441432766912133458087044329762650730"
	"13211724767374410897713049517598165269748996656083088572971308446123"
	"28048158183688460841810358675468966072419992914391745390423176762239"
	"15240427312631616697862137772447801225570867246272637231016486333713"
	"26026643396968372997571401800074575660647495336650508398955803913150"
	"17272669436072315832778855751327805404604883696082510773723091920301"
	"63418282678954710596318008981176618769985322925408055298716152301469"
	"61853635864142214714655200402836227812364715460217340637808409360003"
	"48910105036022998972334924226566232038133329638279026972914714812709"
	"58417331053289795749870555930308896051853740764947431872552777931038"
	"54926052287244168376327129690560474663312395094753532970935873624575"
	"05647546031399144510101774834456608462525301850567353999722732737771"
	"40151363817698572684924341841953947776753020319882281059241500255724"
	"70539647437650297348382705749368005824809080499182921056627051262644"
	"08838171934530995544635665146374450295088613407465831949101645145945"
	"63942603223821730300641642793166016168402929976476443208549733057621"
	"63183270432926727105560764608359224730650287878493707964981102557320"
	"774912091734170718784350784");

    mpi_free(u);
}

void test_mp_gcd_bug()
{
    mp_size usize;
    mp_digit *u = mp_from_str("1212649829976295580876071451640", 10, &usize);
    CU_ASSERT_PTR_NOT_NULL(u);
    CU_ASSERT(usize > 0);

    mp_size vsize;
    mp_digit *v = mp_from_str( "739557989059675857117084097688", 10, &vsize);
    CU_ASSERT_PTR_NOT_NULL(v);
    CU_ASSERT(vsize > 0);

    mp_size gsize = MIN(usize, vsize);
    mp_digit *g = mp_new(gsize);
    mp_gcd(u, usize, v, vsize, g);
    MP_NORMALIZE(g, gsize);
    CU_ASSERT_EQUAL(gsize, 1);
    CU_ASSERT_EQUAL(g[0], 8);

    mp_free(u);
    mp_free(v);
    mp_free(g);
}

void test_base64_encode()
{
    char *base64;

    base64 = base64_encode_string("A");
    CU_ASSERT_PTR_NOT_NULL(base64);
    CU_ASSERT_STRING_EQUAL(base64, "QQ==");
    free(base64);

    base64 = base64_encode_string("AS");
    CU_ASSERT_PTR_NOT_NULL(base64);
    CU_ASSERT_STRING_EQUAL(base64, "QVM=");
    free(base64);

    base64 = base64_encode_string("ASD");
    CU_ASSERT_PTR_NOT_NULL(base64);
    CU_ASSERT_STRING_EQUAL(base64, "QVNE");
    free(base64);

    base64 = base64_encode_string("ASDF");
    CU_ASSERT_PTR_NOT_NULL(base64);
    CU_ASSERT_STRING_EQUAL(base64, "QVNERg==");
    free(base64);
}

void test_base64_decode()
{
    char *original;
    unsigned output_size;

    original = base64_decode("QQ==", &output_size);
    CU_ASSERT_EQUAL(output_size, 1);
    CU_ASSERT_PTR_NOT_NULL(original);
    CU_ASSERT_EQUAL(original[0], 'A');
    free(original);

    original = base64_decode("QVM=", &output_size);
    CU_ASSERT_EQUAL(output_size, 2);
    CU_ASSERT_PTR_NOT_NULL(original);
    CU_ASSERT_EQUAL(original[0], 'A');
    CU_ASSERT_EQUAL(original[1], 'S');
    free(original);

    original = base64_decode("QVNE", &output_size);
    CU_ASSERT_EQUAL(output_size, 3);
    CU_ASSERT_PTR_NOT_NULL(original);
    CU_ASSERT_EQUAL(original[0], 'A');
    CU_ASSERT_EQUAL(original[1], 'S');
    CU_ASSERT_EQUAL(original[2], 'D');
    free(original);

    original = base64_decode("QVNERg==", &output_size);
    CU_ASSERT_EQUAL(output_size, 4);
    CU_ASSERT_PTR_NOT_NULL(original);
    CU_ASSERT_EQUAL(original[0], 'A');
    CU_ASSERT_EQUAL(original[1], 'S');
    CU_ASSERT_EQUAL(original[2], 'D');
    CU_ASSERT_EQUAL(original[3], 'F');
    free(original);
}

void test_base64_encode_decode()
{
    const unsigned char bytes[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
	0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
	0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
	0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
	0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
	0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb,
	0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,
	0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
	0xfc, 0xfd, 0xfe, 0xff,
    };
    for (unsigned i=1; i<=sizeof(bytes); i++) {
	char *encode = base64_encode(bytes, i);
	CU_ASSERT_PTR_NOT_NULL(encode);

	unsigned decode_len;
	char *decode = base64_decode(encode, &decode_len);
	CU_ASSERT_PTR_NOT_NULL(decode);
	CU_ASSERT_EQUAL(decode_len, i);
	CU_ASSERT_TRUE(memcmp(bytes, decode, i) == 0);

	free(encode);
	free(decode);
    }
}

void test_mt64_init_u64()
{
    mt64_context ctx;
    mt64_init_u64(&ctx, UINT64_C(5489));
    for (int i = 0; i < 950; ++i)
	mt64_gen_u64(&ctx);
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12518650711486048524));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4818749123633553555));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2958342177660858065));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3745796037585186959));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9797933292464336113));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15543349084849553232));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  486551758244958240));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10593020500469347495));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6369600516659338451));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9007750655643717510));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17785627354791621201));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14764033308265707661));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18034000857235843178));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16557096199299614580));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2437200641143386748));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10182421626449994068));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(   75625115684404924));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6127271793307871498));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11214820889151820355));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12416802602799347959));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1548125927064443941));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10690183998159090903));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10911454061780779591));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17091566498439379262));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13151991451832194121));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11968421298731076421));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7281465657596491640));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16882855184145485567));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16502249339396824566));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3454458562438881671));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5912850829099334963));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5330146666763391722));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8451916490946969729));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6215268879167456629));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9983414677725534452));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1533824374095340090));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11273301590638124495));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  888171466822353933));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12600073355339343855));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10435556484299784260));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4932808928766631330));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15166592873301253506));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6396823673282492139));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14209064470829046875));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6577114328647476307));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1197590027279852334));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17587678522946712038));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15126029715399578860));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(   57675930565383847));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10193180073869439881));
}

void test_mt64_init_u64_array()
{
    mt64_context ctx;
    const uint64_t init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    mt64_init_u64_array(&ctx, init, 4);
    for (int i = 0; i < 950; ++i)
	mt64_gen_u64(&ctx);
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12920301578340189344));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15368071871011048915));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1610400750626363239));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11994736084146033126));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6042574085746186088));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4154587549267685807));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15915752367312946034));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1191196620621769193));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  467437822242538360));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2836463788873877488));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10476401302029164984));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1716169985450737419));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5327734953288310341));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3994170067185955262));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  884431883768190063));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11019001754831208284));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14322807384384895215));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  161011537360955545));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1466223959660131656));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5227048585229497539));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12410731857504225031));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2142243279080761103));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17682826799106851430));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1792612570704179953));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14727410295243056025));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1459567192481221274));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5669760721687603135));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17507918443756456845));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10354471145847018200));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10362475129248202288));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13143844410150939443));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6861184673150072028));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18396524361124732580));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  543906666394301875));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12476817828199026728));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11853496871128122868));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12747674713108891748));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7986179867749890282));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9158195177777627533));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2217320706811118570));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8631389005200569973));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5538133061362648855));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3369942850878700758));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7813559982698427184));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  509051590411815948));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10197035660403006684));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13004818533162292132));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9831652587047067687));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7619315254749630976));
    CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  994412663058993407));
}
