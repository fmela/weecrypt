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

CU_TestInfo mp_digit_tests[] = {
	TEST_FUNC(test_mp_digit_mul),
	TEST_FUNC(test_mp_digit_div),
	TEST_FUNC(test_mp_digit_invert),
	TEST_FUNC(test_mp_digit_log2),
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

CU_TestInfo mp_basic_tests[] = {
	TEST_FUNC(test_mp_add_n),
	TEST_FUNC(test_mp_sub_n),
	TEST_FUNC(test_mp_mul),
	TEST_FUNC(test_mp_mul_bug),
	TEST_FUNC(test_mp_div),
	TEST_FUNC(test_mp_lshift),
	TEST_FUNC(test_mp_rshift),
	CU_TEST_INFO_NULL
};

void test_mpi_rand();
void test_mpi_cmp();

CU_TestInfo mpi_basic_tests[] = {
	TEST_FUNC(test_mpi_rand),
	TEST_FUNC(test_mpi_cmp),
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

#define TEST_SUITE(suite) { #suite, NULL, NULL, suite }

CU_SuiteInfo test_suites[] = {
	TEST_SUITE(mp_digit_tests),
	TEST_SUITE(mp_conversion_tests),
	TEST_SUITE(mp_basic_tests),
	TEST_SUITE(mpi_basic_tests),
	TEST_SUITE(base64_tests),
	CU_SUITE_INFO_NULL
};

int
main(void)
{
	CU_initialize_registry();
	CU_register_suites(test_suites);
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

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
#else
	_mp_digit_mul(16000000000000000000ULL, 16000000000000000000ULL, &hi, &lo);
	CU_ASSERT_EQUAL(hi, 13877787807814456755ULL);
	CU_ASSERT_EQUAL(lo, 5449091666327633920ULL);
#endif
}

void test_mp_digit_div()
{
	mp_digit q, r;
	_mp_digit_div(47, 129416588, 1000000000, &q, &r);
#if MP_DIGIT_SIZE == 4
	CU_ASSERT_EQUAL(q, 201U);
	CU_ASSERT_EQUAL(r, 992879500U);
#elif MP_DIGIT_SIZE == 8
	CU_ASSERT_EQUAL(q, 866996971464ULL);
	CU_ASSERT_EQUAL(r, 478342540ULL);
#endif
}

void test_mp_digit_invert()
{
	for (int i = 0; i < 10000; ++i) {
		mp_digit r;
		mp_rand(&r, 1);
		r |= 1; /* Ensure r is odd. */
		mp_digit inverse = mp_digit_invert(r);
		CU_ASSERT_EQUAL(r * inverse, (mp_digit)1);
	}
}

void test_mp_digit_log2()
{
	for (unsigned i = 0; i < 10000; ++i) {
		mp_digit r;
		mp_rand(&r, 1);
		r |= ((mp_digit)1) << (MP_DIGIT_BITS - 1); /* Ensure MSB is set. */
		unsigned log2 = i % MP_DIGIT_BITS;
		r >>= log2;
		CU_ASSERT_EQUAL(mp_digit_log2(r), MP_DIGIT_BITS - 1 - log2);
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
	mp_free(m);
}

void
test_mp_add_n()
{
	const int N = 100;
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
	const int N = 100;
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
	const int N = 100;
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
	CU_ASSERT_NSTRING_EQUAL(original, "A", 1);
	free(original);

	original = base64_decode("QVM=", &output_size);
	CU_ASSERT_EQUAL(output_size, 2);
	CU_ASSERT_PTR_NOT_NULL(original);
	CU_ASSERT_NSTRING_EQUAL(original, "AS", 2);
	free(original);

	original = base64_decode("QVNE", &output_size);
	CU_ASSERT_EQUAL(output_size, 3);
	CU_ASSERT_PTR_NOT_NULL(original);
	CU_ASSERT_NSTRING_EQUAL(original, "ASD", 3);
	free(original);

	original = base64_decode("QVNERg==", &output_size);
	CU_ASSERT_EQUAL(output_size, 4);
	CU_ASSERT_PTR_NOT_NULL(original);
	CU_ASSERT_NSTRING_EQUAL(original, "ASDF", 4);
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
