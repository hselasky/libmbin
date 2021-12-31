/*-
 * Copyright (c) 2014-2021 Hans Petter Selasky. All rights reserved.
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
#include <math.h>

#include "math_bin.h"

#include "math_bin_complex.h"

#define	MBIN_FILTER_SIZE(n) ((((n) * (n)) + (n)) / 2)

int
mbin_filter_table_d(uint32_t n, const double *input, double *output, double zero)
{
	const uint32_t sx = 2 * MBIN_FILTER_SIZE(n);
	const uint32_t sy = MBIN_FILTER_SIZE(n);
	double bitmap[sx][sy];
	double value[sx][n];
	uint8_t clean[sx];
	double m;

	uint32_t x;
	uint32_t y;
	uint32_t u;
	uint32_t t;
	uint32_t j;
	uint32_t k;

	memset(bitmap, 0, sizeof(bitmap));
	memset(value, 0, sizeof(value));
	memset(output, 0, sizeof(output[0]) * sy * n);
	memset(clean, 0, sizeof(clean));

	/* build equation set */
	for (j = x = 0; x != n; x++) {
		for (y = x; (x + y) != (2 * n); y++, j++) {
			memcpy(value[j], input + ((x + y) * n), sizeof(double) * n);

			for (k = t = 0; t != n; t++) {
				for (u = t; u != n; u++, k++) {
					bitmap[j][k] =
					    ((input[t + (x * n)] * input[u + (y * n)]) +
					    (input[u + (x * n)] * input[t + (y * n)]));
				}
			}
		}
	}
repeat:
	/* solve equation set */
	for (x = 0; x != sx; x++) {

		if (clean[x] != 0)
			continue;

		clean[x] = 1;

		for (y = u = 0; u != sy; u++) {
			if (fabs(bitmap[x][y]) < fabs(bitmap[x][u]))
				y = u;
		}
		m = bitmap[x][y];
		if (fabs(m) <= zero) {
			for (y = 0; y != n; y++) {
				if (fabs(value[x][y]) > zero)
					return (-1);
			}
			continue;
		}
		for (u = 0; u != sy; u++)
			bitmap[x][u] /= m;
		for (u = 0; u != n; u++)
			value[x][u] /= m;

		bitmap[x][y] = 1.0;

		for (u = 0; u != sx; u++) {
			if (u == x)
				continue;
			m = bitmap[u][y];
			if (fabs(m) <= zero)
				continue;
			for (t = 0; t != sy; t++)
				bitmap[u][t] -= bitmap[x][t] * m;
			for (t = 0; t != n; t++)
				value[u][t] -= value[x][t] * m;

			bitmap[u][y] = 0.0;
			clean[u] = 0;
		}
	}

	/* sort solution */
	for (x = 0; x != sx; x++) {
		for (u = y = 0; y != sy; y++)
			u += (fabs(bitmap[x][y]) > zero);

		if (u != 1) {
			if (u == 0) {
				for (y = 0; y != n; y++) {
					if (fabs(value[x][y]) > zero)
						return (-1);
				}
				continue;
			}
			for (u = y = 0; y != sy; y++) {
				u += (fabs(bitmap[x][y]) > zero);
				if (u == 2) {
					/*
					 * more than one variable, set to
					 * zero
					 */
					for (x = 0; x != sx; x++) {
						if (fabs(bitmap[x][y]) <= zero)
							continue;
						bitmap[x][y] = 0.0;
						clean[x] = 0;
					}
					goto repeat;
				}
			}
			return (-1);
		}
		for (y = 0; y != sx; y++) {
			if (fabs(bitmap[x][y]) <= zero)
				continue;
			memcpy(output + (y * n), value[x], sizeof(double) * n);
			break;
		}
	}
	return (0);
}

