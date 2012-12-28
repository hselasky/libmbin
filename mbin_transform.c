/*-
 * Copyright (c) 2008-2011 Hans Petter Selasky. All rights reserved.
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "math_bin.h"

/* XOR - transform */

void
mbin_transform_xor_fwd_32x32(uint32_t *ptr, uint32_t mask,
    uint32_t f_slice, uint32_t t_slice)
{
	uint32_t x;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		ptr[x] &= ~t_slice;
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		if (ptr[x] & f_slice)
			mbin_expand_xor_32x32(ptr, x, mask, t_slice);
		if (x == mask)
			break;
		x++;
	}
}

/* XOR - transform */

void
mbin_transform_multi_xor_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask)
{
	uint32_t x;
	uint32_t val;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		if (val) {
			/* expand logic expression */
			mbin_expand_xor_32x32(temp, x, mask, val);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}

/* XOR-ADD - transform */

void
mbin_transform_add_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask)
{
	uint32_t x;
	uint32_t val;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		if (val) {
			/* expand logic expression */
			mbin_expand_add_32x32(temp, x, mask, val);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}

/* XOR-ADD-MODULUS - transform */

void
mbin_transform_add_mod_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask, uint32_t mod)
{
	uint32_t x;
	uint32_t val;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		if (val) {
			/* expand logic expression */
			mbin_expand_add_mod_32x32(temp, x, mask, val, mod);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}

/* Greater Equal, GTE - transform */

void
mbin_transform_gte_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask)
{
	uint32_t x;
	uint32_t val;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		if (val) {
			/* expand logic expression */
			mbin_expand_gte_32x32(temp, x, mask, val);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}

void
mbin_transform_xor_gte_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask)
{
	uint32_t x;
	uint32_t val;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		if (val) {
			/* expand logic expression */
			mbin_expand_xor_gte_32x32(temp, x, mask, val);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}

/* Sum Of Sums, SOS - transform (slow version) */

void
mbin_transform_sos_inv_32x32(uint32_t *ptr, uint8_t lmax)
{
	const uint32_t max = 1U << lmax;
	uint32_t *k = malloc(sizeof(k[0]) * max);
	uint32_t *t = malloc(sizeof(t[0]) * max);
	uint32_t x;
	uint32_t y;

	for (x = 0; x != max; x++) {
		k[x] = mbin_coeff_32(max, x);
		if (x & 1)
			k[x] = -k[x];
	}

	memset(t, 0, sizeof(t[0]) * max);

	for (x = 0; x != max; x++) {
		for (y = 0; y != (max - x); y++) {
			t[x + y] += k[y] * ptr[x];
		}
	}

	memset(ptr, 0, sizeof(ptr[0]) * max);

	for (x = 0; x != max; x++) {
		for (y = 0; y <= x; y++) {
			ptr[x - y] += mbin_coeff_32(x, x - y) * t[x];
		}
	}

	for (x = 0; x != (max / 2);) {

		y = ptr[x];
		ptr[x] = -ptr[max - 1 - x];
		ptr[max - 1 - x] = y;

		x++;

		y = -ptr[x];
		ptr[x] = ptr[max - 1 - x];
		ptr[max - 1 - x] = y;

		x++;
	}

	free(k);
	free(t);
}

/* Sum Of Sums, SOS - transform (slow version) */

void
mbin_transform_sos_fwd_32x32(uint32_t *ptr, uint8_t lmax)
{
	const uint32_t max = 1U << lmax;
	uint32_t *k = malloc(sizeof(k[0]) * max);
	uint32_t *t = malloc(sizeof(t[0]) * max);
	uint32_t x;
	uint32_t y;
	uint32_t sum;
	uint32_t val;

	memset(k, 0, sizeof(k[0]) * max);
	memset(t, 0, sizeof(t[0]) * max);

	k[0] = 1;

	for (x = 0; x != max; x++) {

		sum = 0;
		val = ptr[x];
		if (val) {
			for (y = 0; y != max; y++) {
				sum += k[y];
				t[y] += sum * val;
				k[y] = sum;
			}
		} else {
			for (y = 0; y != max; y++) {
				sum += k[y];
				k[y] = sum;
			}
		}
	}

	for (x = 0; x != max; x++)
		ptr[x] = t[x];

	free(k);
	free(t);
}

/* Sum Of Sums, under modulus, SOS - transform */

void
mbin_transform_sos_mod_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t *scratch, uint32_t mask, uint32_t mod)
{
	uint32_t x;
	uint32_t y;
	uint32_t val;
	uint32_t sum;

	x = 0;
	while (1) {
		scratch[x] = 0;
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	scratch[0] = 1;

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		sum = 0;
		if (val) {
			for (y = x;; y++) {
				sum += scratch[y - x];
				sum %= mod;
				temp[y] = (mod + temp[y] -
				    ((sum * val) % mod)) % mod;
				scratch[y - x] = sum;
				if (y == mask)
					break;
			}
			temp[x] = val;
		} else {
			for (y = x;; y++) {
				sum += scratch[y - x];
				sum %= mod;
				scratch[y - x] = sum;
				if (y == mask)
					break;
			}
		}
		if (x == mask)
			break;
		x++;
	}
}

