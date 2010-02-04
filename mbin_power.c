/*-
 * Copyright (c) 2009-2010 Hans Petter Selasky. All rights reserved.
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

uint32_t
mbin_power_32(uint32_t x, uint32_t y)
{
	uint32_t r = 1;

	while (y) {
		if (y & 1)
			r *= x;
		x *= x;
		y /= 2;
	}
	return (r);
}

uint64_t
mbin_power_64(uint64_t x, uint64_t y)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r *= x;
		x *= x;
		y /= 2;
	}
	return (r);
}

uint32_t
mbin_power_mod_32(uint32_t x, uint32_t y, uint32_t mod)
{
	uint64_t r = 1;
	uint64_t t = x % mod;

	while (y) {
		if (y & 1) {
			r *= t;
			r %= mod;
		}
		t *= t;
		t %= mod;
		y /= 2;
	}
	return (r);
}

static const uint32_t mbin_log5_table[32] = {
	0x00000000,
	0x00000000,
	0x00000004,
	0xca253518,
	0x106b8670,
	0xb7240de0,
	0xbe959fc0,
	0x7e7d4f80,
	0xb822df00,
	0x5be6be00,
	0x5e517c00,
	0x16b2f800,
	0x95a5f000,
	0xcc4be000,
	0x1c97c000,
	0x492f8000,
	0xd25f0000,
	0xa4be0000,
	0x497c0000,
	0x92f80000,
	0x25f00000,
	0x4be00000,
	0x97c00000,
	0x2f800000,
	0x5f000000,
	0xbe000000,
	0x7c000000,
	0xf8000000,
	0xf0000000,
	0xe0000000,
	0xc0000000,
	0x80000000,
};

uint32_t
mbin_log_5(uint32_t r, uint32_t x)
{
	uint8_t n;

	if (x & 2) {
		/* value is considered negative */
		x = -x;
	}
	for (n = 2; n != 32; n++) {
		if (x & (1 << n)) {
			x = x + (x << n);
			r -= mbin_log5_table[n];
		}
	}
	return (r);
}

uint32_t
mbin_exp_5(uint32_t r, uint32_t x)
{
	uint8_t n;

	for (n = 2; n != 32; n++) {
		if (x & (1 << n)) {
			r = r + (r << n);
			x -= mbin_log5_table[n];
		}
	}
	return (r);
}

uint32_t
mbin_power_odd_32(uint32_t rem, uint32_t base, uint32_t exp)
{
	if (base & 2) {
		/* divider is considered negative */
		base = -base;
		/* check if result should be negative */
		if (exp & 1)
			rem = -rem;
	}
	return (mbin_exp_5(rem, mbin_log_5(0, base) * exp));
}
