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

/*
 * baseH is an exponent function
 */

/*
 * Rewritten step function:
 *
 * a2 = a1 ^ (a1 << s) ^
 *     (2 * (((~a1) & (a0 << s)) | (a0 & (~a1 ^ (a0 << s)))));
 */

#include <stdint.h>

#include "math_bin.h"

uint32_t
mbin_baseH_gen_div(uint8_t shift)
{
	;				/* indent fix */
#if 0
	uint8_t n = 32;
	uint32_t x = 1;

	while (n--) {
		x = (x * x) + 4;
	}
	return (x);
#else
	switch (shift) {
	case 2:
		return (0x3ef7226d);
		break;
	case 3:
		return (0xd1d69179);
		break;
	case 4:
		return (0x14640cf1);
		break;
	case 5:
		return (0x1d32efe1);
		break;
	case 6:
		return (0xbe5fafc1);
		break;
	case 7:
		return (0x58de7f81);
		break;
	case 8:
		return (0xb2f8ff01);
		break;
	case 9:
		return (0xdfdffe01);
		break;
	case 10:
		return (0x7f6ffc01);
		break;
	case 11:
		return (0xfd7ff801);
		break;
	case 12:
		return (0xf4fff001);
		break;
	case 13:
		return (0xcfffe001);
		break;
	case 14:
		return (0x2fffc001);
		break;
	case 15:
		return (0x7fff8001);
		break;
	case 16:
		return (0xffff0001);
		break;
	case 17:
		return (0xfffe0001);
		break;
	case 18:
		return (0xfffc0001);
		break;
	case 19:
		return (0xfff80001);
		break;
	case 20:
		return (0xfff00001);
		break;
	case 21:
		return (0xffe00001);
		break;
	case 22:
		return (0xffc00001);
		break;
	case 23:
		return (0xff800001);
		break;
	case 24:
		return (0xff000001);
		break;
	case 25:
		return (0xfe000001);
		break;
	case 26:
		return (0xfc000001);
		break;
	case 27:
		return (0xf8000001);
		break;
	case 28:
		return (0xf0000001);
		break;
	case 29:
		return (0xe0000001);
		break;
	case 30:
		return (0xc0000001);
		break;
	case 31:
		return (0x80000001);
		break;
	default:
		return (0);
	}
#endif
}

uint32_t
mbin_base_2toH_32(uint32_t index, uint8_t shift)
{
	uint32_t f;
	uint32_t g;
	uint32_t t;
	uint8_t n;

#if 0
	/* XXX shift == 2 */
	/* XXX constant origin needs to be investigated */
	/* XXX probably this constant should be removed */
	index -= 0x30c41244;
#endif

	f = mbin_baseH_gen_div(shift);

	g = mbin_power_32(f, -index);

	t = 1;
	for (n = 2; n != 32; n++) {
		t |= g & (1 << n);
		g = g * f;
	}
	return (t);
}

uint32_t
mbin_base_Hto2_32(uint32_t bh, uint8_t shift)
{
	uint32_t b2 = 0;
	uint32_t m = 4;

	while (m) {
		if ((mbin_base_2toH_32(b2, shift) ^ bh) & m) {
			b2 ^= m / 4;
		}
		m *= 2;
	}
	return (b2);
}

/*
 * This function restores the state at the given index.
 */
void
mbin_baseH_get_state32(struct mbin_baseH_state32 *ps,
    uint32_t index, uint8_t shift)
{
	uint32_t ap;
	uint32_t an;

	ap = mbin_base_2toH_32(index, shift);
	an = mbin_base_2toH_32(index + 1, shift);

	ps->a = ap;
	ps->c = an ^ ap ^ (ap << shift);
	ps->s = shift;
}

/*
 * This function increments the state by one.
 */
void
mbin_baseH_inc_state32(struct mbin_baseH_state32 *ps)
{
	uint32_t a;
	uint32_t c;
	uint8_t s;

	a = ps->a;
	c = ps->c;
	s = ps->s;

	/* standard addition formula */

	ps->a = a ^ (a << s) ^ c;
	ps->c = 2 * (((a ^ c) & (a << s)) | (a & c));
}

/*
 * This function deciphers the state like an exponent,
 */
uint32_t
mbin_baseH_decipher_state32(struct mbin_baseH_state32 *ps)
{
	struct mbin_baseH_state32 psc = *ps;
	uint32_t t;
	uint8_t n;

	t = 0;
	for (n = 0; n != 32; n++) {
		t |= psc.a & (1 << n);
		mbin_baseH_inc_state32(&psc);
	}
	return (t);
}

static const uint32_t
	mbin_baseH_VX_data[32][2] =
{
	[2] = {0x2acdfb69, 0xf0423765,},
	[3] = {0xe60d1bd9, 0x36dc08c9,},
	[4] = {0x1429dc41, 0xa91d6411,},
	[5] = {0x03b434a1, 0x95e19421,},
	[6] = {0x0ec42181, 0x3acc6041,},
	[7] = {0x5656c381, 0x38c1c081,},
	[8] = {0xe8780801, 0x5c080101,},
	[9] = {0x6a641201, 0xa8240201,},
	[10] = {0x0be02801, 0x40a00401,},
	[11] = {0x39c05801, 0x02c00801,},
	[12] = {0x1400c001, 0x0c001001,},
	[13] = {0x1401a001, 0x34002001,},
	[14] = {0xa0038001, 0xe0004001,},
	[15] = {0xc0078001, 0xc0008001,},
	[16] = {0x00100001, 0x00010001,},
	[17] = {0x00220001, 0x00020001,},
	[18] = {0x00480001, 0x00040001,},
	[19] = {0x00980001, 0x00080001,},
	[20] = {0x01400001, 0x00100001,},
	[21] = {0x02a00001, 0x00200001,},
	[22] = {0x05800001, 0x00400001,},
	[23] = {0x0b800001, 0x00800001,},
	[24] = {0x18000001, 0x01000001,},
	[25] = {0x32000001, 0x02000001,},
	[26] = {0x68000001, 0x04000001,},
	[27] = {0xd8000001, 0x08000001,},
	[28] = {0xc0000001, 0x10000001,},
	[29] = {0xa0000001, 0x20000001,},
	[30] = {0x80000001, 0x40000001,},
	[31] = {0x80000001, 0x80000001,},
};

uint32_t
mbin_baseH_2toVX_32(uint32_t value, uint8_t shift)
{
	uint32_t t;
	uint32_t k;
	uint8_t n;

	if (shift < 2 || shift > 31)
		return (0);

	value = -value;

	k = mbin_baseH_VX_data[shift][0];

	t = 0;
	for (n = 0; n != 32; n++) {
		if (value & (1 << n)) {
			t |= (1 << n);
			value += k << n;
		}
		k *= mbin_baseH_VX_data[shift][1];
	}
	return (t);
}

uint32_t
mbin_baseH_VXto2_32(uint32_t value, uint8_t shift)
{
	uint32_t t;
	uint32_t k;
	uint8_t n;

	if (shift < 2 || shift > 31)
		return (0);

	k = mbin_baseH_VX_data[shift][0];

	t = 0;
	for (n = 0; n != 32; n++) {
		if (value & (1 << n))
			t -= k << n;
		k *= mbin_baseH_VX_data[shift][1];
	}

	t = -t;

	return (t);
}
