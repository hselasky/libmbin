/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

#define	FET32_PRIME		0xC0000001U
#define	FET32_RING_SIZE		0x30000000U

uint32_t
mbin_fet_32_generate_power(uint64_t y)
{
	uint64_t r = 1;
	uint64_t t = 2;

	y %= (FET32_PRIME - 1);

	while (y) {
		if (y & 1) {
			r *= t;
			r %= FET32_PRIME;
		}
		t *= t;
		t %= FET32_PRIME;

		y /= 2;
	}
	return (r);
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
mbin_fet_32_fix_factor(uint32_t factor, uint32_t mul, uint32_t mod)
{
	factor = (((uint64_t)factor) * ((uint64_t)mul)) % FET32_PRIME;

	/* correctly handle negative values */
	if (factor >= (mod / 2))
		factor -= mod;

	return (factor);
}

void
mbin_fet_32_generate(uint8_t power)
{
	uint32_t *ktable;
	uint32_t *ktemp;
	const char *br_type;
	uint32_t freq;
	uint32_t size;
	uint32_t factor;
	uint32_t prem;
	uint32_t max;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t n;
	uint32_t mod;

	if (power >= 30 || power <= 0) {
		printf("#error \"Invalid power\"\n");
		return;
	}
	br_type = (power <= 8) ? "uint8_t" :
	    (power <= 16) ? "uint16_t" :
	    "uint32_t";

	printf("void\n"
	    "fet_%d_32(int64_t *data)\n", 1 << power);

	printf("{\n");

	mbin_fet_generate_bitrev(power, br_type);

	size = (power << power) * sizeof(ktable[0]);

#ifdef FET32_PROVE
	ktable = malloc(size);

	if (ktable == NULL) {
		printf("#error \"Out of memory\"\n");
		return;
	}
	memset(ktable, 0, size);

	size = (1 << power) * sizeof(ktemp[0]);

	ktemp = malloc(size);

	if (ktemp == NULL) {
		printf("#error \"Out of memory\"\n");
		free(ktable);
		return;
	}
	memset(ktemp, 0, size);
#endif

#ifndef FET32_PROVE
	ktable = NULL;
	ktemp = NULL;
#endif

	max = 1 << power;

	mod = FET32_PRIME;

	prem = mbin_power_mod_32(2, 30, mod);

#ifdef FET32_PROVE
	for (freq = 0; freq != max; freq++) {

		uint32_t ta;
		uint32_t tb;

		for (x = 0; x != max; x++)
			ktemp[x] = mbin_fet_32_generate_power(((freq * x) % max) * (FET32_RING_SIZE / max));

		y = 0;
		for (n = 0; n != power; n++) {
			for (z = 0; z != (1 << (power - 1 - n)); z++) {
				ta = ktemp[y + z];
				tb = ktemp[y + z + (1 << (power - 1 - n))];

				factor = (((uint64_t)ta) *
				    ((uint64_t)mbin_power_mod_32(tb, mod - 2, mod))) % mod;

				if (freq & (1 << n)) {
					factor = (mod - factor) % mod;
				}
				size = (n << power) + y + z + (1 << (power - 1 - n));

				if ((ktable[size] != 0) && (ktable[size] != factor))
					goto error;

				ktable[size] = factor;

				factor = 1;

				size = (n << power) + y + z;

				if ((ktable[size] != 0) && (ktable[size] != factor))
					goto error;

				ktable[size] = factor;

				if (freq & (1 << n)) {
					ktemp[y + z] = 0;
					ktemp[y + z + (1 << (power - 1 - n))] = ((uint64_t)ta * (uint64_t)2) % mod;
				} else {
					ktemp[y + z] = ((uint64_t)ta * (uint64_t)2) % mod;
					ktemp[y + z + (1 << (power - 1 - n))] = 0;
				}
			}

			if (freq & (1 << n))
				y += (1 << (power - 1 - n));
		}
	}
#endif

	n = power - 1;
	x = 0;
	for (y = 0; y != max; y += (1 << (power - n))) {
		x += (max >> (n + 1));
	}

	printf("\t" "static const int32_t ktable[0x%x] = {\n", (int)x);

#ifdef FET32_PROVE
	for (n = (power - 1); n != power; n++) {
		for (y = 0; y != max; y += (1 << (power - n))) {
			for (x = 0; x != (max >> (n + 1)); x++) {

				z = ktable[(n << power) + x + y + (max >> (n + 1))];

				if (x != 0) {
					if (factor != z)
						goto error;
					continue;
				}
				factor = z;

				printf("\t\t%d,\n", mbin_fet_32_fix_factor(factor, prem, mod));
			}
		}
	}
#endif

#ifndef FET32_PROVE
	freq = (FET32_RING_SIZE >> power);
	for (y = 0; y != (1 << (power - 1)); y++) {
		z = mbin_bitrev32(y << (32 - (power - 1)));
		factor = mbin_fet_32_generate_power(FET32_RING_SIZE - (freq * z));
		printf("\t\t%d,\n", mbin_fet_32_fix_factor(factor, prem, mod));
	}
#endif

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

	prem = mbin_power_mod_32(2, (30 * (power + 1)), mod);

	prem = mbin_fet_32_fix_factor(prem, 1, mod);

	for (n = 0; n != power; n++) {

		printf("\t" "/* round %d */\n", n);
		printf("\t" "ptab = ktable;\n");

		printf("\t" "for (y = 0; y != 0x%x; y += 0x%x) {\n", max, 1 << (power - n));

		if (n != 0) {
			printf("\t\t" "td = *(ptab++);\n");
		}
		printf("\t\t" "for (x = 0; x != 0x%x; x++) {\n", (max >> (n + 1)));

		if (n != 0) {
			printf("\t\t\t" "ta = data[x];\n");
			printf("\t\t\t" "tb = data[x+0x%x] * (int64_t)(td);\n", (max >> (n + 1)));
			printf("\t\t\t" "tb = (tb >> 30) - ((tb & 0x3fffffff) * 3);\n");

			printf("\t\t\t" "tc = ta + tb;\n");
			printf("\t\t\t" "tc = (tc >> 30) - ((tc & 0x3fffffff) * 3);\n");
			printf("\t\t\t" "data[x] = tc;\n");

			printf("\t\t\t" "tc = ta - tb;\n");
			printf("\t\t\t" "tc = (tc >> 30) - ((tc & 0x3fffffff) * 3);\n");
			printf("\t\t\t" "data[x+0x%x] = tc;\n", (max >> (n + 1)));
		} else {
			printf("\t\t\t" "ta = data[x] * %dLL;\n", (int32_t)prem);
			printf("\t\t\t" "tb = data[x+0x%x] * %dLL;\n", (max >> (n + 1)), (int32_t)prem);

			printf("\t\t\t" "tc = ta + tb;\n");
			printf("\t\t\t" "tc = (tc >> 30) - ((tc & 0x3fffffff) * 3);\n");
			printf("\t\t\t" "tc = (tc >> 30) - ((tc & 0x3fffffff) * 3);\n");
			printf("\t\t\t" "data[x] = tc;\n");

			printf("\t\t\t" "tc = ta - tb;\n");
			printf("\t\t\t" "tc = (tc >> 30) - ((tc & 0x3fffffff) * 3);\n");
			printf("\t\t\t" "tc = (tc >> 30) - ((tc & 0x3fffffff) * 3);\n");
			printf("\t\t\t" "data[x+0x%x] = tc;\n", (max >> (n + 1)));
		}

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

	factor = (((uint64_t)mbin_power_mod_32(2, 2 * 2 * 30, mod)) *
	    ((uint64_t)mbin_power_mod_32(max, mod - 2, mod))) % mod;

	factor = mbin_fet_32_fix_factor(factor, 1, mod);

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
	    "\t\t\t" "ta += 0xC0000001LL;\n"
	    "\t\t" "if (ta < 0)\n"
	    "\t\t\t" "ta += 0xC0000001LL;\n"
	    "\t\t" "tb = b[x];\n"
	    "\t\t" "if (tb < 0)\n"
	    "\t\t\t" "tb += 0xC0000001LL;\n"
	    "\t\t" "if (tb < 0)\n"
	    "\t\t\t" "tb += 0xC0000001LL;\n"
	    "\t\t" "ta = ((uint64_t)(uint32_t)ta) * ((uint64_t)(uint32_t)tb);\n"
	    "\t\t" "ta = (((uint64_t)ta) >> 30) - ((ta & 0x3fffffff) * 3);\n"
	    "\t\t" "ta = (ta >> 30) - ((ta & 0x3fffffff) * 3);\n"
	    "\t\t" "ta = ta * (int64_t)%d;\n"
	    "\t\t" "ta = (ta >> 30) - ((ta & 0x3fffffff) * 3);\n"
	    "\t\t" "ta = (ta >> 30) - ((ta & 0x3fffffff) * 3);\n"
	    "\t\t" "c[x] = ta;\n"
	    "\t" "}\n",
	    max, factor);

	printf("\n"
	    "\t" "for (x = 1; x != 0x%x; x++) {\n"
	    "\t\t" "/* swap */\n"
	    "\t\t" "ta = c[0x%x - x];\n"
	    "\t\t" "tb = c[x];\n"
	    "\t\t" "c[x] = ta;\n"
	    "\t\t" "c[0x%x - x] = tb;\n"
	    "\t" "}\n", max / 2, max, max);

	printf("}\n");

	goto done;

#ifdef FET32_PROVE
error:
	printf("#error \"Wrong factor\"\n");
#endif

done:
	if (ktable != NULL)
		free(ktable);
	if (ktemp != NULL)
		free(ktemp);
	return;
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
