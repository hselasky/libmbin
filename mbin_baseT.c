/*-
 * Copyright (c) 2008-2009 Hans Petter Selasky. All rights reserved.
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
 * Number base conversion from binary to the T-base.
 *
 * You get 1.999 base when "tm = 0" and "tp = -1".
 */

uint32_t
mbin_base_2toT_32(uint32_t tm, uint32_t tp, uint32_t r)
{
	uint8_t x;

	x = 32;
	while (x--) {
		r = r - (2 * (r & tm)) + (2 * (r & tp));
		tm *= 2;
		tp *= 2;
	}
	return (r);
}

uint32_t
mbin_base_Tto2_32(uint32_t tm, uint32_t tp, uint32_t r)
{
	uint8_t x;
	uint8_t y;
	uint32_t um;
	uint32_t up;

	x = 32;
	while (x--) {
		um = (tm << x);
		up = (tp << x);
		for (y = x; y != 32; y++) {
			if (r & um & (1 << y)) {
				r += (2 << y);
			}
			if (r & up & (1 << y)) {
				r -= (2 << y);
			}
		}
	}
	return (r);
}

uint32_t
mbin_base_T_add_32(uint32_t tm, uint32_t tp, uint32_t a, uint32_t b)
{
	a = mbin_base_Tto2_32(tm, tp, a);
	b = mbin_base_Tto2_32(tm, tp, b);
	return (mbin_base_2toT_32(tm, tp, a + b));
}

uint32_t
mbin_base_T_sub_32(uint32_t tm, uint32_t tp, uint32_t a, uint32_t b)
{
	a = mbin_base_Tto2_32(tm, tp, a);
	b = mbin_base_Tto2_32(tm, tp, b);
	return (mbin_base_2toT_32(tm, tp, a - b));
}

uint32_t
mbin_base_T_div_odd_32(uint32_t tm, uint32_t tp, uint32_t rem, uint32_t div)
{
	uint32_t m;
	uint32_t s;

	s = 0;
	m = 1;
	while (m) {
		if (rem & m) {
			rem = mbin_base_T_sub_32(tm, tp, rem, div);
			s |= m;
		}
		/* "div" = "2*div" in 2-base */
		div = mbin_base_T_add_32(tm, tp, div, div);
		m = 2 * m;
	}
	return (s);
}

#if 0

/* Special case code */

uint32_t
mbin_add_1999_32(uint32_t a, uint32_t b)
{
	uint32_t r;
	uint32_t m;

	r = 0;
	m = 1;
	while (m) {
		r = r + (2 * (r & (0 - m))) + (a & m) + (b & m);
		m *= 2;
	}
	return (r);
}

uint32_t
mbin_sub_1999_32(uint32_t a, uint32_t b)
{
	uint32_t r;
	uint32_t m;

	r = 0;
	m = 1;
	while (m) {
		r = r + (2 * (r & (0 - m))) + (a & m) - (b & m);
		m *= 2;
	}
	return (r);
}

#endif
