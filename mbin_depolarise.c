/*-
 * Copyright (c) 2008 Hans Petter Selasky
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
 * Functions for converting polar numbers into a non-polar number:
 *
 * -+-+-+-+ or +-+-+-+- -> +++++
 */

uint32_t
mbin_depolarise32(uint32_t val, uint32_t neg_pol)
{
#if 1
	return ((val ^ neg_pol) - neg_pol);
#else
	return ((val & ~neg_pol) - (val & neg_pol));
#endif
}

/*
 * Properties:
 *
 * 1)
 * x = 2;
 * a = mbin_depolar_div32(t, x);
 * t == mbin_polarise32(a,a);
 * t == (a ^ (x*a))
 * a == mbin_depolarise(t,a);
 *
 * 2)
 * a == mbin_depolarise32(-a,-a);
 * a == mbin_depolarise32(a,~a);
 *
 * 3)
 * (t == 2**n) =>
 *   (mbin_depolar_div32(t, 2*x) =
 *     (mbin_div_odd32(t, -(2*x*x) + x) - mbin_div_odd32(t, x)) / 2)
 *
 * Also see: Greycode
 *
 * NOTE: Polar division only supports even divisors.
 */
uint32_t
mbin_depolar_div32(uint32_t rem, uint32_t div)
{
	uint32_t m = 1;
	uint32_t s = 0;

	while (m) {
		if ((rem ^ s) & m) {
			s += div;
		}
		m *= 2;
		div *= 2;
	}
	return (rem ^ s);
}

uint16_t
mbin_depolarise16(uint16_t val, uint16_t neg_pol)
{
#if 1
	return ((val ^ neg_pol) - neg_pol);
#else
	return ((val & ~neg_pol) - (val & neg_pol));
#endif
}

uint16_t
mbin_depolar_div16(uint16_t rem, uint16_t div)
{
	uint16_t m = 1;
	uint16_t s = 0;

	while (m) {
		if ((rem ^ s) & m) {
			s += div;
		}
		m *= 2;
		div *= 2;
	}
	return (rem ^ s);
}

uint8_t
mbin_depolarise8(uint8_t val, uint8_t neg_pol)
{
#if 1
	return ((val ^ neg_pol) - neg_pol);
#else
	return ((val & ~neg_pol) - (val & neg_pol));
#endif
}

uint8_t
mbin_depolar_div8(uint8_t rem, uint8_t div)
{
	uint8_t m = 1;
	uint8_t s = 0;

	while (m) {
		if ((rem ^ s) & m) {
			s += div;
		}
		m *= 2;
		div *= 2;
	}
	return (rem ^ s);
}
