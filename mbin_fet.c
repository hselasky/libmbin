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

	for (i = 0; i != max; i += 2) {

		j = mbin_bitrev32(i << (32 - power));

		if (j < i)
			lim++;
	}

	printf("\t" "static const %s bit_rev[0x%x] = {\n",
	    br_type, (2 * lim) + 8);

	printf("\t\t");

	k = 0;

	for (i = 0; i != max; i += 2) {

		j = mbin_bitrev32(i << (32 - power));

		if (j < i) {
			printf("0x%03x, 0x%03x, ", i, j);
			k++;
			if (((k & 3) == 0) && (k != lim))
				printf("\n\t\t");
		}
	}

	printf("\n\t};\n");
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
	uint8_t forward;

	if (power > FET32_SHIFT || power < 2) {
		printf("#error \"Invalid power\"\n");
		return;
	}
	br_type = (power <= 7) ? "uint8_t" :
	    (power <= 15) ? "uint16_t" :
	    "uint32_t";

	max = 1 << power;
	forward = 0;

	printf("static const uint32_t "
	    "fet_%d_32_table[0x%x] = {\n",
	    max, max / 2);

	freq = (FET32_RING_SIZE >> power);
	for (y = 0; y != (max / 2); y++) {
		uint32_t factor;

		z = mbin_bitrev32(y << (32 - power));
		factor = mbin_fet_32_generate_power(freq * z);
		printf("\t0x%08x,\n", factor);
	}

	printf("};\n\n");

top:
	printf("void\n"
	    "fet_%s_%d_32(uint32_t *data)\n"
	    "{\n", forward ? "forward" : "inverse", max);

	mbin_fet_generate_bitrev(power, br_type);

	printf("\n");

	printf("\t" "const uint32_t *ptab;\n");
	printf("\t" "const %s *p = bit_rev;\n", br_type);
	printf("\t" "uint64_t ta;\n");
	printf("\t" "uint64_t tb;\n");
	printf("\t" "uint64_t tc;\n");
	if (forward != 0) {
		printf("\t" "uint64_t te = ((uint64_t)data[0]) * 0x%xULL;\n",
		    0x10001 << (power - 2));
	}
	printf("\t" "uint32_t td;\n");
	printf("\t" "%s x;\n", br_type);
	printf("\t" "%s y;\n", br_type);
	printf("\n");

	for (n = 0; n != power; n++) {

		printf("\t" "/* round %d */\n", n);
		printf("\t" "ptab = fet_%d_32_table;\n", max);

		printf("\t" "for (y = 0; y != 0x%x; y += 0x%x) {\n",
		    max, (1 << (power - n)));

		if (n != 0) {
			printf("\t\t" "td = *ptab;\n");
			printf("\t\t" "ptab++;\n");
		}
		if (n != (power - 1)) {
			printf("\t\t" "for (x = 0; x != 0x%x; x++) {\n",
			    (max >> (n + 1)));
		} else {
			printf("\t\t" "x = 0;\n"
			    "\t\t" "if (1) {\n");
		}

		if (n == 0) {
			printf("\t\t\t" "ta = data[x];\n");
			printf("\t\t\t" "data[x] = ta;\n");
			printf("\t\t\t" "data[x+0x%x] = ta;\n",
			    (max >> (n + 1)));
		} else {
			printf("\t\t\t" "ta = data[x];\n");
			printf("\t\t\t" "tb = data[x+0x%x] * (int64_t)(td);\n",
			    (max >> (n + 1)));
			printf("\t\t\t" "tc = ta + tb%s;\n",
			    (forward != 0 && n == (power - 1)) ? " - te" : "");
			printf("\t\t\t" "tc = (tc >> 32) + ((uint32_t)tc);\n");
			printf("\t\t\t" "tc = (tc >> 32) + ((uint32_t)tc);\n");
			if (forward != 0 && n == (power - 1)) {
				printf("\t\t\t" "tc <<= %d;\n", 18 - power);
				printf("\t\t\t" "tc = (tc >> 32) + ((uint32_t)tc);\n");
				printf("\t\t\t" "tc = (tc >> 32) + ((uint32_t)tc);\n");
			}
			if (n == (power - 1))
				printf("\t\t\t" "if (tc == 0xFFFFFFFFULL) tc = 0;\n");

			printf("\t\t\t" "data[x] = tc;\n");
			printf("\t\t\t" "data[x+0x%x] = %s;\n",
			    (max >> (n + 1)), (n == (power - 1)) ? "0" : "tc");
		}

		printf("\t\t" "}\n");
		printf("\t\t" "data += 0x%x;\n", 1 << (power - n));
		printf("\t" "}\n");
		printf("\t" "data -= 0x%x;\n", max);
	}
	printf("\t/* In-place index bit-reversal */\n");

	printf(
	    "\t" "while (*p) {\n"
	    "\t\t" "uint32_t t;\n"
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

	if (forward == 0) {
		forward = 1;
		goto top;
	}
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
