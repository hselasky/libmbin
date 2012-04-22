/*-
 * Copyright (c) 2010-2012 Hans Petter Selasky. All rights reserved.
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
 * This file implements a fast transform that can be used for fast
 * integer convolution.
 */

#include <stdio.h>

#include <stdint.h>

#include <stdlib.h>

#include <string.h>

#include "math_bin.h"

/* parameters */

#define	FET32_PRIME		0xFFFFFFFFU
#define	FET32_RING_SIZE		0x00010000U
#define	FET32_SHIFT		16

uint32_t
mbin_fet_32_generate_power(uint32_t y)
{
	return (mbin_power_mod_32(7, y, FET32_PRIME));
}

static void
mbin_fet_generate_bitrev(uint8_t power, const char *br_type)
{
	uint32_t i;
	uint32_t j;
	uint32_t k;
	uint32_t max = (1U << power);
	uint32_t lim;

	lim = 0;

	for (i = 0; i != max; i++) {

		j = mbin_bitrev32(i << (32 - power));

		if (j < i) {
			lim++;
		}
	}

	printf("\t" "static const %s bit_rev[0x%x] = {\n",
	    (power <= 8) ? "uint8_t" :
	    (power <= 16) ? "uint16_t" :
	    "uint32_t", (2 * lim) + 8);

	printf("\t\t");

	k = 0;

	for (i = 0; i != max; i++) {

		j = mbin_bitrev32(i << (32 - power));

		if (j < i) {
			printf("0x%03x, 0x%03x, ", i, j);
			k++;
			if (((k & 3) == 0) && (k != lim))
				printf("\n\t\t");
		}
	}

	printf("\n\t};\n");
	return;
}

static int32_t
mbin_fet_32_fix_factor(uint32_t factor)
{
	/* correctly handle negative values */
	if (factor >= (FET32_PRIME / 2))
		factor -= FET32_PRIME;

	return (factor);
}

