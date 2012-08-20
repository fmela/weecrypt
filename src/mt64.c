/* Adapted from original 64-bit Mersenne Twister, downloaded from:
	http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
*/
/*
   A C-program for MT19937-64 (2004/9/29 version).
   Coded by Takuji Nishimura and Makoto Matsumoto.

   This is a 64-bit version of Mersenne Twister pseudorandom number
   generator.

   Before using, initialize the state by using init_genrand64(seed)
   or init_by_array64(init_key, key_length).

   Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   References:
   T. Nishimura, ``Tables of 64-bit Mersenne Twisters''
     ACM Transactions on Modeling and
     Computer Simulation 10. (2000) 348--357.
   M. Matsumoto and T. Nishimura,
     ``Mersenne Twister: a 623-dimensionally equidistributed
       uniform pseudorandom number generator''
     ACM Transactions on Modeling and
     Computer Simulation 8. (Jan. 1998) 3--30.

   Any feedback is very welcome.
   http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove spaces)
*/

#include "mt64.h"

#define NN			312
#define MM			156
#define MATRIX_A	UINT64_C(0xB5026F5AA96619E9)
#define UM			UINT64_C(0xFFFFFFFF80000000) /* Most significant 33 bits */
#define LM			UINT64_C(0x7FFFFFFF)		 /* Least significant 31 bits */

void mt64_init_u64(mt64_context *ctx, uint64_t seed)
{
    ctx->mt[0] = seed;
    for (ctx->mti=1; ctx->mti<NN; ctx->mti++)
        ctx->mt[ctx->mti] = (UINT64_C(6364136223846793005) *
			(ctx->mt[ctx->mti-1] ^ (ctx->mt[ctx->mti-1] >> 62)) + ctx->mti);
}

void mt64_init_u64_array(mt64_context *ctx,
						 const uint64_t seed[], unsigned seed_size)
{
    mt64_init_u64(ctx, UINT64_C(19650218));
    uint64_t i=1, j=0;
    uint64_t k = (NN>seed_size ? NN : seed_size);
    for (; k; k--) {
        ctx->mt[i] = (ctx->mt[i] ^ ((ctx->mt[i-1] ^ (ctx->mt[i-1] >> 62)) *
									UINT64_C(3935559000370003845)))
			+ seed[j] + j; /* non linear */
        i++; j++;
        if (i>=NN) { ctx->mt[0] = ctx->mt[NN-1]; i=1; }
        if (j>=seed_size) j=0;
    }
    for (k=NN-1; k; k--) {
        ctx->mt[i] = (ctx->mt[i] ^ ((ctx->mt[i-1] ^ (ctx->mt[i-1] >> 62)) *
									UINT64_C(2862933555777941757)))
			- i; /* non linear */
        i++;
        if (i>=NN) { ctx->mt[0] = ctx->mt[NN-1]; i=1; }
    }

    ctx->mt[0] = 1ULL << 63; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0, 2^64-1]-interval */
uint64_t mt64_gen_u64(mt64_context *ctx)
{
    static const uint64_t mag01[2]={0, MATRIX_A};

    uint64_t x;
    if (ctx->mti >= NN) { /* generate NN words at one time */

        /* if init_genrand64() has not been called, */
        /* a default initial seed is used     */
        if (ctx->mti == NN+1)
            mt64_init_u64(ctx, UINT64_C(5489));

		int i;
        for (i=0;i<NN-MM;i++) {
            x = (ctx->mt[i]&UM)|(ctx->mt[i+1]&LM);
            ctx->mt[i] = ctx->mt[i+MM] ^ (x>>1) ^ mag01[(int)(x&1)];
        }
        for (;i<NN-1;i++) {
            x = (ctx->mt[i]&UM)|(ctx->mt[i+1]&LM);
            ctx->mt[i] = ctx->mt[i+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&1)];
        }
        x = (ctx->mt[NN-1]&UM)|(ctx->mt[0]&LM);
        ctx->mt[NN-1] = ctx->mt[MM-1] ^ (x>>1) ^ mag01[(int)(x&1)];

        ctx->mti = 0;
    }

    x = ctx->mt[ctx->mti++];

    x ^= (x >> 29) & UINT64_C(0x5555555555555555);
    x ^= (x << 17) & UINT64_C(0x71D67FFFEDA60000);
    x ^= (x << 37) & UINT64_C(0xFFF7EEE000000000);
    x ^= (x >> 43);

    return x;
}
