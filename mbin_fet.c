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
#define	FET32_SHIFT		16
#define	FET32_FACTOR		7

uint32_t
mbin_fet_32_generate_power(uint32_t y)
{
	return (mbin_power_mod_32(FET32_FACTOR, y, FET32_PRIME));
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

		if (j < i)
			lim++;
	}

	printf("\t" "static const %s bit_rev[0x%x] = {\n",
	    br_type, (2 * lim) + 8);

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
}

static void
mbin_fet_32_add_mul(const char *indent, const char *va,
    const char *vb, const char *vc, const char *vr)
{
#if 0
	printf("%s" "%s = %s + (uint64_t)%s * (uint64_t)%s;\n",
	    indent, vr, va, vb, vc);
	printf("%s" "%s = (uint32_t)%s + (%s >> 32);\n",
	    indent, vr, vr, vr);
	printf("%s" "%s = (uint32_t)%s + (%s >> 32);\n",
	    indent, vr, vr, vr);
#else
	printf("%s" "asm(\n", indent);
	printf("%s\t" "\"mul %%2\\n\"\n", indent);
	printf("%s\t" "\"add %%%%edx, %%0\\n\"\n", indent);
	printf("%s\t" "\"adc $0, %%0\\n\"\n", indent);
	printf("%s\t" "\"add %%3, %%0\\n\"\n", indent);
	printf("%s\t" "\"adc $0, %%0\\n\" :\n", indent);
	printf("%s\t" "\"=a\" (%s) : \n", indent, vr);
	printf("%s\t" "\"a\" (%s), \"%s\" (%s), \"r\" (%s) : \"%%edx\");\n",
	    indent, vb, "r", vc, va);
#endif
}