/* Polynom transform */

void
mbin_transform_poly_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t *scratch,
    uint32_t mask)
{
	uint32_t x;
	uint32_t y;
	uint32_t val;
	uint32_t sum;

	x = 0;
	while (1) {
		scratch[x] = 1;
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		sum = 0;
		if (val) {
			for (y = x;; y++) {
				sum = scratch[y - x] * (y - x + 1);
				temp[y] -= sum * val;
				scratch[y - x] = sum;
				if (y == mask)
					break;
			}
			temp[x] = val;
		} else {
			for (y = x;; y++) {
				sum = scratch[y - x] * (y - x + 1);
				scratch[y - x] = sum;
				if (y == mask)
					break;
			}
		}
		if (x == mask)
			break;
		x++;
	}
}

void
mbin_transform_find_negative_32x1(uint32_t *ptr, uint32_t *neg,
    uint32_t mask, uint32_t slice)
{
	uint32_t x;
	uint32_t m;

	for (m = 1; (m & mask); m *= 2) {

		for (x = 0;; x++) {

			if (x & m) {
				x += m - 1;
				if (x == mask)
					break;
				continue;
			}
			if ((ptr[x] & slice) &&
			    (ptr[x ^ m] & slice) &&
			    (neg[x] == neg[x ^ m])) {

				ptr[x ^ m] ^= slice;
				neg[x ^ m] = 0;
				neg[x] ^= m;
			}
			if (x == mask)
				break;
		}
	}
}

void
mbin_transform_find_gte_32x1(uint32_t *ptr, uint32_t *gte,
    uint32_t mask, uint32_t slice)
{
	uint32_t x;
	uint32_t y;
	uint32_t msb;
	uint32_t first;
	uint32_t last;
	uint32_t z;
	uint32_t neg;

	for (x = 0; x != mask; x++) {

		if (ptr[x] & slice) {

			neg = slice;

			for (y = 1; (x + y) != mask; y++) {
				z = x + y;

				if ((neg ^ ptr[z]) & slice) {
					if (((-z) & z) != z)
						break;
					else if (y == 1)
						break;
					else
						neg ^= slice;
				}
			}

			if (y == 1)
				return;

			gte[x] ^= slice;
			gte[x + y] ^= slice;

			first = x;
			last = x + y - 1;

			msb = mbin_msb32(last);

			msb = (2 * msb) - 1;

			for (y = 0; y != (mask + 1); y++) {
				if (((y & msb) >= first) && ((y & msb) <= last))
					ptr[y] ^= slice;
			}
			return;
		}
	}
}

void
mbin_multiply_xform_32(const uint32_t *a, const uint32_t *b, uint32_t *c, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;

	for (x = 0; x != max; x++)
		c[x] = a[x] * b[x];
}

void
mbin_multiply_xform_64(const uint64_t *a, const uint64_t *b, uint64_t *c, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;

	for (x = 0; x != max; x++)
		c[x] = a[x] * b[x];
}

void
mbin_multiply_xform_double(const double *a, const double *b, double *c, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;

	for (x = 0; x != max; x++)
		c[x] = a[x] * b[x];
}

void
mbin_multiply_xform_complex_double(const struct mbin_complex_double *a,
    const struct mbin_complex_double *b,
    struct mbin_complex_double *c, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;

	for (x = 0; x != max; x++)
		c[x] = mbin_mul_complex_double(a[x], b[x]);
}

