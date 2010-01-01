/*-
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
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
 * The following file contains functions that are used to transform a
 * truth table into a list of XOR and AND statements.
 */

#include <stdint.h>

#include "math_bin.h"

void
mbin_expand_gte_32x32(uint32_t *ptr, uint32_t set_bits,
    uint32_t mask, uint32_t val)
{
	uint32_t x;
	uint32_t xmask;

	xmask = mbin_msb32(set_bits);
	xmask = xmask | (xmask - 1);

	x = set_bits | (~mask);

	while (1) {
		ptr[x & mask] -= val;

		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		if ((x & xmask) < set_bits) {
			/* jump to next area */
			x &= ~xmask;
			x |= set_bits;
		}
	}
}

void
mbin_expand_add_32x32(uint32_t *ptr, uint32_t set_bits,
    uint32_t mask, uint32_t val)
{
	uint32_t x;

	set_bits |= (~mask);
	x = set_bits;

	while (1) {
		ptr[x & mask] -= val;

		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		x |= set_bits;
	}
}

void
mbin_expand_add_mod_32x32(uint32_t *ptr, uint32_t set_bits,
    uint32_t mask, uint32_t val, uint32_t mod)
{
	uint32_t x;

	set_bits |= (~mask);
	x = set_bits;

	while (1) {
		ptr[x & mask] = (mod + ptr[x & mask] - val) % mod;

		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		x |= set_bits;
	}
}

void
mbin_expand_xor_32x32(uint32_t *ptr, uint32_t set_bits,
    uint32_t mask, uint32_t slice)
{
	uint32_t x;

	set_bits |= (~mask);
	x = set_bits;

	while (1) {
		ptr[x & mask] ^= slice;
		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		x |= set_bits;
	}
}

void
mbin_expand_xor_16x32(uint16_t *ptr, uint32_t set_bits,
    uint32_t mask, uint16_t slice)
{
	uint32_t x;

	set_bits |= (~mask);
	x = set_bits;

	while (1) {
		ptr[x & mask] ^= slice;
		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		x |= set_bits;
	}
}

void
mbin_expand_xor_8x32(uint8_t *ptr, uint32_t set_bits,
    uint32_t mask, uint8_t slice)
{
	uint32_t x;

	set_bits |= (~mask);
	x = set_bits;

	while (1) {
		ptr[x & mask] ^= slice;
		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		x |= set_bits;
	}
}
