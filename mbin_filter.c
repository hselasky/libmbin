/*-
 * Copyright (c) 2014 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * This file contains functions to quickly compute the Nth version of
 * any linear filter with or without feedback.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "math_bin.h"

int
mbin_filter_table_d(uint32_t n, const double *input, double *output)
{
	const uint32_t s = (n * n);
	double bitmap[s][s];
	double value[s][n];
	double m;

	uint32_t x;
	uint32_t y;
	uint32_t u;
	uint32_t t;

	memset(bitmap, 0, sizeof(bitmap));
	memset(value, 0, sizeof(value));
	memset(output, 0, sizeof(output[0]) * s * n);

	/* build equation set */
	for (x = 0; x != n; x++) {
		for (y = 0; y != n; y++) {
			memcpy(value[x + (y * n)], input + ((x + y) * n), sizeof(value[0]));

			for (t = 0; t != n; t++) {
				for (u = 0; u != n; u++) {
					bitmap[x + (y * n)]
					    [t + (u * n)] = input[t + (x * n)] *
					    input[u + (y * n)];
				}
			}
		}
	}

	/* solve equation set */
	for (x = 0; x != s; x++) {
		for (y = 0; y != s; y++) {
			m = bitmap[x][y];
			if (m == 0.0)
				continue;

			for (u = 0; u != s; u++)
				bitmap[x][u] /= m;
			for (u = 0; u != n; u++)
				value[x][n] /= m;

			bitmap[x][y] = 1.0;

			for (u = 0; u != s; u++) {
				if (u == x)
					continue;
				m = bitmap[u][y];
				if (m == 0.0)
					continue;
				for (t = 0; t != s; t++)
					bitmap[u][t] -= bitmap[x][t] * m;
				for (t = 0; t != n; t++)
					value[u][t] -= value[x][t] * m;

				bitmap[u][y] = 0.0;
			}
			break;
		}
		if (y == s)
			return (-1);
	}

	/* sort solution */
	for (x = 0; x != s; x++) {
		for (u = y = 0; y != s; y++)
			u += (bitmap[x][y] != 0.0);

		if (u != 1)
			return (-1);

		for (y = 0; y != s; y++) {
			if (bitmap[x][y] == 0.0)
				continue;
			memcpy(output + (y * n), value[x], sizeof(value[0]));
			break;
		}
	}
	return (0);
}

int
mbin_filter_table_alloc_d(uint32_t n, mbin_filter_d_fn_t *fn, void *arg, double **ppout)
{
	double input[2 * n][n];
	double value[n];
	uint32_t x;

	mbin_filter_impulse_d(value, n);

	*ppout = malloc(sizeof(double) * n * n * n);

	if (*ppout == NULL)
		return (-1);

	for (x = 0; x != 2 * n; x++) {
		memcpy(input[x], value, sizeof(value));
		fn(value, n, arg);
	}

	if (mbin_filter_table_d(n, input[0], *ppout) != 0) {
		free(*ppout);
		return (-1);
	}
	return (0);
}

void
mbin_filter_table_free_d(double *pout)
{
	free(pout);
}

void
mbin_filter_mul_d(const double *a, const double *b, double *c,
    const double *table, uint32_t n)
{
	uint32_t x;
	uint32_t y;
	uint32_t z;

	memset(c, 0, sizeof(c[0]) * n);

	for (x = 0; x != n; x++) {
		for (y = 0; y != n; y++) {
			double f = a[x] * b[y];

			if (f == 0.0)
				continue;
			uint32_t off = (x * n) + (y * n * n);

			for (z = 0; z != n; z++)
				c[z] += table[off + z] * f;
		}
	}
}

void
mbin_filter_exp_d(const double *base, uint64_t exp,
    double *c, const double *ptable, uint32_t n)
{
	double d[n];
	double e[n];

	memcpy(d, base, sizeof(d));

	mbin_filter_impulse_d(c, n);

	while (1) {
		if (exp & 1) {
			mbin_filter_mul_d(c, d, e, ptable, n);
			memcpy(c, e, sizeof(c[0]) * n);
		}
		/* get next exponent bit, if any */
		if ((exp /= 2) == 0)
			break;

		/* square */
		mbin_filter_mul_d(d, d, e, ptable, n);
		memcpy(d, e, sizeof(d));
	}
}

void
mbin_filter_impulse_d(double *ptr, uint32_t n)
{
	memset(ptr, 0, sizeof(ptr[0]) * n);
	ptr[0] = 1.0;
}