/*
 * Inverse greater equal transform.
 *
 * f(x,y) = ((x >= y) ? 1 : 0;
 */
void
mbin_inverse_gte_xform_32(uint32_t *ptr, uint8_t lmax)
{
	const uint32_t max = 1U << lmax;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	/* decode a triangle */

	y = ptr[0];
	for (x = 1; x != max; x++) {
		z = ptr[x];
		ptr[x] = ptr[x] - y;
		y = z;
	}
}

void
mbin_inverse_gte_mask_xform_32(uint32_t *ptr, uint8_t log2_max)
{
	while (log2_max--) {
		const uint32_t max = 1U << log2_max;
		uint32_t x;

		for (x = 0; x != max; x++)
			ptr[x + max] -= ptr[x];

		mbin_inverse_gte_xform_32(ptr + max, log2_max);
	}
}

/*
 * Forward greater equal transform.
 *
 * f(x,y) = (x >= y) ? 1 : 0;
 */
void
mbin_forward_gte_xform_32(uint32_t *ptr, uint8_t lmax)
{
	const uint32_t max = 1U << lmax;
	uint32_t x;
	uint32_t y;

	/* encode a triangle */

	y = ptr[0];
	for (x = 1; x != max; x++) {
		y += ptr[x];
		ptr[x] = y;
	}
}

void
mbin_forward_gte_mask_xform_32(uint32_t *ptr, uint8_t log2_max)
{
	uint32_t x;
	uint8_t y;

	for (y = 0; y != log2_max; y++) {
		const uint32_t max = 1U << y;

		mbin_forward_gte_xform_32(ptr + max, y);

		for (x = 0; x != max; x++)
			ptr[x + max] += ptr[x];
	}
}

/*
 * Inverse additive transform.
 *
 * f(x,y) = ((x & y) == y) ? 1 : 0;
 */
void
mbin_inverse_add_xform_32(uint32_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	int32_t a;
	int32_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] = b - a;
			}
		}
	}
}

/*
 * Forward additive transform.
 *
 * f(x,y) = ((x & y) == y) ? 1 : 0;
 */
void
mbin_forward_add_xform_32(uint32_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	int32_t a;
	int32_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] = a + b;
			}
		}
	}
}

void
mbin_inverse_add_xform_double(double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	double a;
	double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] = b - a;
			}
		}
	}
}

void
mbin_forward_add_xform_double(double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	double a;
	double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] = a + b;
			}
		}
	}
}

void
mbin_inverse_add_xform_complex_double(struct mbin_complex_double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	struct mbin_complex_double a;
	struct mbin_complex_double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)].x = b.x - a.x;
				ptr[y + z + (x / 2)].y = b.y - a.y;
			}
		}
	}
}

void
mbin_forward_add_xform_complex_double(struct mbin_complex_double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	struct mbin_complex_double a;
	struct mbin_complex_double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)].x = a.x + b.x;
				ptr[y + z + (x / 2)].y = a.y + b.y;
			}
		}
	}
}

/*
 * Inverse reversed additive transform.
 *
 * f(x,y) = ((x | y) == y) ? 1 : 0;
 */
void
mbin_inverse_rev_add_xform_32(uint32_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	int32_t a;
	int32_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z] = a - b;
			}
		}
	}
}

/*
 * Forward reversed additive transform.
 *
 * f(x,y) = ((x | y) == y) ? 1 : 0;
 */
void
mbin_forward_rev_add_xform_32(uint32_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	int32_t a;
	int32_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z] = a + b;
			}
		}
	}
}

void
mbin_inverse_rev_add_xform_double(double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	double a;
	double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z] = a - b;
			}
		}
	}
}

void
mbin_forward_rev_add_xform_double(double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	double a;
	double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z] = a + b;
			}
		}
	}
}

void
mbin_inverse_rev_add_xform_complex_double(struct mbin_complex_double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	struct mbin_complex_double a;
	struct mbin_complex_double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z].x = a.x - b.x;
				ptr[y + z].y = a.y - b.y;
			}
		}
	}
}

void
mbin_forward_rev_add_xform_complex_double(struct mbin_complex_double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	struct mbin_complex_double a;
	struct mbin_complex_double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z].x = a.x + b.x;
				ptr[y + z].y = a.y + b.y;
			}
		}
	}
}

/*
 * Exclusive-or transform.
 *
 * f(x,y) = ((x & y) == y) ? 1 : 0;
 */
