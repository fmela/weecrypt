/*
 * mpi_crt.c
 * Copyright (C) 2003-2010 Farooq Mela. All rights reserved.
 *
 * $Id$
 */

#include "mpi.h"

void
mpi_crt_init(mpi_crt_ctx *ctx)
{
	ctx->i = 0;
}

int
mpi_crt_step(mpi_crt_ctx *ctx, const mpi *a_i, const mpi *m_i)
{
	if (ctx->i == 0) {
		mpi_init_mpi(ctx->x, a_i);
		mpi_init_mpi(ctx->m, m_i);
	} else {
		mpi_t u, v, gcd;

		mpi_init(u);
		mpi_init(v);
		mpi_init(gcd);

		mpi_gcdext(ctx->m, m_i, u, v, gcd);
		if (!mpi_is_one(gcd)) {
			mpi_free(u);
			mpi_free(v);
			mpi_free(gcd);
			mpi_free(ctx->x);
			mpi_free(ctx->m);
			ctx->i = 0;
			return -1;
		}

		mpi_mul(u, ctx->m, u);
		mpi_mul(u, a_i, u);
		mpi_mul(v, m_i, v);
		mpi_mul(v, ctx->x, v);
		mpi_add(u, v, ctx->x);
		mpi_mul(ctx->m, m_i, ctx->m);
		mpi_mod(ctx->x, ctx->m, ctx->x);

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
	if (mpi_is_neg(ctx->x))
		mpi_add(ctx->x, ctx->m, a);
	else
		mpi_set_mpi(a, ctx->x);
	mpi_free(ctx->x);
	mpi_free(ctx->m);
	return 0;
}
