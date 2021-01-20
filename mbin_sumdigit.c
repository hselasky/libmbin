/*-
 * Copyright (c) 2011-2021 Hans Petter Selasky. All rights reserved.
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
mbin_sumdigit_32(uint32_t x, uint32_t y, uint32_t mod)
{
	uint32_t xt;
	uint32_t yt;
	uint32_t sum = 0;

	while (x != 0 && y != 0) {
		xt = x % mod;
		x /= mod;
		yt = y % mod;
		y /= mod;

		sum += xt * yt;
		sum %= mod;
	}
	return (sum);
}

uint32_t
mbin_sumdigit_r2_32(uint32_t x, uint32_t y)
{
	return mbin_sumdigit_32(x, y, 2);
}

uint32_t
mbin_sumdigit_r3_32(uint32_t x, uint32_t y)
{
	return mbin_sumdigit_32(x, y, 3);
}

uint32_t
mbin_sumdigit_r4_32(uint32_t x, uint32_t y)
{
	return mbin_sumdigit_32(x, y, 4);
}
