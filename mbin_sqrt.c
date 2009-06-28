/*-
 * Copyright (c) 2001-2009 Hans Petter Selasky. All rights reserved.
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

/*
 * This square root function should be faster than Newtons method:
 *
 * xn = (x + N/x) / 2
 */

uint32_t
mbin_sqrt_64(uint64_t a)
{
	uint64_t b = 0x4000000000000000ULL;

	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00ULL;
	} else {
		b >>= 1;
		b ^= 0xc00ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700ULL;
	} else {
		b >>= 1;
		b ^= 0x300ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0ULL;
	} else {
		b >>= 1;
		b ^= 0xc0ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70ULL;
	} else {
		b >>= 1;
		b ^= 0x30ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1cULL;
	} else {
		b >>= 1;
		b ^= 0xcULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7ULL;
	} else {
		b >>= 1;
		b ^= 0x3ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1ULL;
	} else {
		b >>= 1;
	}
	return (b);
}