void
mbin_xor_xform_32(uint32_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t a;
	uint32_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] = a ^ b;
			}
		}
	}
}

void
mbin_xor_xform_8(uint8_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint8_t a;
	uint8_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] = a ^ b;
			}
		}
	}
}

/*
 * Sumbits-and transform
 *
 * f(x,y) = (mbin_sumbits32(x & y) & 1) ? -1 : 1;
 */
void
mbin_sumdigits_r2_xform_32(uint32_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	int32_t a;
	int32_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z] = a + b;
				ptr[y + z + (x / 2)] = a - b;
			}
		}
	}
}

/*
 * Sumbits-and transform (radix-2)
 *
 * f(x,y) = (mbin_sumbits32(x & y) & 1) ? -1 : 1;
 */
void
mbin_sumdigits_r2_xform_64(uint64_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	int64_t a;
	int64_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z] = a + b;
				ptr[y + z + (x / 2)] = a - b;
			}
		}
	}
}

/*
 * Sumbits-and transform (radix-2)
 *
 * f(x,y) = (mbin_sumbits32(x & y) & 1) ? -1 : 1;
 */
void
mbin_sumdigits_r2_xform_double(double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	double a;
	double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z] = a + b;
				ptr[y + z + (x / 2)] = a - b;
			}
		}
	}
}

/* Sumbits-and transform (radix-2) */

void
mbin_sumdigits_r2_xform_complex_double(struct mbin_complex_double *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	struct mbin_complex_double a;
	struct mbin_complex_double b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z].x = a.x + b.x;
				ptr[y + z].y = a.y + b.y;
				ptr[y + z + (x / 2)].x = a.x - b.x;
				ptr[y + z + (x / 2)].y = a.y - b.y;
			}
		}
	}
}

/* Sumdigits transform radix 3 */

void
mbin_sumdigits_r3_xform_complex_double(struct mbin_complex_double *ptr, uint8_t log3_max)
{
	static const union {
		double	k;
		uint64_t l;
	}     m = {
		.l = 0x3febb67ae8584caa
	};
	uint32_t max = 1;
	uint32_t xp;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint8_t n;

	for (n = 0; n != log3_max; n++)
		max *= 3;

	for (xp = 1, x = 3; x <= max; xp = x, x *= 3) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != xp; z++) {
				struct mbin_complex_double temp[3];

				temp[0] = ptr[y + z];
				temp[1] = ptr[y + z + (1 * xp)];
				temp[2] = ptr[y + z + (2 * xp)];

				ptr[y + z].x = temp[0].x + temp[1].x + temp[2].x;
				ptr[y + z].y = temp[0].y + temp[1].y + temp[2].y;
				ptr[y + z + (1 * xp)].x =
				    temp[0].x - (temp[1].x / 2.0) - (temp[1].y * m.k) -
				    (temp[2].x / 2.0) + (temp[2].y * m.k);
				ptr[y + z + (1 * xp)].y =
				    temp[0].y - (temp[1].y / 2.0) + (temp[1].x * m.k) -
				    (temp[2].y / 2.0) - (temp[2].x * m.k);
				ptr[y + z + (2 * xp)].x =
				    temp[0].x - (temp[1].x / 2.0) + (temp[1].y * m.k) -
				    (temp[2].x / 2.0) - (temp[2].y * m.k);
				ptr[y + z + (2 * xp)].y =
				    temp[0].y - (temp[1].y / 2.0) - (temp[1].x * m.k) -
				    (temp[2].y / 2.0) + (temp[2].x * m.k);
			}
		}
	}
}

/* Sumdigits transform radix 4 */