int
mbin_filter_table_alloc_d(uint32_t n, mbin_filter_d_fn_t *fn, void *arg, double **ppout, double zero)
{
	const uint32_t s = MBIN_FILTER_SIZE(n);
	double input[2 * n][n];
	double value[n];
	uint32_t x;

	*ppout = malloc(sizeof(double) * ((s * n) + n));

	if (*ppout == NULL)
		return (-1);

	for (x = 0; x != 2 * n; x++) {
		fn(value, x, n, arg);
		memcpy(input[x], value, sizeof(value));
		if (x == 0)
			memcpy(*ppout + (s * n), value, sizeof(value));
	}

	if (mbin_filter_table_d(n, input[0], *ppout, zero) != 0) {
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
		for (y = x; y != n; y++) {
			double f = (a[x] * b[y]) + (b[x] * a[y]);

			if (f != 0.0) {
				for (z = 0; z != n; z++)
					c[z] += table[z] * f;
			}
			table += n;
		}
	}
}

void
mbin_filter_exp_d(const double *base, uint64_t exp,
    double *c, const double *ptable, uint32_t n)
{
	const uint32_t s = MBIN_FILTER_SIZE(n);
	double d[n];
	double e[n];

	memcpy(d, base, sizeof(d));
	memcpy(c, ptable + (s * n), sizeof(d));

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

#define	U64(m) ((uint64_t)(m))

int
mbin_filter_table_p_32(uint32_t n, const uint32_t mod,
    const uint32_t *input, uint32_t *output)
{
	const uint32_t sx = 2 * MBIN_FILTER_SIZE(n);
	const uint32_t sy = MBIN_FILTER_SIZE(n);
	uint32_t bitmap[sx][sy];
	uint32_t value[sx][n];
	uint8_t clean[sx];
	uint32_t m;
	uint32_t k;

	uint32_t x;
	uint32_t y;
	uint32_t u;
	uint32_t t;
	uint32_t j;

	memset(bitmap, 0, sizeof(bitmap));
	memset(value, 0, sizeof(value));
	memset(output, 0, sizeof(output[0]) * sy * n);
	memset(clean, 0, sizeof(clean));

	/* build equation set */
	for (j = x = 0; x != n; x++) {
		for (y = x; (x + y) != (2 * n); y++, j++) {
			memcpy(value[j], input + ((x + y) * n), sizeof(uint32_t) * n);

			for (k = t = 0; t != n; t++) {
				for (u = t; u != n; u++, k++) {
					uint32_t a, b, c, d;

					a = t + (x * n);
					b = u + (y * n);
					c = u + (x * n);
					d = t + (y * n);

					if (t == u) {
						bitmap[j][k] =
						    (U64(input[a]) * U64(input[b])) % U64(mod);
					} else {
						bitmap[j][k] =
						    ((U64(input[a]) * U64(input[b])) +
						    (U64(input[c]) * U64(input[d]))) % U64(mod);
					}
				}
			}
		}
	}

repeat:
	/* solve equation set */
	for (x = 0; x != sx; x++) {

		if (clean[x] != 0)
			continue;

		clean[x] = 1;

		for (y = 0; y != sy; y++) {
			m = bitmap[x][y];
			if (m != 0)
				break;
		}
		if (y == sy) {
			for (y = 0; y != n; y++) {
				if (value[x][y] != 0) {
					return (-1);
				}
			}
			continue;
		}
		k = mbin_power_mod_32(m, mod - 2, mod);

		for (u = 0; u != sy; u++)
			bitmap[x][u] = (U64(bitmap[x][u]) * U64(k)) % U64(mod);
		for (u = 0; u != n; u++)
			value[x][u] = (U64(value[x][u]) * U64(k)) % U64(mod);

		for (u = 0; u != sx; u++) {
			if (u == x)
				continue;
			m = bitmap[u][y];
			if (m == 0)
				continue;
			for (t = 0; t != sy; t++) {
				bitmap[u][t] = (U64(mod) + U64(bitmap[u][t]) -
				    ((U64(bitmap[x][t]) * U64(m)) % U64(mod))) % U64(mod);
			}
			for (t = 0; t != n; t++) {
				value[u][t] = (U64(mod) + U64(value[u][t]) -
				    ((U64(value[x][t]) * U64(m)) % U64(mod))) % U64(mod);
			}
			bitmap[u][y] = 0;
			clean[u] = 0;
		}
	}

	/* sort solution */
	for (x = 0; x != sx; x++) {
		for (u = y = 0; y != sy; y++)
			u += (bitmap[x][y] != 0);

		if (u != 1) {
			if (u == 0) {
				for (y = 0; y != n; y++) {
					if (value[x][y] != 0) {
						return (-1);
					}
				}
				continue;
			}
			for (u = y = 0; y != sy; y++) {
				u += (bitmap[x][y] != 0);
				if (u == 2) {
					/*
					 * more than one variable, set to
					 * zero
					 */
					for (x = 0; x != sx; x++) {
						if (bitmap[x][y] == 0)
							continue;
						bitmap[x][y] = 0;
						clean[x] = 0;
					}
					goto repeat;
				}
			}
			return (-1);
		}
		for (y = 0; y != sx; y++) {
			if (bitmap[x][y] == 0)
				continue;
			memcpy(output + (y * n), value[x], sizeof(uint32_t) * n);
			break;
		}
	}
	return (0);
}

int
mbin_filter_table_alloc_p_32(uint32_t n, const uint32_t mod, mbin_filter_p_32_fn_t *fn, void *arg, uint32_t **ppout)
{
	const uint32_t s = MBIN_FILTER_SIZE(n);
	uint32_t input[2 * n][n];
	uint32_t value[n];
	uint32_t x;

	*ppout = malloc(sizeof(uint32_t) * ((s * n) + n));

	if (*ppout == NULL)
		return (-1);

	for (x = 0; x != 2 * n; x++) {
		fn(value, x, n, mod, arg);
		memcpy(input[x], value, sizeof(value));
		if (x == 0)
			memcpy(*ppout + (s * n), value, sizeof(value));
	}

	if (mbin_filter_table_p_32(n, mod, input[0], *ppout) != 0) {
		free(*ppout);
		return (-1);
	}
	return (0);
}

void
mbin_filter_table_free_p_32(uint32_t *pout)
{
	free(pout);
}

void
mbin_filter_mul_p_32(const uint32_t *pa, const uint32_t *pb, uint32_t *c,
    const uint32_t *table, const uint32_t mod, uint32_t n)
{
	uint32_t x;
	uint32_t y;
	uint32_t z;

	memset(c, 0, sizeof(uint32_t) * n);

	for (x = 0; x != n; x++) {
		for (y = x; y != n; y++) {
			uint32_t f;

			if (x == y) {
				f = (U64(pa[x]) * U64(pb[y])) % U64(mod);
			} else {
				f = (U64(pa[x]) * U64(pb[y]) +
				    U64(pb[x]) * U64(pa[y])) % U64(mod);
			}
			if (f != 0) {
				for (z = 0; z != n; z++)
					c[z] = (U64(c[z]) + U64(table[z]) * f) % U64(mod);
			}
			table += n;
		}
	}
}

void
mbin_filter_exp_p_32(const uint32_t *base, uint64_t exp,
    uint32_t *c, const uint32_t *ptable, uint32_t n, const uint32_t mod)
{
	const uint32_t s = MBIN_FILTER_SIZE(n);
	uint32_t d[n];
	uint32_t e[n];

	memcpy(d, base, sizeof(d));
	memcpy(c, ptable + (s * n), sizeof(d));

	while (1) {
		if (exp & 1) {
			mbin_filter_mul_p_32(c, d, e, ptable, mod, n);
			memcpy(c, e, sizeof(c[0]) * n);
		}
		/* get next exponent bit, if any */
		if ((exp /= 2) == 0)
			break;

		/* square */
		mbin_filter_mul_p_32(d, d, e, ptable, mod, n);
		memcpy(d, e, sizeof(d));
	}
}

void
mbin_filter_impulse_p_32(uint32_t *ptr, uint32_t n)
{
	memset(ptr, 0, sizeof(ptr[0]) * n);
	ptr[0] = 1;
}

int
mbin_xor2_filter_table_p_64(uint64_t n, const uint64_t p,
    const uint64_t *input, uint64_t *output)
{
	const uint32_t sx = 2 * MBIN_FILTER_SIZE(n);
	const uint32_t sy = MBIN_FILTER_SIZE(n);
	uint64_t bitmap[sx][sy];
	uint64_t value[sx][n];
	uint8_t clean[sx];
	uint64_t m;

	uint64_t x;
	uint64_t y;
	uint64_t u;
	uint64_t t;
	uint64_t k;
	uint64_t j;

	memset(bitmap, 0, sizeof(bitmap));
	memset(value, 0, sizeof(value));
	memset(output, 0, sizeof(output[0]) * sy * n);
	memset(clean, 0, sizeof(clean));

	/* build equation set */
	for (j = x = 0; x != n; x++) {
		for (y = x; (x + y) != (2 * n); y++, j++) {
			memcpy(value[j], input + ((x + y) * n), sizeof(uint64_t) * n);

			for (k = t = 0; t != n; t++) {
				for (u = t; u != n; u++, k++) {
					uint32_t a, b, c, d;

					a = t + (x * n);
					b = u + (y * n);
					c = u + (x * n);
					d = t + (y * n);

					if (t == u) {
						bitmap[j][k] = mbin_xor2_mul_mod_64(input[a], input[b], p);
					} else {
						bitmap[j][k] =
						    mbin_xor2_mul_mod_64(input[a], input[b], p) ^
						    mbin_xor2_mul_mod_64(input[c], input[d], p);
					}
				}
			}
		}
	}
repeat:
	/* solve equation set */
	for (x = 0; x != sx; x++) {

		if (clean[x] != 0)
			continue;

		clean[x] = 1;

		for (y = 0; y != sy; y++) {
			m = bitmap[x][y];
			if (m == 0)
				continue;
			break;
		}
		if (y == sy) {
			for (y = 0; y != n; y++) {
				if (value[x][y] != 0)
					return (-1);
			}
			continue;
		}
		if (m != 0)
			m = mbin_xor2_neg_mod_64(m, p);

		for (u = 0; u != sy; u++)
			bitmap[x][u] = mbin_xor2_mul_mod_64(bitmap[x][u], m, p);
		for (u = 0; u != n; u++)
			value[x][u] = mbin_xor2_mul_mod_64(value[x][u], m, p);

		for (u = 0; u != sx; u++) {
			if (u == x)
				continue;
			m = bitmap[u][y];
			if (m == 0)
				continue;
			for (t = 0; t != sy; t++) {
				bitmap[u][t] ^=
				    mbin_xor2_mul_mod_64(bitmap[x][t], m, p);
			}
			for (t = 0; t != n; t++) {
				value[u][t] ^=
				    mbin_xor2_mul_mod_64(value[x][t], m, p);
			}
			bitmap[u][y] = 0;
			clean[u] = 0;
		}
	}

	/* sort solution */
	for (x = 0; x != sx; x++) {
		for (u = y = 0; y != sy; y++)
			u += (bitmap[x][y] != 0);

		if (u != 1) {
			if (u == 0) {
				for (y = 0; y != n; y++) {
					if (value[x][y] != 0)
						return (-1);
				}
				continue;
			}
			for (u = y = 0; y != sy; y++) {
				u += (bitmap[x][y] != 0);
				if (u == 2) {
					/*
					 * more than one variable, set to
					 * zero
					 */
					for (x = 0; x != sx; x++) {
						if (bitmap[x][y] == 0)
							continue;
						bitmap[x][y] = 0;
						clean[x] = 0;
					}
					goto repeat;
				}
			}
			return (-1);
		}
		for (y = 0; y != sx; y++) {
			if (bitmap[x][y] == 0)
				continue;
			memcpy(output + (y * n), value[x], sizeof(uint64_t) * n);
			break;
		}
	}
	return (0);
}

int
mbin_xor2_filter_table_alloc_p_64(uint64_t n, uint64_t mod, mbin_xor2_filter_p_64_fn_t *fn, void *arg, uint64_t **ppout)
{
	const uint32_t s = MBIN_FILTER_SIZE(n);
	uint64_t input[2 * n][n];
	uint64_t value[n];
	uint64_t x;

	*ppout = malloc(sizeof(uint64_t) * ((n * s) + n));

	if (*ppout == NULL)
		return (-1);

	for (x = 0; x != 2 * n; x++) {
		fn(value, x, n, mod, arg);
		memcpy(input[x], value, sizeof(value));
		if (x == 0)
			memcpy(*ppout + (s * n), value, sizeof(value));
	}

	if (mbin_xor2_filter_table_p_64(n, mod, input[0], *ppout) != 0) {
		free(*ppout);
		return (-1);
	}
	return (0);
}

void
mbin_xor2_filter_table_free_p_64(uint64_t *pout)
{
	free(pout);
}

void
mbin_xor2_filter_mul_p_64(const uint64_t *a, const uint64_t *b, uint64_t *c,
    const uint64_t *table, uint64_t p, uint64_t n)
{
	uint64_t x;
	uint64_t y;
	uint64_t z;

	memset(c, 0, sizeof(uint64_t) * n);

	for (x = 0; x != n; x++) {
		for (y = x; y != n; y++) {
			uint64_t f;

			if (x == y) {
				f = mbin_xor2_mul_mod_64(a[x], b[y], p);
			} else {
				f = mbin_xor2_mul_mod_64(a[x], b[y], p) ^
				    mbin_xor2_mul_mod_64(b[x], a[y], p);
			}
			if (f != 0) {
				for (z = 0; z != n; z++) {
					c[z] ^= mbin_xor2_mul_mod_64(table[z], f, p);
				}
			}
			table += n;
		}
	}
}

void
mbin_xor2_filter_exp_p_64(const uint64_t *base, uint64_t exp,
    uint64_t *c, const uint64_t *ptable, uint64_t n, uint64_t mod)
{
	const uint32_t s = MBIN_FILTER_SIZE(n);
	uint64_t d[n];
	uint64_t e[n];

	memcpy(d, base, sizeof(d));
	memcpy(c, ptable + (s * n), sizeof(d));

	while (1) {
		if (exp & 1) {
			mbin_xor2_filter_mul_p_64(c, d, e, ptable, mod, n);
			memcpy(c, e, sizeof(c[0]) * n);
		}
		/* get next exponent bit, if any */
		if ((exp /= 2) == 0)
			break;

		/* square */
		mbin_xor2_filter_mul_p_64(d, d, e, ptable, mod, n);
		memcpy(d, e, sizeof(d));
	}
}

void
mbin_xor2_filter_impulse_p_64(uint64_t *ptr, uint64_t n)
{
	memset(ptr, 0, sizeof(ptr[0]) * n);
	ptr[0] = 1;
}

int
mbin_filter_table_cd(uint32_t n, const cd_t *input, cd_t *output)
{
	const uint32_t s = (n * n);
	cd_t bitmap[s][s];
	cd_t value[s][n];
	cd_t m;

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
			memcpy(value[x + (y * n)], input + ((x + y) * n),
			    sizeof(value[0]));

			for (t = 0; t != n; t++) {
				for (u = 0; u != n; u++) {
					bitmap[x + (y * n)][t + (u * n)] =
					    cd_mul(input[t + (x * n)],
						   input[u + (y * n)]);
				}
			}
		}
	}