static void
mbin_fet_32_generate_r2(uint32_t step, uint32_t max)
{
	const char *indent;

	indent = "\t";

	printf("%s" "do {\n", indent);

	indent = "\t\t";

	printf("%s" "enum { FET_MAX = 0x%x, FET_STEP = 0x%x };\n", indent, max, step);
	printf("%s" "\n", indent);

	printf("%s" "const uint32_t *pk0 = fet_%d_32_table;\n", indent, max);
	printf("%s" "\n", indent);

	printf("%s" "for (y = 0; y != FET_MAX; y += 2 * FET_STEP) {\n", indent);

	indent = "\t\t\t";

	printf("%s" "uint32_t k0 = pk0[0];\n", indent);
	printf("%s" "uint32_t k1 = pk0[1];\n", indent);
	printf("%s" "pk0 += 2;\n", indent);

	printf("%s" "for (x = 0; x != FET_STEP; x++) {\n", indent);

	indent = "\t\t\t\t";

	printf("%s" "uint32_t t0;\n", indent);
	printf("%s" "uint32_t t1;\n", indent);
	printf("%s" "uint32_t temp;\n", indent);
	printf("%s" "\n", indent);
	printf("%s" "t0 = data[y + x];\n", indent);
	printf("%s" "t1 = data[y + x + FET_STEP];\n", indent);
	printf("%s" "\n", indent);
	mbin_fet_32_add_mul(indent, "t0", "t1", "k0", "temp");
	printf("%s" "data[y + x] = temp;\n", indent);
	printf("%s" "\n", indent);
	mbin_fet_32_add_mul(indent, "t0", "t1", "k1", "temp");
	printf("%s" "data[y + x + FET_STEP] = temp;\n", indent);

	indent = "\t\t\t";

	printf("%s" "}\n", indent);

	indent = "\t\t";

	printf("%s" "}\n", indent);

	indent = "\t";

	printf("%s" "} while (0);\n", indent);
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

	if (power > FET32_SHIFT || power < 2) {
		printf("#error \"Invalid power\"\n");
		return;
	}
	br_type = (power <= 7) ? "uint8_t" :
	    (power <= 15) ? "uint16_t" :
	    "uint32_t";

	max = 1 << power;

	printf("static const uint32_t "
	    "fet_%d_32_table[0x%x] = {\n",
	    max, max);

	freq = 1U << (FET32_SHIFT - power);
	for (y = 0; y != max; y++) {
		uint32_t factor;

		z = mbin_bitrev32(y << (32 - power));
		factor = mbin_fet_32_generate_power(freq * z);
		printf("\t0x%08x,\n", factor);
	}

	printf("};\n\n");

	printf("void\n"
	    "fet_inverse_%d_32(uint32_t *data)\n"
	    "{\n", max);

	mbin_fet_generate_bitrev(power, br_type);

	printf("\n");

	printf("\t" "const %s *p = bit_rev;\n", br_type);

	printf("\t" "%s x;\n", br_type);
	printf("\t" "%s y;\n", br_type);

	for (n = 0; n != power; n++) {
		if (1) {
			printf("\t" "/* round %d - r2 */\n", n);
			mbin_fet_32_generate_r2(1U << (power - n - 1), max);
		}
	}

	printf("\t/* In-place index bit-reversal */\n");

	printf(
	    "\t" "while (*p) {\n"
	    "\t\t" "uint32_t t;\n"
	    "\t\t" "%s a;\n"
	    "\t\t" "%s b;\n\n"
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
	    "\t" "}\n", br_type, br_type);

	printf("}\n\n");

	printf("void\n"
	    "fet_forward_%d_32(uint32_t *data)\n"
	    "{\n", max);

	printf("\t" "uint64_t t0 = data[0];\n\n");
	printf("\t" "uint64_t t1;\n\n");
	printf("\t" "%s x;\n\n", br_type);

	printf("\t" "fet_inverse_%d_32(data);\n", max);

	printf("\t" "t0 = t0 * 0x%xULL;\n", 0x10001 << (power - 1));
	printf("\t" "t0 = ((uint64_t)(uint32_t)t0) + (t0 >> 32);\n");
	printf("\t" "t0 = ((uint64_t)(uint32_t)t0) + (t0 >> 32);\n");
	printf("\t" "t0 = 0xFFFFFFFFULL - t0;\n");

	printf("\t" "t1 = data[0];\n");
	printf("\t" "t1 = t1 + t0;\n");
	printf("\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "t1 <<= %d;\n", 17 - power);
	printf("\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "if (t1 == 0xFFFFFFFFULL)\n");
	printf("\t" "\t" "t1 = 0;\n");
	printf("\t" "else\n");
	printf("\t" "\t" "t1 = (uint16_t)-t1;\n");
	printf("\t" "data[0] = t1;\n");

	printf("\t" "for (x = 1; x != 0x%x; x++) {\n", (max / 2) + 1);
	printf("\t" "\t" "uint32_t temp;\n");
	printf("\t" "\t" "temp = data[x];\n");
	printf("\t" "\t" "t1 = data[0x%x-x];\n", max);
	printf("\t" "\t" "t1 = t1 + t0;\n");
	printf("\t" "\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "\t" "t1 <<= %d;\n", 17 - power);
	printf("\t" "\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "\t" "if (t1 == 0xFFFFFFFFULL)\n");
	printf("\t" "\t" "\t" "t1 = 0;\n");
	printf("\t" "\t" "else\n");
	printf("\t" "\t" "\t" "t1 = (uint16_t)-t1;\n");
	printf("\t" "\t" "data[x] = t1;\n");
	printf("\t" "\t" "t1 = temp;\n");
	printf("\t" "\t" "t1 = t1 + t0;\n");
	printf("\t" "\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "\t" "t1 <<= %d;\n", 17 - power);
	printf("\t" "\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "\t" "t1 = ((uint64_t)(uint32_t)t1) + (t1 >> 32);\n");
	printf("\t" "\t" "if (t1 == 0xFFFFFFFFULL)\n");
	printf("\t" "\t" "\t" "t1 = 0;\n");
	printf("\t" "\t" "else\n");
	printf("\t" "\t" "\t" "t1 = (uint16_t)-t1;\n");
	printf("\t" "\t" "data[0x%x-x] = t1;\n", max);
	printf("\t" "}\n");
	printf("}\n");
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