void
mbin_sumdigits_r4_xform_complex_double(struct mbin_complex_double *ptr, uint8_t log4_max)
{
	const uint32_t max = 1U << (2 * log4_max);
	uint32_t xp;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	for (xp = 1, x = 4; x <= max; xp = x, x *= 4) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != xp; z++) {
				struct mbin_complex_double temp[4];

				temp[0] = ptr[y + z];
				temp[1] = ptr[y + z + (1 * xp)];
				temp[2] = ptr[y + z + (2 * xp)];
				temp[3] = ptr[y + z + (3 * xp)];

				ptr[y + z].x =
				    temp[0].x + temp[1].x + temp[2].x + temp[3].x;
				ptr[y + z].y =
				    temp[0].y + temp[1].y + temp[2].y + temp[3].y;
				ptr[y + z + (1 * xp)].x =
				    temp[0].x - temp[1].y - temp[2].x + temp[3].y;
				ptr[y + z + (1 * xp)].y =
				    temp[0].y + temp[1].x - temp[2].y - temp[3].x;
				ptr[y + z + (2 * xp)].x =
				    temp[0].x - temp[1].x + temp[2].x - temp[3].x;
				ptr[y + z + (2 * xp)].y =
				    temp[0].y - temp[1].y + temp[2].y - temp[3].y;
				ptr[y + z + (3 * xp)].x =
				    temp[0].x + temp[1].y - temp[2].x - temp[3].y;
				ptr[y + z + (3 * xp)].y =
				    temp[0].y - temp[1].x - temp[2].y + temp[3].x;
			}
		}
	}
}

/* Sumdigits input reorder by index and radix */

uint32_t
mbin_sumdigits_reorder_32(uint32_t x, uint32_t radix, uint32_t max)
{
	uint32_t r = 0;
	uint32_t k = 1;

	if (radix == 2)
		return (x % max);
	if (radix == 4)
		return ((x ^ (2 * (x & 0x55555555))) % max);

	while (x) {
		r += k * ((radix - (x % radix)) % radix);
		x /= radix;
		k *= radix;
	}
	return (r % max);

}

/* Sumdigits predict maximum by added offset and radix */

uint32_t
mbin_sumdigits_predmax_32(uint32_t x, uint32_t radix, uint32_t max)
{
	uint32_t k1 = 0;
	uint32_t k2 = 1;
	uint32_t r = 0;

	while (k1 < max) {

		r += (((x + max - k1) / k2) % radix) * k2;

		k1 = (k1 * radix) + 1;
		k2 = (k2 * radix);
	}
	return (r % max);
}

/*
 * Inverse additive transform.
 *
 * f(x,y) = ((x & y) == y) ? 1 : 0;
 */
void
mbin_xor3_mod_inverse_add_xform_64(uint64_t *ptr, uint64_t neg,
    uint64_t mod, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint64_t a;
	uint64_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				a = mbin_xor3_exp_mod_any_64(a, neg, mod);
				ptr[y + z + (x / 2)] =
				    mbin_xor3_mul_mod_any_64(a, b, mod);
			}
		}
	}
}

/*
 * Forward additive transform.
 *
 * f(x,y) = ((x & y) == y) ? 1 : 0;
 */
void
mbin_xor3_mod_forward_add_xform_64(uint64_t *ptr, uint64_t neg,
    uint64_t mod, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint64_t a;
	uint64_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] =
				    mbin_xor3_mul_mod_any_64(a, b, mod);
			}
		}
	}
}

/*
 * Inverse additive transform.
 *
 * f(x,y) = ((x & y) == y) ? 1 : 0;
 */
void
mbin_xor3_inverse_add_xform_64(uint64_t *ptr, uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint64_t neg;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint64_t a;
	uint64_t b;

	neg = mbin_power_64(3, 32) - 1ULL;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				a = mbin_xor3_exp_64(a, neg);
				ptr[y + z + (x / 2)] =
				    mbin_xor3_mul_64(a, b);
			}
		}
	}
}

/*
 * Forward additive transform.
 *
 * f(x,y) = ((x & y) == y) ? 1 : 0;
 */
void
mbin_xor3_forward_add_xform_64(uint64_t *ptr,
    uint8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint64_t a;
	uint64_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] =
				    mbin_xor3_mul_64(a, b);
			}
		}
	}
}

/*
 * Forward additive transform.
 *
 * f(x,y) = ((x & y) == y) ? 1 : 0;
 */
void
mbin_xor3_xform_64(uint64_t *ptr, int8_t log2_max)
{
	const uint32_t max = 1U << log2_max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint64_t a;
	uint64_t b;

	for (x = 2; x <= max; x *= 2) {
		for (y = 0; y != max; y += x) {
			for (z = 0; z != (x / 2); z++) {
				a = ptr[y + z];
				b = ptr[y + z + (x / 2)];
				ptr[y + z + (x / 2)] =
				    mbin_xor3_64(
				    mbin_xor3_64(a, a), b);
			}
		}
	}
}