repeat:
	/* solve equation set */
	for (x = 0; x != s; x++) {
		for (y = 0; y != s; y++) {
			m = bitmap[x][y];
			if (m.x != 0.0 || m.y != 0.0)
				break;
		}
		if (y == s) {
			for (y = 0; y != n; y++) {
				if (value[x][y].x != 0.0 || value[x][y].y != 0.0)
					return (-1);
			}
			continue;
		}
		for (u = 0; u != s; u++) {
			bitmap[x][u] =
			    cd_div(bitmap[x][u], m);
		}
		for (u = 0; u != n; u++) {
			value[x][u] =
			    cd_div(value[x][u], m);
		}
		bitmap[x][y].x = 1.0;
		bitmap[x][y].y = 0.0;

		for (u = 0; u != s; u++) {
			if (u == x)
				continue;
			m = bitmap[u][y];
			if (m.x == 0.0 && m.y == 0.0)
				continue;
			for (t = 0; t != s; t++) {
				bitmap[u][t] = cd_sub(bitmap[u][t], cd_mul(bitmap[x][t], m));
			}
			for (t = 0; t != n; t++) {
				value[u][t] = cd_sub(value[u][t], cd_mul(value[x][t], m));
			}
			bitmap[u][y].x = 0.0;
			bitmap[u][y].y = 0.0;
		}
	}

	/* sort solution */
	for (x = 0; x != s; x++) {
		for (u = y = 0; y != s; y++)
			u += (bitmap[x][y].x != 0.0 || bitmap[x][y].y != 0.0);

		if (u != 1) {
			if (u == 0) {
				for (y = 0; y != n; y++) {
					if (value[x][y].x != 0.0 || value[x][y].y != 0.0)
						return (-1);
				}
				continue;
			}
			for (u = y = 0; y != s; y++) {
				u += (bitmap[x][y].x != 0.0 || bitmap[x][y].y != 0.0);
				if (u == 2) {
					/*
					 * more than one variable, set to
					 * zero
					 */
					for (x = 0; x != s; x++) {
						bitmap[x][y].x = 0.0;
						bitmap[x][y].y = 0.0;
					}
					goto repeat;
				}
			}
			return (-1);
		}
		for (y = 0; y != s; y++) {
			if (bitmap[x][y].x == 0.0 && bitmap[x][y].y == 0.0)
				continue;
			memcpy(output + (y * n), value[x], sizeof(value[0]));
			break;
		}
	}
	return (0);
}

