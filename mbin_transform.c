/*-
 * Copyright (c) 2008-2010 Hans Petter Selasky. All rights reserved.
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

/* Sum Of Sums, SOS - transform */

void
mbin_transform_sos_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t *scratch,
    uint32_t mask)
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
				temp[y] -= sum * val;
				scratch[y - x] = sum;
				if (y == mask)
					break;
			}
			temp[x] = val;
		} else {
			for (y = x;; y++) {
				sum += scratch[y - x];
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