void
mbin_fet_32_generate(uint8_t power)
{
	const char *br_type;
	uint32_t freq;
	uint32_t max;
	uint32_t y;
	uint32_t z;
	uint32_t n;

	if (power > FET32_SHIFT || power <= 0) {
		printf("#error \"Invalid power\"\n");
		return;
	}
	br_type = (power <= 8) ? "uint8_t" :
	    (power <= 16) ? "uint16_t" :
	    "uint32_t";

	printf("void\n"
	    "fet_forward_%d_32(int64_t *data)\n", 1 << power);

	printf("{\n");

	mbin_fet_generate_bitrev(power, br_type);

	max = 1 << power;

	printf("\t" "static const int32_t ktable[0x%x] = {\n", max / 2);

	freq = (FET32_RING_SIZE >> power);
	for (y = 0; y != (max / 2); y++) {
		uint32_t factor;

		z = mbin_bitrev32(y << (32 - power));
		factor = mbin_fet_32_generate_power(freq * z);
		printf("\t\t%d,\n", mbin_fet_32_fix_factor(factor));
	}

	printf("\t" "};\n");

	printf("\n");

	printf("\t" "const int32_t *ptab;\n");
	printf("\t" "const %s *p = bit_rev;\n", br_type);
	printf("\t" "int64_t ta;\n");
	printf("\t" "int64_t tb;\n");
	printf("\t" "int64_t tc;\n");
	printf("\t" "int32_t td;\n");
	printf("\t" "%s x;\n", br_type);
	printf("\t" "%s y;\n", br_type);

	printf("\n");

	for (n = 0; n != power; n++) {

		printf("\t" "/* round %d */\n", n);
		printf("\t" "ptab = ktable;\n");

		printf("\t" "for (y = 0; y != 0x%x; y += 0x%x) {\n", max, 1 << (power - n));

		if (n != 0) {
			printf("\t\t" "td = ptab[0];\n");
			printf("\t\t" "ptab++;\n");
		}
		printf("\t\t" "for (x = 0; x != 0x%x; x++) {\n", (max >> (n + 1)));

		if (n == 0) {
			printf("\t\t\t" "ta = data[x];\n");
			printf("\t\t\t" "tb = data[x+0x%x];\n", (max >> (n + 1)));
		} else {
			printf("\t\t\t" "ta = data[x];\n");
			printf("\t\t\t" "tb = data[x+0x%x] * (int64_t)(td);\n", (max >> (n + 1)));
		}

		printf("\t\t\t" "tc = ta + tb;\n");
		printf("\t\t\t" "tc = (tc >> 32) + ((uint32_t)tc);\n");
		printf("\t\t\t" "tc = (tc >> 32) + ((uint32_t)tc);\n");
		printf("\t\t\t" "data[x] = tc;\n");
		printf("\t\t\t" "data[x+0x%x] = tc;\n", (max >> (n + 1)));

		printf("\t\t" "}\n");
		printf("\t\t" "data += 0x%x;\n", 1 << (power - n));
		printf("\t" "}\n");
		printf("\t" "data -= 0x%x;\n", max);
	}

	printf("\t/* In-place index bit-reversal */\n");

	printf(
	    "\t" "while (*p) {\n"
	    "\t\t" "int64_t t;\n"
	    "\t\t" "%s a,b;\n\n"
	    "\t\t" "a = *(p + 0);\n"
	    "\t\t" "b = *(p + 1);\n"
	    "\t\t" "t = data[b];\n"
	    "\t\t" "data[b] = data[a];\n"
	    "\t\t" "data[a] = t;\n"
	    "\n"
	    "\t\t" "a = *(p + 2);\n"
	    "\t\t" "b = *(p + 3);\n"
	    "\t\t" "p += 4;\n"
	    "\t\t" "t = data[b];\n"
	    "\t\t" "data[b] = data[a];\n"
	    "\t\t" "data[a] = t;\n"
	    "\t" "}\n", br_type);

	printf("}\n");

	printf("\n"
	    "void\n"
	    "fet_conv_%d_32(const int64_t *a, const int64_t *b, int64_t *c)\n", max);

	printf("{\n"
	    "\t" "int64_t ta;\n"
	    "\t" "int64_t tb;\n"
	    "\t" "uint32_t x;\n"
	    "\n"
	    "\t" "for (x = 0; x != 0x%x; x++) {\n"
	    "\t\t" "ta = a[x];\n"
	    "\t\t" "if (ta < 0)\n"
	    "\t\t\t" "ta += 0x%08xLL;\n"
	    "\t\t" "if (ta < 0)\n"
	    "\t\t\t" "ta += 0x%08xLL;\n"
	    "\t\t" "tb = b[x];\n"
	    "\t\t" "if (tb < 0)\n"
	    "\t\t\t" "tb += 0x%08xLL;\n"
	    "\t\t" "if (tb < 0)\n"
	    "\t\t\t" "tb += 0x%08xLL;\n"
	    "\t\t" "ta = ((uint64_t)(uint32_t)ta) * ((uint64_t)(uint32_t)tb);\n"
	    "\t\t" "ta = (((uint64_t)ta) >> 32) + ((uint32_t)ta);\n"
	    "\t\t" "ta = (((uint64_t)ta) >> 32) + ((uint32_t)ta);\n"
	    "\t\t" "c[x] = ta;\n"
	    "\t" "}\n",
	    max,
	    FET32_PRIME,
	    FET32_PRIME,
	    FET32_PRIME,
	    FET32_PRIME);

	printf("\n"
	    "\t" "for (x = 1; x != 0x%x; x++) {\n"
	    "\t\t" "/* swap */\n"
	    "\t\t" "ta = c[0x%x - x];\n"
	    "\t\t" "tb = c[x];\n"
	    "\t\t" "c[x] = ta;\n"
	    "\t\t" "c[0x%x - x] = tb;\n"
	    "\t" "}\n", max / 2, max, max);

	printf("}\n");

	printf("\n"
	    "int64_t\n"
	    "fet_to_lin(int64_t x)\n"
	    "{\n"
	    "\t" "if (x < 0)\n"
	    "\t\t" "x += 0x%08xLL;\n"
	    "\t" "if (x < 0)\n"
	    "\t\t" "x += 0x%08xLL;\n"
	    "\t" "if (x >= (0x%08xLL/2))\n"
	    "\t\t" "x -= 0x%08xLL;\n"
	    "\n"
	    "\t" "return (x);\n"
	    "}\n",
	    FET32_PRIME,
	    FET32_PRIME,
	    FET32_PRIME,
	    FET32_PRIME);
}

void
mbin_fet_32_find_mod(struct mbin_fet_32_mod *pmod)
{
	/* search for a suitable prime */

	uint32_t max;
	uint32_t x;
	uint32_t y;

	pmod->mod |= 1;
	pmod->mod += 2;

	if (pmod->mod < 11)
		pmod->mod = 11;

	if (pmod->base == 0)
		pmod->base = 3;

	for (; pmod->mod != 0xFFFFFFFFU; pmod->mod += 2) {

		if (!(pmod->mod % 3U))
			continue;

		if (!(pmod->mod % 5U))
			continue;

		if (!(pmod->mod % 7U))
			continue;

		max = (mbin_sqrt_64(pmod->mod) | 1) + 2;

		for (x = 3; x != max; x += 2) {
			if (!(pmod->mod % x))
				break;
		}

		if (x != max)
			continue;

		/* verify the prime */

		x = 1;
		y = 0;

		while (1) {

			x = (((uint64_t)x) * ((uint64_t)pmod->base)) % pmod->mod;
			y++;
			if (x == 1)
				break;

			if (y == pmod->mod)
				goto skip;
		}

		pmod->length = y;
		pmod->conv = mbin_power_mod_32(y, pmod->mod - 2, pmod->mod);

		return;
skip:		;
	}

	pmod->length = 0;
}
