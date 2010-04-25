/*
 * mpi_crt.c
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mpi.h"

#define X	(ctx->x)
#define M	(ctx->m)

void
mpi_crt_init(mpi_crt_ctx *ctx)
{
	ctx->i = 0;
}

int
mpi_crt_step(mpi_crt_ctx *ctx, const mpi *a_i, const mpi *m_i)
{
	if (ctx->i == 0) {
		mpi_init_mpi(X, a_i);
		mpi_init_mpi(M, m_i);
	} else {
		mpi_t u, v, gcd;

		mpi_init(u);
		mpi_init(v);
		mpi_init(gcd);

		mpi_gcdext(M, m_i, u, v, gcd);
		if (!mpi_is_one(gcd)) {
			mpi_free(u);
			mpi_free(v);
			mpi_free(gcd);
			mpi_free(X);
			mpi_free(M);
			ctx->i = 0;
			return -1;
		}

		mpi_mul(u, M, u);
		mpi_mul(u, a_i, u);
		mpi_mul(v, m_i, v);
		mpi_mul(v, X, v);
		mpi_add(u, v, X);
		mpi_mul(M, m_i, M);
		mpi_mod(X, M, X);

		mpi_free(u);
		mpi_free(v);
		mpi_free(gcd);
	}
	ctx->i++;
	return 0;
}

int
mpi_crt_finish(mpi_crt_ctx *ctx, mpi *a)
{
	if (ctx->i == 0)
		return -1;
	if (mpi_is_neg(X))
		mpi_add(X, M, a);
	else
		mpi_set_mpi(a, X);
	mpi_free(X);
	mpi_free(M);
	return 0;
}
