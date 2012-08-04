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

CU_TestInfo mp_basic_tests[] = {
	TEST_FUNC(test_mp_add_n),
	TEST_FUNC(test_mp_sub_n),
	TEST_FUNC(test_mp_mul),
	TEST_FUNC(test_mp_mul_bug),
	TEST_FUNC(test_mp_div),
	TEST_FUNC(test_mp_lshift),
	TEST_FUNC(test_mp_rshift),
	TEST_FUNC(test_mp_sieve),
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
	CU_basic_set_mode(CU_BRM_VERBOSE);
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
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14514284786278117030));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4620546740167642908));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13109570281517897720));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17462938647148434322));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  355488278567739596));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7469126240319926998));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4635995468481642529));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  418970542659199878));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9604170989252516556));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6358044926049913402));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5058016125798318033));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10349215569089701407));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2583272014892537200));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10032373690199166667));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9627645531742285868));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15810285301089087632));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9219209713614924562));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7736011505917826031));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13729552270962724157));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4596340717661012313));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4413874586873285858));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5904155143473820934));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16795776195466785825));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3040631852046752166));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4529279813148173111));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3658352497551999605));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13205889818278417278));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17853215078830450730));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14193508720503142180));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1488787817663097441));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8484116316263611556));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4745643133208116498));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14333959900198994173));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10770733876927207790));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17529942701849009476));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8081518017574486547));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5945178879512507902));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9821139136195250096));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4728986788662773602));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  840062144447779464));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9315169977352719788));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12843335216705846126));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1682692516156909696));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16733405176195045732));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  570275675392078508));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2804578118555336986));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18105853946332827420));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11444576169427052165));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5511269538150904327));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6665263661402689669));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8872308438533970361));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5494304472256329401));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5260777597240341458));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17048363385688465216));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11601203342555724204));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13927871433293278342));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13168989862813642697));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13332527631701716084));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1288265801825883165));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8980511589347843149));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1639193574298669424));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14012553476551396225));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7818048564976445173));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11012385938523194722));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1594098091654903511));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5035242355473277827));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11507220397369885600));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4097669440061230013));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4158775797243890311));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8008476757622511610));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18212599999684195413));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3892070972454396029));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15739033291548026583));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5240984520368774617));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15428220128146522508));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6764778500174078837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17250425930626079997));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15862445320841941901));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9055707723866709616));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  407278260229756649));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6679883267401891436));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13585010976506536654));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9580697194899010248));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7802093638911637786));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  535562807229422763));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16772549087470588412));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2069348082463192648));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18080878539236249869));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12688200000096479737));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8989665349769173357));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13575112928849473200));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10859033464356012248));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9748216112997718693));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8405158063935141693));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15279502632583570477));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16055899490125284200));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9066388900883848980));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17884680971936629565));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16395391805201036549));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2550532686790805254));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8052938288948613298));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6344035301348514175));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2193824757648316037));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10113332896580941759));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14001553499759966766));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  597702890888347204));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1874324574384293454));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10826913572691111562));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12821185545071087721));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14606566723149387105));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15679487422249894303));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16146086267469614290));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11169330698794304272));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17590151747242102595));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18278229723818623796));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15994633360516603469));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11881756471423721131));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11153906733009525059));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16836145075420168747));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8614597919830747987));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1459907787369619658));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16682004712721580156));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15261848763679157527));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2717413695111288049));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14889665525641206303));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12338480473037317818));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2557597240994564872));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12402353581130313583));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15355546302939095474));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17651033590338072704));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11616809212196625943));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6561978461173088746));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5962436378610109024));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1168012300494473422));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5175053317267933097));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4740525681678845797));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1614376253554691208));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1358027693590031708));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1856992378370522222));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2410813678132517023));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11582456654366157909));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5754940895753314317));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17548218371729667895));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17945642044770404276));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3721164045489467070));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13394551493150992827));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12475264300415171883));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10462606688633056562));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13251365510693735175));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3876338822302790600));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13771801863059799470));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13815564444636394855));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16495110748802246170));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2156091871580385249));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12069080176326280986));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  489805578737239572));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5271183164515543116));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11286401144444756863));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6746000579485080744));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5186625150343537151));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13119883039086991857));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16025170396082521338));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2259331576759215945));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16362343102415556603));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10982898132796723193));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14666888772828547003));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10462483830193419334));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18236154274104239589));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17759599582309981676));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9339512652453242670));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14635458573977612405));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13273192362623128494));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7419053614262815071));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2139880725825605974));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15336265650071823816));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6291952205449675957));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14977329074317573394));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4364768269648744391));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17232241565077788317));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8450549923677533764));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15732483035355013039));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13831185231495622915));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6819123640184841760));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11886944798543888851));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10879889186777890996));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15555433551230813341));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  105259452319848079));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3441909642659419332));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5480947869602487239));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6247709904124292706));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13391610271247915041));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18346462037123761313));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16636317150577797347));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14149179703416851896));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2376171948756359367));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5152472389910152792));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2368047066677070121));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16396163399604156946));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14864288050288048653));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7393398358587456124));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9728143941576351989));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5481913815176021747));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16927964714362701213));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14993236783745363262));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9552302871570670457));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11071069341174528295));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15381321939083200837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8816171210895558106));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6071991122052964372));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10925078611503375837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15239629154712277871));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8615167154188153180));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4917230293625512515));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14895742215835130464));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2359753755290725009));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6783321469015983851));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  360705462143558065));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2287732638733919300));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2984153050512747353));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8021412450653308816));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12759258587083258672));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1585563973173997547));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18209504305389149669));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11416757620121532143));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6846989578536141166));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4365862612957164362));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2931876801952518067));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  680191398818283694));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1834352496547951770));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12616538556720116808));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17563613795929063197));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14519515363534791688));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4349527158980778739));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6714794984698083967));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6696141578113299617));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17231874453010340947));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18425812703539835928));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3707544366662920973));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10197276740411893574));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12864434420502416888));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12767250491273234520));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1588549204908870909));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6610295429674120152));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5281895767268096036));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1739897672032589486));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17406469206626426854));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8710378533013875691));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9587926405039941516));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2805299725371867574));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7146901261023555807));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1825062423171923931));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3049052876249887095));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10771741767689142181));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8733642741329011601));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11979515434717210935));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10043245691272652957));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5830279975302858953));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17190113074333440499));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18260575806620923460));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14335648769917655401));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4153816861017702156));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14590500750979768984));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  810991542442466488));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7089785717813579612));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12357837562747114001));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5554121432788679660));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5931025703748246718));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2097835176693352889));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12745618408404359587));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6090924568528767236));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14734637834598564704));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14439652293742648615));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  132405348116615733));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13869945305505934743));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7372953811704808036));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7756437368369298361));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3794582695199039623));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12917619229835701974));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14320084076906478671));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2606626751703588462));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3137561743724131360));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13808802441028589896));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14231944027275971054));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16852581317945783254));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10323673491841952054));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2313335010769237820));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13955532667350441768));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5747153089934705338));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13377135145695875091));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6830230899286657495));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(   81856298782858401));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1754724887913860152));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13750479713795882912));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11120120136303124367));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15046307382468953177));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3696979254055818020));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15352898388246644384));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1024778962410818770));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2388728043318081123));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6871857727931721608));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17721619206096294273));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10585202864517959301));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10898249199547365704));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9663430180652362739));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1737102419936989910));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5117227310201589790));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16884367896390523102));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10498150099412419335));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1921007855220546564));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7643484074408755248));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11318429053286342939));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1370093900783164344));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6776537281339823025));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3450492372588984223));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9401014545757436331));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7896519943553875907));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14303443932332314010));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  281238069833157985));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9628364435514671685));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1035647896705322917));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  940113500519447970));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12858978713386075837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2103046007104782505));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1170332608028903179));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6569179731999105361));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9795365446060253382));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3663276878692063340));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11746321300354091749));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5408361990473950532));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9735653452670998906));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4324195634733601175));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9037136744494003310));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10715330324656609711));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3474343689175121886));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5794004792094061662));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13295581273946061060));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7292949743142825837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10886028626057941279));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10688849249577735178));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17297010345160851373));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13658139148821214513));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4468290234101910565));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9583516840381960864));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2100818272677130469));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3835407486618772476));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11687972045781987867));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2584265809482868424));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2184370854727222683));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17762352308671769689));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10901114407297935135));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17932666452350314317));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14800534017102555607));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16233839909626358812));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1704089397092793640));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2891239861334407450));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18077585692287687954));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2363047449739120434));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5904357530901606076));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16765772907460692007));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8757786729323486734));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3706883612695347371));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14958907430930711064));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9624134580897548276));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10298009507777483067));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5667412839234900228));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6828701555684071915));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10482797977665945217));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13440894740881464138));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12078258924098889769));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5740761565098658841));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13914375003115830180));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16808960379045776034));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18421450170384511575));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16478974619417516521));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14381565232287562804));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12792472782420522791));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6620422687983566193));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12025299949416885293));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(    6046334025019123));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16769051888439418536));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10312203372653850423));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  720028297035890629));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6441255456466558203));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9874005816230679263));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15903170012916142038));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7557768652767625223));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17626605079857371651));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9092603716684679963));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15518831173015579794));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  300798272301981904));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13762040857722893585));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3117104080838901168));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4702649037537941245));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14408238429167682374));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17923200330177894118));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7470538549881440849));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3664543122474851710));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17626200978883719521));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15355649603762884691));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4749231114166154448));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11220859020615935192));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4740127963151294603));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16616708905207951068));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9828299274924872726));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8985762004928355786));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14578866413196595465));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11009044264074492189));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16196760954725621137));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10725252972011913420));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4601011175737567235));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1441938685024169613));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1896485105672535586));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6635496128279078494));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7401072902622950072));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16075245295895555285));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11009539992705810569));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13666961049432909413));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  930044899627839572));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7899294831116079515));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7830402010660588539));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5485720725031791061));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17051528642209786987));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7280223907880312904));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10641556535303807158));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12639056541805784436));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12321318600465693220));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10108223508416203621));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16243972184205577210));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8544062083712081766));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11274622334580836223));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10844017387984539333));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14774228730866078526));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  560237794062265107));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5844494700804214355));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12270220729021534083));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8560016492134621125));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12198417933760222474));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10133839346494565561));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9295901871619786454));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10849442312533122519));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18021432643418872607));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10155396024449547909));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10524212640889309144));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16662796689072019468));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  965963318619140447));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8887484786999567242));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15714444653107301219));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1678356452623540647));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11052117692502964420));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14549914962216724919));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2062106447906584711));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9160372737526136799));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  408961132483689555));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16057982805180036489));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3569128826873655261));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9330490631980133992));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1176328083272936519));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11222898184704497134));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9302091588024171405));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10671057562378043302));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4098229850247478874));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8603114141751656125));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5095034292565071557));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17972196540767155575));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17052421619317624598));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1582078615100434096));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12012345949788712038));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16161371278263065802));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2541771182459136706));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4555228648728151989));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8434259952664443907));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11417314755930316675));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4859944209493970278));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3960064386733120970));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  831798891742765072));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15333350611999607709));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16195235791627584805));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11597945977924582290));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5623573319924035254));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11517834322140013944));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4133597640080778846));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5871425684860123605));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1689282515842046354));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12636468992840026995));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14838814546330146559));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  521771145052581487));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2880434048302248640));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8371131723257691693));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14881811984607317690));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1324986559026356337));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15096177686518116013));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4421234407032663127));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14405416956529710514));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3720189381923668652));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  409223713688462738));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9606291214917499037));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9223836018030016969));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  190459553092726002));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12216883190512504355));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2445407445757699168));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4632853494959579227));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13184809158706083946));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5787237245171889527));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10294885203231741175));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4191072920233802133));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4291939441266046279));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16375865614446560083));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8623994097296487259));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15309273767847758202));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9397335507036899909));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6747046333776906674));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13832845734789247298));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7019441607318179720));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10005910351872177492));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4145022192145704170));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4353043221960833896));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8973895156742077167));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  438950987149754489));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2185272607213603213));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8466605802960622962));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12110999198806592422));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11821045514824268224));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10878266882585355136));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11760743717116988087));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4184976790109698342));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18330309416210613006));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1107206443001387417));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(   79384941109554222));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9163366224008952362));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3321824684344751056));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3693723307432954164));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6079394558849393056));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11401125038466760935));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3656219353656357222));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1735342045865967049));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4042759343240967690));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12711975181279962687));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9500297538285176400));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15298274009373410204));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9806309365986113958));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10640867530898511005));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17462737140104853956));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4414872795937286161));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14852747253248972903));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15278706409822090441));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6433625831299907179));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3321907667985685429));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11390693584827212740));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11529629266037992234));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10328859824139248147));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16428469301035734767));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17926643922068445985));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  705326063324784242));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8105287564212541268));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15433828269766668455));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3714790519415313767));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5417718938962187987));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7847045502609209896));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8025090526912661197));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2234136672994823541));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16041001438425499955));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10050820915370092068));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14731208739754682952));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9320476318639351023));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14993011533295358880));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4179632880986595543));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8947621078428360390));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14715184767037401701));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2617407252848328649));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4818108510694228841));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3602814087839803186));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14679368779377024657));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7354547195052671772));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  453184876960970470));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15781004944602184656));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12000277437894508493));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14990587330205222466));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13913588124149397652));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14252160166631667289));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1532590395334038243));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10283229111568663870));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17325140074534683390));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15829693190940193580));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7621681592523599724));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10682206684717316020));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1847393779801417006));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3066262069769536156));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14633662576956154615));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15324290530255177253));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14627271171597064522));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14334883061544405592));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12329324284039697670));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14425669906700626239));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4967072546582904838));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11336784484312139551));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9293117687355182150));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18198595579111618687));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3236555730692485133));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3659681352365625914));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5185822088933195476));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1820961806679957133));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5103404090674191862));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16176358349875499548));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15699479324816269479));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6929077312607579230));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7724671543660786314));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15226863704421704735));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10411799650043017788));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2743533500235068318));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7917895244279791454));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9194839772540541837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8170679394364395846));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2830213237197365734));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7353896603754987224));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17634372441601249827));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8515117661105161813));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5818937363197514778));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8536843065945835629));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2920190566549352463));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4206179361653770600));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15470355568872211976));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8427825008315838911));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5786540713287383830));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15547153445796060183));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12329720415526259303));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5557519966701086911));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17778904544770937806));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17514165232876376499));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17788126989478779154));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17150186057659184837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(   96482940290395907));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5391763100021787727));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13311921842198397018));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  212666859219880844));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17021563369181645958));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11336487866339302675));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9141466969851850320));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15662548514627491449));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7860565965198889264));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13899151565605256321));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13381351357933618242));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14888589325358078776));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8892463471491396086));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15103645417329254911));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5461076326426815327));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1842242118931503497));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4404875173572687401));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13971514988681285540));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17325818256926300242));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15093194250549176553));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2037055123708268678));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16257942776085749532));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10590700494563920368));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9510359897405254265));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14355127120473277462));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17727696335014918206));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5852409884542362577));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15296449745630772621));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15183080793648581194));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16672300287494724460));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13969062191570534211));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9287911224447475220));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8201339388669403090));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6896471492123786378));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17836899414146968932));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18212192901661339968));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13589433629948059304));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11028761701391980161));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6774257768766057466));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12173254712476586450));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12848080044100037611));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8528727180962924144));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1419515180397454273));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14756964613420120449));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5897971337265509756));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7895151636400005603));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12470640271491881548));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1601970429693801261));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15095880759160767112));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9199134360165595311));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3011979166743445660));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12194258846860258337));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3655956427657893470));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2336006494839215747));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14852738832219225696));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  980198724853947166));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15813714724821744703));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  791599627749143069));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7348649629500435093));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15262170387612229708));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15303522042429377318));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8425613881574971669));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15801520478161584059));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14074339996639769183));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11257371430211006951));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13402846107666422911));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6626174270103647993));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14681865631183261473));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4451829808427344249));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5724496674828323027));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14972202730373011904));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11481387700399791014));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17251674197980054589));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1838448127446963389));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1207376433173020767));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9073502573042965711));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13838468943242852421));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5020402172333584866));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1289816897702617855));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16074253538753775279));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2848619861528754568));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5973640488400207347));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3071333206607986706));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12888295794552218363));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5137015169022219459));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6855845130657996130));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13509486497434551605));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3108141178579225625));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11547672040624451730));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8228290404814299648));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  556050802577131935));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1291564719606180243));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12244181387410231701));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  278862824664555643));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15819233907350967248));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17068340672410761541));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18086332283553587440));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  315661412098236862));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2955724640055190386));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13784171517969596718));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1290092202280378555));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16592960356544350813));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16858716830044052167));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  897800245874941430));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13540719245109081222));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1083645675370617492));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1386604666325681943));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15322508077521817796));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17200996622826525908));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  377359248448239623));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  903752203431456047));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5917034777147383262));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14427307358937396394));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7312697203701764121));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16205567221211754073));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2397860267640591749));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7771620645425797965));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4628927853429026927));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10814117705525372791));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1781328114115837238));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8047892713447387890));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14325316982420673798));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  307254776259779147));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18088484138876255865));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6235903010317883578));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4576014393928771646));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11902339501721522128));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11466077100832512450));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1320279640819393445));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7149459462530810516));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17183597293436304345));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10494281344202625434));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15351591175592386700));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6787100053213878886));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4662750932232247137));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9155970812684385713));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16135661826301908461));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18304055221177982100));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9416630457538799891));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14565714036259547408));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1567015689328697302));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6354592639376292074));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17850376331797597416));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10659961423006612289));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17519724454480552469));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11678126474628634190));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1758403873368227086));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15990654484983665543));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11214965843994241271));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12011525616758716968));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10447121228281024024));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2305551614414338365));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16044382098030270132));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12224093843246544395));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16482020615341119088));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10963267852467973529));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17349096433594909876));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12720620716465149679));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16276416704186986927));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9749492746774307320));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4878537625661881849));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3596658220251367324));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1250366616134450842));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6810591609326569041));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3598393359978582179));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12253949833836762468));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12506534045411421786));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5579778259811453914));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9776215669441974360));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2458400878391347086));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8348402873370648617));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7121289029007601878));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12229263687747326448));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17037102369907624029));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4480768498551219639));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2055333633189980498));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1996380200405366126));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16269828220783280610));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2087527536209518012));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15824764389964056849));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2567742633386698867));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  712225451323391987));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8933753793270353555));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13585078573555411625));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  867327728114824049));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7583624842930804212));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3377553416701167536));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16921474325853581610));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16140895111948716532));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12207578918983470422));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1646174217514160398));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1607026980068118758));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13303437415396801338));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12587920481280066999));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13987107713225334715));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4992520640352863728));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7263447506959407124));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5199858030989230147));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9061431503722510949));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14400252600937389012));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5744796948459454394));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10292591259771880588));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1721312458226718226));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16635218433302059851));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11656534395951367480));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3706250251522790279));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13109149372599278855));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15782623443980200467));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16550074045325605656));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8552238076040605299));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9482885590978036086));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9671478455262059566));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10893144584594332227));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16019322840413540987));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8296393284743045220));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17938815114635715205));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6264829385778688153));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1851240405679727308));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8252127944258078957));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  278628055251899660));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17284352895107447551));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1917595448595108230));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4194184253204412852));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6053135762636289112));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17829479236593827180));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17138125122391290546));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12701771613115087757));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15851635548108017237));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15429185374696762248));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1092543755026054444));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11339471547318422768));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5928009338273096310));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15799438807585898358));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7759398761882215565));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6631869218547533701));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1467136438670102142));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8634286993913126889));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1625376813926906406));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11217810185908857400));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8625591546392093923));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17296397694263603933));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15605747394391364522));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1662045846911902792));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17127000247114321803));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8356660387712428204));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1918485604873496907));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2370571666901420648));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15787385066387380099));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5464477678597110906));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11425249858769946518));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6591019215869863149));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9479744778601152624));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15172779966452753614));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7752719149069356364));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18415611086810130089));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17283471393219112121));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12355267205606811577));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  635145025396992592));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5396237116775390812));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3552831403988064335));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14168163303014268947));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12627213986475551446));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15759311136550812405));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1626935584364203400));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9310496835576179512));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6245520276975783671));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7024663181048246622));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3427477009023336836));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2933969795091320036));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2151226409008220811));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11539738627618492576));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13725329897058492021));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7498965915916310645));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10290852052645224423));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6604848973131369881));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15811231974215060058));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2271887981598533043));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3594314518191525754));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17149519013436525742));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8679304823079730570));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13578822930296496311));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7091768012700047649));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3785057672906901675));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4236181286844492290));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8508302517230928544));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7300358184937218339));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12384908352232692669));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12492744495972933877));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7314891822313963191));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8324938997494297354));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5209197900603935779));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2658432712832078185));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5318876851637134397));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16575705827128833203));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10064850062465251207));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4212292470574654048));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14980416404180533629));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16757608328210085737));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2318174214965864870));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11090483489978208173));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4454423999516879015));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16112997200825396525));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6280448590284782941));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9633346215123474089));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4043276543108671776));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5617487009102249240));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5876783769254797390));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12282204452000419979));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11539547785106538148));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7026195643862072596));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3299214246762090106));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12374300965992954143));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6758847474999357295));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15792537753857445948));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17507047352822497538));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12191874001355785115));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13766936657810901564));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6187579338224863221));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  575235837446271943));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3941374392937187760));
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
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7266447313870364031));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4946485549665804864));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16945909448695747420));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16394063075524226720));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4873882236456199058));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14877448043947020171));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6740343660852211943));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13857871200353263164));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5249110015610582907));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10205081126064480383));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1235879089597390050));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17320312680810499042));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16489141110565194782));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8942268601720066061));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13520575722002588570));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14226945236717732373));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9383926873555417063));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15690281668532552105));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11510704754157191257));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15864264574919463609));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6489677788245343319));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5112602299894754389));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10828930062652518694));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15942305434158995996));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15445717675088218264));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4764500002345775851));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14673753115101942098));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  236502320419669032));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13670483975188204088));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14931360615268175698));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8904234204977263924));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12836915408046564963));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12120302420213647524));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15755110976537356441));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5405758943702519480));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10951858968426898805));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17251681303478610375));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4144140664012008120));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18286145806977825275));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13075804672185204371));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10831805955733617705));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6172975950399619139));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12837097014497293886));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12903857913610213846));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  560691676108914154));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1074659097419704618));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14266121283820281686));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11696403736022963346));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13383246710985227247));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7132746073714321322));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10608108217231874211));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9027884570906061560));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12893913769120703138));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15675160838921962454));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2511068401785704737));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14483183001716371453));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3774730664208216065));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5083371700846102796));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9583498264570933637));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17119870085051257224));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5217910858257235075));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10612176809475689857));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1924700483125896976));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7171619684536160599));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10949279256701751503));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15596196964072664893));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14097948002655599357));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  615821766635933047));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5636498760852923045));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17618792803942051220));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  580805356741162327));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  425267967796817241));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8381470634608387938));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13212228678420887626));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16993060308636741960));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  957923366004347591));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6210242862396777185));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1012818702180800310));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15299383925974515757));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17501832009465945633));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17453794942891241229));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15807805462076484491));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8407189590930420827));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  974125122787311712));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1861591264068118966));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  997568339582634050));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18046771844467391493));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17981867688435687790));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3809841506498447207));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9460108917638135678));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16172980638639374310));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  958022432077424298));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4393365126459778813));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13408683141069553686));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13900005529547645957));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15773550354402817866));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16475327524349230602));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6260298154874769264));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12224576659776460914));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6405294864092763507));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7585484664713203306));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5187641382818981381));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12435998400285353380));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13554353441017344755));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  646091557254529188));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11393747116974949255));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16797249248413342857));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15713519023537495495));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12823504709579858843));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4738086532119935073));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4429068783387643752));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  585582692562183870));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1048280754023674130));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6788940719869959076));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11670856244972073775));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2488756775360218862));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2061695363573180185));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6884655301895085032));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3566345954323888697));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12784319933059041817));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4772468691551857254));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6864898938209826895));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7198730565322227090));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2452224231472687253));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13424792606032445807));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10827695224855383989));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11016608897122070904));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14683280565151378358));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7077866519618824360));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17487079941198422333));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3956319990205097495));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5804870313319323478));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8017203611194497730));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3310931575584983808));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5009341981771541845));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6930001938490791874));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14415278059151389495));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11001114762641844083));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6715939435439735925));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  411419160297131328));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4522402260441335284));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3381955501804126859));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15935778656111987797));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4345051260540166684));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13978444093099579683));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9219789505504949817));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9245142924137529075));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11628184459157386459));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7242398879359936370));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8511401943157540109));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11948130810477009827));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6865450671488705049));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13965005347172621081));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15956599226522058336));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7737868921014130584));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2107342503741411693));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15818996300425101108));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16399939197527488760));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13971145494081508107));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3910681448359868691));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4249175367970221090));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9735751321242454020));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12418107929362160460));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  241792245481991138));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5806488997649497146));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10724207982663648949));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1121862814449214435));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1326996977123564236));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4902706567834759475));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12782714623891689967));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7306216312942796257));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15681656478863766664));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  957364844878149318));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5651946387216554503));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8197027112357634782));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6302075516351125977));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13454588464089597862));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15638309200463515550));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10116604639722073476));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12052913535387714920));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2889379661594013754));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15383926144832314187));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7841953313015471731));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17310575136995821873));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9820021961316981626));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15319619724109527290));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15349724127275899898));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10511508162402504492));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6289553862380300393));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15046218882019267110));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11772020174577005930));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3537640779967351792));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6801855569284252424));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17687268231192623388));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12968358613633237218));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1429775571144180123));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10427377732172208413));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12155566091986788996));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16465954421598296115));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12710429690464359999));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9547226351541565595));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12156624891403410342));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2985938688676214686));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18066917785985010959));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5975570403614438776));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11541343163022500560));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11115388652389704592));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9499328389494710074));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9247163036769651820));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3688303938005101774));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2210483654336887556));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15458161910089693228));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6558785204455557683));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1288373156735958118));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18433986059948829624));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3435082195390932486));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16822351800343061990));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3120532877336962310));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16681785111062885568));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7835551710041302304));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2612798015018627203));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15083279177152657491));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6591467229462292195));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10592706450534565444));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7438147750787157163));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  323186165595851698));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7444710627467609883));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8473714411329896576));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2782675857700189492));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3383567662400128329));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3200233909833521327));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12897601280285604448));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3612068790453735040));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8324209243736219497));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15789570356497723463));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1083312926512215996));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4797349136059339390));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5556729349871544986));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18266943104929747076));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1620389818516182276));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  172225355691600141));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3034352936522087096));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1266779576738385285));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3906668377244742888));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6961783143042492788));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17159706887321247572));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4676208075243319061));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10315634697142985816));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13435140047933251189));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  716076639492622016));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13847954035438697558));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7195811275139178570));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10815312636510328870));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6214164734784158515));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16412194511839921544));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3862249798930641332));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1005482699535576005));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4644542796609371301));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17600091057367987283));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4209958422564632034));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5419285945389823940));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11453701547564354601));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9951588026679380114));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7425168333159839689));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8436306210125134906));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11216615872596820107));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3681345096403933680));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5770016989916553752));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11102855936150871733));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11187980892339693935));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  396336430216428875));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6384853777489155236));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7551613839184151117));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16527062023276943109));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13429850429024956898));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9901753960477271766));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9731501992702612259));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5217575797614661659));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10311708346636548706));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15111747519735330483));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4353415295139137513));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1845293119018433391));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11952006873430493561));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3531972641585683893));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16852246477648409827));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15956854822143321380));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12314609993579474774));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16763911684844598963));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16392145690385382634));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1545507136970403756));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17771199061862790062));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12121348462972638971));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12613068545148305776));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  954203144844315208));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1257976447679270605));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3664184785462160180));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2747964788443845091));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15895917007470512307));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15552935765724302120));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16366915862261682626));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8385468783684865323));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10745343827145102946));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2485742734157099909));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  916246281077683950));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15214206653637466707));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12895483149474345798));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1079510114301747843));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10718876134480663664));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1259990987526807294));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8326303777037206221));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14104661172014248293));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15531278677382192198));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3874303698666230242));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3611366553819264523));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1358753803061653874));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1552102816982246938));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14492630642488100979));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15001394966632908727));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2273140352787320862));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17843678642369606172));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2903980458593894032));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16971437123015263604));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12969653681729206264));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3593636458822318001));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9719758956915223015));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7437601263394568346));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3327758049015164431));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17851524109089292731));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14769614194455139039));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8017093497335662337));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12026985381690317404));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  739616144640253634));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15535375191850690266));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2418267053891303448));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15314073759564095878));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10333316143274529509));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16565481511572123421));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16317667579273275294));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13991958187675987741));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3753596784796798785));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9078249094693663275));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8459506356724650587));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12579909555010529099));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7827737296967050903));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5489801927693999341));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10995988997350541459));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14721747867313883304));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7915884580303296560));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4105766302083365910));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12455549072515054554));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13602111324515032467));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5205971628932290989));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5034622965420036444));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9134927878875794005));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11319873529597990213));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14815445109496752058));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2266601052460299470));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5696993487088103383));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6540200741841280242));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6631495948031875490));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5328340585170897740));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17897267040961463930));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9030000260502624168));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14285709137129830926));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12854071997824681544));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15408328651008978682));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1063314403033437073));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13765209628446252802));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  242013711116865605));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4772374239432528212));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2515855479965038648));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5872624715703151235));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14237704570091006662));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  678604024776645862));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12329607334079533339));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17570877682732917020));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2695443415284373666));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4312672841405514468));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6454343485137106900));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8425658828390111343));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16335501385875554899));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5551095603809016713));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11781094401885925035));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9395557946368382509));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9765123360948816956));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18107191819981188154));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16049267500594757404));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16349966108299794199));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1040405303135858246));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2366386386131378192));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  223761048139910454));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15375217587047847934));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15231693398695187454));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12916726640254571028));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8878036960829635584));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1626201782473074365));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5758998126998248293));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18077917959300292758));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10585588923088536745));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15072345664541731497));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3559348759319842667));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12744591691872202375));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2388494115860283059));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6414691845696331748));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3069528498807764495));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8737958486926519702));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18059264986425101074));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3139684427605102737));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12378931902986734693));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  410666675039477949));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12139894855769838924));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5780722552400398675));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7039346665375142557));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3020733445712569008));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2612305843503943561));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13651771214166527665));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16478681918975800939));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  566088527565499576));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4715785502295754870));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6957318344287196220));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11645756868405128885));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13139951104358618000));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17650948583490040612));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18168787973649736637));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5486282999836125542));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6122201977153895166));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17324241605502052782));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10063523107521105867));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17537430712468011382));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10828407533637104262));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10294139354198325113));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12557151830240236401));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16673044307512640231));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10918020421896090419));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11077531235278014145));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5499571814940871256));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2334252435740638702));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18177461912527387031));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2000007376901262542));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7968425560071444214));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1472650787501520648));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3115849849651526279));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7980970700139577536));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12153253535907642097));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8109716914843248719));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3154976533165008908));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5553369513523832559));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10345792701798576501));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3677445364544507875));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10637177623943913351));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7380255087060498096));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14479400372337014801));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15381362583330700960));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  204531043189704802));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13699106540959723942));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3817903465872254783));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10972364467110284934));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2701394334530963810));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2931625600749229147));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16428252083632828910));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11873166501966812913));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5566810080537233762));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7840617383807795056));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10699413880206684652));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18259119259617231436));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10332714341486317526));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10137911902863059694));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  669146221352346842));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8373571610024623455));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10620002450820868661));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12220730820779815970));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5902974968095412898));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7931010481705150841));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16413777368097063650));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11273457888324769727));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13719113891065284171));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8327795098009702553));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10333342364827584837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6202832891413866653));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9137034567886143162));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14514450826524340059));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  473610156015331016));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  813689571029117640));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13776316799690285717));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10429708855338427756));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8995290140880620858));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2320123852041754384));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8082864073645003641));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6961777411740398590));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10008644283003991179));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3239064015890722333));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16762634970725218787));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16467281536733948427));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10563290046315192938));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5108560603794851559));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15121667220761532906));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14155440077372845941));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10050536352394623377));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15474881667376037792));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3448088038819200619));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3692020001240358871));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6444847992258394902));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8687650838094264665));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3028124591188972359));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16945232313401161629));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15547830510283682816));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3982930188609442149));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14270781928849894661));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13768475593433447867));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13815150225221307677));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8502397232429564693));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  718377350715476994));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7459266877697905475));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8353375565171101521));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7807281661994435472));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16924127046922196149));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10157812396471387805));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2519858716882670232));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7384148884750265792));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8077153156180046901));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3499231286164597752));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2700106282881469611));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14679824700835879737));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14188324938219126828));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3016120398601032793));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10858152824243889420));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9412371965669250534));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4857522662584941069));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  984331743838900386));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4094160040294753142));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2368635764350388458));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15101240511397838657));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15584415763303953578));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7831857200208015446));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1952643641639729063));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4184323302594028609));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16795120381104846695));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3541559381538365280));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15408472870896842474));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5628362450757896366));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16277348886873708846));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12437047172652330846));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10172715019035948149));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1999700669649752791));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6217957085626135027));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11220551167830336823));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16478747645632411810));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5437280487207382147));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11382378739613087836));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15866932785489521505));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5502694314775516684));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16440179278067648435));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15510104554374162846));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15722061259110909195));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10760687291786964354));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10736868329920212671));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4166148127664495614));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14303518358120527892));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9122250801678898571));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10028508179936801946));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  216630713752669403));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10655207865433859491));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4041437116174699233));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6280982262534375348));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  297501356638818866));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13976146806363377485));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13752396481560145603));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11472199956603637419));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16393728429143900496));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14752844047515986640));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1524477318846038424));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6596889774254235440));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1591982099532234960));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8065146456116391065));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3964696017750868345));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17040425970526664920));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11511165586176539991));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3443401252003315103));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16314977947073778249));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16860120454903458341));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5370503221561340846));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15362920279125264094));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2822458124714999779));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14575378304387898337));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9689406052675046032));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2872149351415175149));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13019620945255883050));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14929026760148695825));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8503417349692327218));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9677798905341573754));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  828949921821462483));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16110482368362750196));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15794218816553655671));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14942910774764855088));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12026350906243760195));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13610867176871462505));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18324536557697872582));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2658962269666727629));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  327225403251576027));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9207535177029277544));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8744129291351887858));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6129603385168921503));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18385497655031085907));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13024478718952333892));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14547683159720717167));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5932119629366981711));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  325385464632594563));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3559879386019806291));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6629264948665231298));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14358245326238118181));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15662449672706340765));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13975503159145803297));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3609534220891499022));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4224273587485638227));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9274084767162416370));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13156843921244091998));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18284750575626858789));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14664767920489118779));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11292057742031803221));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13919998707305829132));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14473305049457001422));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9696877879685767807));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1406758246007973837));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2429517644459056881));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14361215588101587430));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11386164476149757528));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10474116023593331839));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2921165656527786564));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15604610369733358953));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12955027028676000544));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10314281035410779907));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3167047178514709947));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1088721329408346700));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17930425515478182741));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7466411836095405617));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15534027454610690575));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10879629128927506091));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11502219301371200635));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13915106894453889418));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4226784327815861027));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12335222183627106346));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3648499746356007767));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18441388887898023393));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18117929843327093625));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4237736098094830438));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14229123019768296655));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3930112058127932690));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12663879236019645778));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9281161952002617309));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4978473890680876319));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  845759387067546611));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1386164484606776333));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8008554770639925512));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11159581016793288971));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18065390393740782906));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17647985458967631018));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9092379465737744314));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2914678236848656327));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4376066698447630270));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16057186499919087528));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3031333261848790078));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2926746602873431597));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7931945763526885287));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  147649915388326849));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15801792398814946230));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5265900391686545347));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16173686275871890830));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7562781050481886043));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5853506575839330404));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14957980734704564792));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10944286556353523404));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1783009880614150597));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9529762028588888983));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  822992871011696119));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2130074274744257510));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8000279549284809219));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3514744284158856431));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  128770032569293263));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3737367602618100572));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16364836605077998543));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  783266423471782696));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4569418252658970391));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11093950688157406886));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14888808512267628166));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4217786261273670948));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17047486076688645713));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14133826721458860485));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17539744882220127106));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12394675039129853905));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5757634999463277090));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9621947619435861331));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1182210208559436772));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14603391040490913939));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17481976703660945893));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14063388816234683976));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2046622692581829572));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8294969799792017441));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5293778434844788058));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17976364049306763808));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  399482430848083948));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16495545010129798933));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15241340958282367519));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  989828753826900814));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17616558773874893537));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2471817920909589004));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11764082277667899978));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9618755269550400950));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1240014743757147125));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1887649378641563002));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1842982574728131416));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13243531042427194002));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7688268125537013927));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3080422097287486736));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2562894809975407783));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12428984115620094788));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1355581933694478148));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9895969242586224966));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8628445623963160889));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4298916726468199239));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12773165416305557280));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5240726258301567487));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4975412836403427561));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1842172398579595303));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7812151462958058676));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17974510987263071769));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14980707022065991200));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18294903201142729875));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12911672684850242753));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8979482998667235743));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16808468362384462073));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5981317232108359798));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12373702800369335100));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16119707581920094765));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2782738549717633602));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15454155188515389391));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16495638000603654629));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16348757069342790497));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7769562861984504567));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17504300515449231559));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5557710032938318996));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11846125204788401203));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13957316349928882624));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2738350683717432043));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15738068448047700954));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6224714837294524999));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6081930777706411111));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11366312928059597928));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4355315799925031482));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12393324728734964015));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15277140291994338591));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1406052433297386355));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15859448364509213398));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1672805458341158435));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2926095111610982994));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11056431822276774455));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12083767323511977430));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3296968762229741153));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12312076899982286460));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17769284994682227273));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15349428916826953443));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1056147296359223910));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18305757538706977431));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6214378374180465222));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14279648441175008454));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17791306410319136644));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  956593013486324072));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2921235772936241950));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10002890515925652606));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10399654693663712506));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6446247931049971441));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6380465770144534958));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11439178472613251620));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10131486500045494660));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3692642123868351947));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10972816599561388940));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4931112976348785580));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8213967169213816566));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15336469859637867841));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15026830342847689383));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7524668622380765825));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17309937346758783807));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  372780684412666438));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5642417144539399955));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18303842993081194577));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11085303253831702827));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15658163165983586950));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8517521928922081563));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16091186344159989860));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17614656488010863910));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4736067146481515156));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13449945221374241354));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17755469346196579408));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13300502638545717375));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6611828134763118043));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14177591906740276597));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9340430243077460347));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7499765399826404087));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3409518087967832469));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9013253864026602045));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4444307427984430192));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3729283608700519712));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13642048880719588383));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16486557958022946240));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2996465014991157904));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10020049344596426576));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12302485648009883778));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8492591321344423126));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17407986443716172520));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10530482934957373052));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15740662350540828750));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1790629986901049436));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6305948377669917188));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15092985352503125323));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  928505047232899787));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14404651977039851607));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7564177565277805597));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3411236815351677870));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7752718145953236134));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12315979971311483798));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12477729506691004724));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14654956300924793305));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6689803038918974388));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1540738812233000153));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13508351811701989957));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15864432023192136053));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7990997967273843917));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7424300239290765161));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(   39585249496300263));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3877436595063283319));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10710642254398044448));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4653804418844456375));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1232267496410380283));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3690525514009038824));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15459770765077428485));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13240346522153894145));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5674964360688390624));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16973644653010587289));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15924280764204855206));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15196708627253442662));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17596174821341373274));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16196745023027393691));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6980050627399795351));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17582264380857746637));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18170372407506856324));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12108126025631005514));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15687749089493373169));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5814107289258228434));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9381977959648494876));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15895601183088112734));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16267869075651604263));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15228381979765852785));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11949618678312581999));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4545324791131029438));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  582725409406225185));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15282520250746126790));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14758446535973412711));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7605613563088071833));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1111140641057375915));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5364843095234852245));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  218335432181198977));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4891472444796201742));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4564628942836375772));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15500501278323817088));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4913946328556108657));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2684786251736694229));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12090498456116310122));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5310885782157038567));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5032788439854011923));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12627401038822728242));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11869662610126430929));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17650156853043540226));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12126672500118808436));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10437658933435653256));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13133995470637873311));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4601324715591152820));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1874350460376708372));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5808688626286061164));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13777088437302430376));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5018451954762213522));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2588296738534474754));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5503414509154170711));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5230497186769951796));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13261090710400573914));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8515217303152165705));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11074538219737365303));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15481562385740613213));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12705484409881007350));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14221931471178549498));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12905633420087112297));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17337759164357146506));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14081997515778175224));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17384320185513122939));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7131793076779216692));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17483217190312403109));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  900692047897995877));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14723287313048560400));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6132094372965340305));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7572797575350925726));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12725160700431903514));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  380860122911632449));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1900504978569024571));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 8423729759529914138));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7305587201606052334));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12446871355267313320));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4615812356515386206));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3361817115406652303));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(17690418922000878428));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14632214537567910559));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2709702289926174775));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3459675155951086144));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7788364399926538150));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16043992474431955950));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15830963823784930267));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4216893617835797954));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  538159724689093771));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16029152738918251363));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(14444848757576686696));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12941757045272633696));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10900480525147953314));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12547307449905859302));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16001571796892398181));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  407942194622690676));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(13873235372903944444));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18071603799493008777));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 1015646077646778622));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9387605808959554815));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11566702442022019410));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7061722181092883183));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2629032108249254109));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5271820053177594520));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12640880742139693547));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10098688629735675775));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 5716304472850923064));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 3312674502353063071));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7295926377425759633));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(  833281439103466115));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16316743519466861667));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9912050326606348167));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(11651133878100804242));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(18026798122431692459));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 6157758321723692663));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 4856021830695749349));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 7074321707293278978));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(10748097797809573561));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 2949954440753264783));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9813922580940661152));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C( 9949237950172138336));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(15643982711269455885));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(16078663425810239127));
	CU_ASSERT_EQUAL(mt64_gen_u64(&ctx), UINT64_C(12508044395364228880));
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
