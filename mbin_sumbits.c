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

#include "math_bin.h"

uint8_t
mbin_sumbits64(uint64_t val)
{
	val = ((val & (0x1ULL * 0xAAAAAAAAAAAAAAAAULL)) >> 1) + (val & (0x1ULL * 0x5555555555555555ULL));
	val = ((val & (0x3ULL * 0x4444444444444444ULL)) >> 2) + (val & (0x3ULL * 0x1111111111111111ULL));
	val = ((val & (0xFULL * 0x1010101010101010ULL)) >> 4) + (val & (0xFULL * 0x0101010101010101ULL));
	val += val >> 8;
	val += val >> 16;
	val += val >> 32;
	return (val & 127);
}

uint8_t
mbin_sumbits32(uint32_t val)
{
#if 0
	uint8_t t = 0;

	while (val) {
		if (val & 1)
			t++;
		val >>= 1;
	}
	return (t);
#else
	val = ((val & (1U * 0xAAAAAAAAU)) / 2U) + (val & (1U * 0x55555555U));
	val = ((val & (3U * 0x44444444U)) / 4U) + (val & (3U * 0x11111111U));
	val = ((val & (15U * 0x10101010U)) / 16U) + (val & (15U * 0x01010101U));
	val += val >> 8;
	val += val >> 16;
	return (val & 63);
#endif
}

uint8_t
mbin_sumbits16(uint16_t val)
{
#if 0
	uint8_t t = 0;

	while (val) {
		if (val & 1)
			t++;
		val >>= 1;
	}
	return (t);
#else
	val = ((val & (1U * 0xAAAAAAAAU)) / 2U) + (val & (1U * 0x55555555U));
	val = ((val & (3U * 0x44444444U)) / 4U) + (val & (3U * 0x11111111U));
	val = ((val & (15U * 0x10101010U)) / 16U) + (val & (15U * 0x01010101U));
	val = ((val & (255U * 0x01000100U)) / 256U) + (val & (255U * 0x00010001U));
	return (val);
#endif
}

uint8_t
mbin_sumbits8(uint8_t val)
{
#if 0
	uint8_t t = 0;

	while (val) {
		if (val & 1)
			t++;
		val >>= 1;
	}
	return (t);
#else
	val = ((val & (1U * 0xAAAAAAAAU)) / 2U) + (val & (1U * 0x55555555U));
	val = ((val & (3U * 0x44444444U)) / 4U) + (val & (3U * 0x11111111U));
	val = ((val & (15U * 0x10101010U)) / 16U) + (val & (15U * 0x01010101U));
	return (val);
#endif
}

/* Find leading digit in 2-base: */

uint8_t
mbin_fld_64(uint64_t y)
{
	if (y == 0)
		return (0);
	return (mbin_sumbits64((~y) & (y - 1)));
}

uint8_t
mbin_fld_32(uint32_t y)
{
	if (y == 0)
		return (0);
	return (mbin_sumbits32((~y) & (y - 1)));
}

uint8_t
mbin_fld_16(uint16_t y)
{
	if (y == 0)
		return (0);
	return (mbin_sumbits16((~y) & (y - 1)));
}

uint8_t
mbin_fld_8(uint8_t y)
{
	if (y == 0)
		return (0);
	return (mbin_sumbits8((~y) & (y - 1)));
}