int
mbin_filter_table_alloc_cd(uint32_t n, mbin_filter_cd_fn_t *fn, void *arg, cd_t **ppout)
{
	cd_t input[2 * n][n];
	cd_t value[n];
	uint32_t x;

	*ppout = malloc(sizeof(cd_t) * (n * n * n + n));

	if (*ppout == NULL)
		return (-1);

	for (x = 0; x != 2 * n; x++) {
		fn(value, x, n, arg);
		memcpy(input[x], value, sizeof(value));
		if (x == 0)
			memcpy(*ppout + (n * n * n), value, sizeof(value));
	}

	if (mbin_filter_table_cd(n, input[0], *ppout) != 0) {
		free(*ppout);
		return (-1);
	}
	return (0);
}

void
mbin_filter_table_free_cd(cd_t *pout)
{
	free(pout);
}

void
mbin_filter_mul_cd(const cd_t *a, const cd_t *b, cd_t *c,
    const cd_t *table, uint32_t n)
{
	uint32_t x;
	uint32_t y;
	uint32_t z;

	memset(c, 0, sizeof(c[0]) * n);

	for (x = 0; x != n; x++) {
		for (y = 0; y != n; y++) {
			cd_t f = cd_mul(a[x], b[y]);

			if (f.x == 0.0 && f.y == 0)
				continue;

			uint32_t off = (x * n) + (y * n * n);

			for (z = 0; z != n; z++) {
				c[z] = cd_add(c[z], cd_mul(table[off + z], f));
			}
		}
	}
}

void
mbin_filter_exp_cd(const cd_t *base, uint64_t exp,
    cd_t *c, const cd_t *ptable, uint32_t n)
{
	cd_t d[n];
	cd_t e[n];

	memcpy(d, base, sizeof(d));
	memcpy(c, ptable + (n * n * n), sizeof(d));

	while (1) {
		if (exp & 1) {
			mbin_filter_mul_cd(c, d, e, ptable, n);
			memcpy(c, e, sizeof(c[0]) * n);
		}
		/* get next exponent bit, if any */
		if ((exp /= 2) == 0)
			break;

		/* square */
		mbin_filter_mul_cd(d, d, e, ptable, n);
		memcpy(d, e, sizeof(d));
	}
}

void
mbin_filter_impulse_cd(cd_t *ptr, uint32_t n)
{
	memset(ptr, 0, sizeof(ptr[0]) * n);
	ptr[0].x = 1.0;
}
