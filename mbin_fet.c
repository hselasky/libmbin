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

#define	FET32_MOD		0xFFFFFFFFU
#define	FET32_SHIFT		16
#define	FET32_FACTOR		7
#define	FET32_LIMIT		6
#undef	FET32_VERIFY

uint32_t
mbin_fet_32_generate_power(uint32_t f, uint32_t y)
{
	return (mbin_power_mod_32(f, y, FET32_MOD));
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
mbin_fet_64_step(const char *ma, const char *mb,
    uint8_t sa, uint8_t sb, uint8_t arch)
{
	switch (arch) {
	case FET_ARCH_AMD64:
		printf("\t" "asm(\n");
		printf("\t" "\t" "\"movq %%3, %%%%rax;\"\n");
		printf("\t" "\t" "\"mov %%%%rax, %%%%rbx;\"\n");
		if (sa != 0)
			printf("\t" "\t" "\"rol $%u, %%%%rax;\"\n", sa);
		if (sb != 0)
			printf("\t" "\t" "\"rol $%u, %%%%rbx;\"\n", sb);
		printf("\t" "\t" "\"movq %%2, %%%%rcx;\"\n");
		printf("\t" "\t" "\"add %%%%rcx, %%%%rax;\"\n");
		printf("\t" "\t" "\"adc $0, %%%%rax;\"\n");
		printf("\t" "\t" "\"movq %%%%rax, %%0;\"\n");
		printf("\t" "\t" "\"add %%%%rcx, %%%%rbx;\"\n");
		printf("\t" "\t" "\"adc $0, %%%%rbx;\"\n");
		printf("\t" "\t" "\"movq %%%%rbx, %%1;\"\n");
		printf("\t" "\t" ": \"=m\" (%s), \"=m\" (%s)\n", ma, mb);
		printf("\t" "\t" ": \"m\" (%s), \"m\" (%s)\n", ma, mb);
		printf("\t" "\t" ": \"memory\", \"rax\", \"rbx\", \"rcx\"\n");
		printf("\t" ");\n");
		break;
	default:
		printf("\t" "do {\n");
		printf("\t" "\t" "" "uint128_t t0;\n");
		printf("\t" "\t" "" "uint128_t t1;\n\n");
		printf("\t" "\t" "t0 = ((uint128_t)(uint64_t)%s) + "
		    "(((uint128_t)(uint64_t)%s) << %u);\n", ma, mb, sa);
		printf("\t" "\t" "t0 = (uint64_t)t0 + (t0 >> 64);\n");
		printf("\t" "\t" "t0 = (uint64_t)t0 + (t0 >> 64);\n");
		printf("\t" "\t" "t1 = ((uint128_t)(uint64_t)%s) + "
		    "(((uint128_t)(uint64_t)%s) << %u);\n", ma, mb, sb);
		printf("\t" "\t" "t1 = (uint64_t)t1 + (t1 >> 64);\n");
		printf("\t" "\t" "t1 = (uint64_t)t1 + (t1 >> 64);\n");
		printf("\t" "\t" "%s = t0;\n", ma);
		printf("\t" "\t" "%s = t1;\n", mb);
		printf("\t" "} while (0);\n");
		break;
	}
}

static uint8_t
mbin_fet_64_generate_power(uint8_t power, uint32_t x)
{
	x = mbin_bitrev32(x << (32 - power)) << (6 - power);
	return (x % 64);
}

static void
mbin_fet_64_generate_r2(uint32_t step, uint8_t power, uint8_t arch)
{
	uint32_t fet_max = 1U << power;
	uint32_t fet_step = step;
	uint32_t pk0 = 0;
	uint32_t x;
	uint32_t y;
	char ma[16];
	char mb[16];

	for (y = 0; y != fet_max; y += 2 * fet_step) {
		uint32_t k0 = pk0;
		uint32_t k1 = pk0 + 1;

		pk0 += 2;

		for (x = 0; x != fet_step; x++) {

			snprintf(ma, sizeof(ma), "data[%u]", y + x);
			snprintf(mb, sizeof(mb), "data[%u]", y + x + fet_step);

			mbin_fet_64_step(ma, mb,
			    mbin_fet_64_generate_power(power, k0),
			    mbin_fet_64_generate_power(power, k1), arch);
		}
	}
}

static void
mbin_fet_64_generate_fixup(const char *indent, uint8_t power, uint8_t arch)
{
	switch (arch) {
	case FET_ARCH_AMD64:
		printf(indent);
		printf("asm(\n");
		printf(indent);
		printf("\t" "\"movq %%1, %%%%rax;\"\n");
		printf(indent);
		printf("\t" "\"rol $%u, %%%%rax;\"\n", 32 - power);
		printf(indent);
		printf("\t" "\"mov %%%%rax, %%%%rbx;\"\n");
		printf(indent);
		printf("\t" "\"shr $32, %%%%rax;\"\n");
		printf(indent);
		printf("\t" "\"sub %%%%rbx, %%%%rax;\"\n");
		printf(indent);
		printf("\t" "\"movq %%%%rax, %%0;\"\n");
		printf(indent);
		printf("\t" ": \"=m\" (data[x])\n");
		printf(indent);
		printf("\t" ": \"m\" (data[x])\n");
		printf(indent);
		printf("\t" ": \"memory\", \"rax\", \"rbx\"\n");
		printf(indent);
		printf(");\n");
		break;
	default:
		printf(indent);
		printf("uint64_t temp;\n");
		printf(indent);
		printf("temp = data[x];\n");
		printf(indent);
		printf("temp = (temp << %u) | (temp >> %u);\n", 32 - power, 32 + power);
		printf(indent);
		printf("temp = (uint32_t)((temp >> 32) - temp);\n");
		printf(indent);
		printf("data[x] = temp;\n");
		break;
	}
}

void
mbin_fet_64_generate(uint8_t power, uint8_t arch)
{
	const char *br_type;
	uint32_t max;
	uint32_t n;

	if (power > 6 || power < 1) {
		printf("#error \"Invalid power\"\n");
		return;
	}
	br_type = "uint8_t";

	max = 1 << power;

	printf("void\n"
	    "fet_inverse_%d_64(uint64_t *data)\n"
	    "{\n", max);

	mbin_fet_generate_bitrev(power, br_type);

	printf("\n");

	printf("\t" "const %s *p = bit_rev;\n", br_type);

	printf("\t" "%s x;\n", br_type);
	printf("\t" "%s y;\n", br_type);

	for (n = 0; n != power; n++) {
		printf("\t" "/* round %d - r2 */\n", n);
		mbin_fet_64_generate_r2(1U << (power - n - 1), power, arch);
	}

	printf("\t/* In-place index bit-reversal */\n");

	printf(
	    "\t" "while (*p) {\n"
	    "\t\t" "uint64_t t;\n"
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
	    "fet_conv_%d_64(uint64_t *a, uint64_t *b, uint64_t *c)\n"
	    "{\n", max);
	printf("\t" "uint32_t x;\n");
	printf("\t" "for (x = 0; x != 0x%x; x++) {\n", max);
	switch (arch) {
	case FET_ARCH_AMD64:
		printf("\t\t" "asm (\n");
		printf("\t\t\t" "\"movq %%1, %%%%rax;\"\n");
		printf("\t\t\t" "\"mulq %%2;\"\n");
		printf("\t\t\t" "\"add %%%%rax, %%%%rdx;\"\n");
		printf("\t\t\t" "\"adc $0, %%%%rdx;\"\n");
		printf("\t\t\t" "\"movq %%%%rdx, %%0;\"\n");
		printf("\t\t\t" ": \"=m\" (c[x])\n");
		printf("\t\t\t" ": \"m\" (a[x]), \"m\" (b[x])\n");
		printf("\t\t\t" ": \"memory\", \"rax\", \"rdx\"\n");
		printf("\t\t" ");\n");
		break;
	default:
		printf("\t" "\t" "uint128_t temp;\n");
		printf("\t" "\t" "temp = ((uint128_t)a[x]) * ((uint128_t)b[x]);\n");
		printf("\t" "\t" "temp = (uint64_t)temp + (temp >> 64);\n");
		printf("\t" "\t" "temp = (uint64_t)temp + (temp >> 64);\n");
		printf("\t" "\t" "c[x] = temp;\n");
		break;
	}
	printf("\t" "}\n");
	printf("}\n");

	printf("void\n"
	    "fet_forward_%d_64(uint64_t *data)\n"
	    "{\n", max);

	printf("\t" "%s x;\n\n", br_type);

	printf("\t" "fet_inverse_%d_64(data);\n", max);
	printf("\n");
	printf("\t" "for (x = 0; x != 0x%x; x++) {\n", max);
	mbin_fet_64_generate_fixup("\t\t", power, arch);
	printf("\t" "}\n");
	printf("\t" "for (x = 1; x != 0x%x; x++) {\n", max / 2);
	printf("\t" "\t" "uint64_t temp;\n");
	printf("\t" "\t" "temp = data[x];\n");
	printf("\t" "\t" "data[x] = data[0x%x - x];\n", max);
	printf("\t" "\t" "data[0x%x - x] = temp;\n", max);
	printf("\t" "}\n");
	printf("}\n");
}

static void
mbin_fet_32_add_mul(const char *indent, const char *va,
    const char *vb, const char *vc, const char *vr,
    uint8_t arch)
{
	switch (arch) {
	case FET_ARCH_I386:
	case FET_ARCH_AMD64:
		printf(indent);
		printf("asm(\n");
		printf(indent);
		printf("\t" "\"mul %%2\\n\"\n");
		printf(indent);
		printf("\t" "\"add %%%%edx, %%0\\n\"\n");
		printf(indent);
		printf("\t" "\"adc $0, %%0\\n\"\n");
		printf(indent);
		printf("\t" "\"add %%3, %%0\\n\"\n");
		printf(indent);
		printf("\t" "\"adc $0, %%0\\n\" :\n");
		printf(indent);
		printf("\t" "\"=a\" (%s) : \n", vr);
		printf(indent);
		printf("\t" "\"a\" (%s), \"%s\" (%s), \"r\" (%s) : \"%%edx\");\n", vb, "r", vc, va);
		break;
	default:			/* generic */
		printf(indent);
		printf("%s = ((uint64_t)(uint32_t)%s) + (((uint64_t)(uint32_t)%s) * "
		    "((uint64_t)(uint32_t)%s));\n", vr, va, vb, vc);
		printf(indent);
		printf("%s = (uint32_t)%s + (%s >> 32);\n", vr, vr, vr);
		printf(indent);
		printf("%s = (uint32_t)%s + (%s >> 32);\n", vr, vr, vr);
		break;
	}
}

static void
mbin_fet_32_generate_r2(uint32_t step, uint32_t max, uint8_t arch)
{
	const char *indent;

	indent = "\t";

	printf(indent);
	printf("do {\n");

	indent = "\t\t";

	printf(indent);
	printf("enum { FET_MAX = 0x%x, FET_STEP = 0x%x };\n", max, step);
	printf(indent);
	printf("\n");

	printf(indent);
	printf("const uint32_t *pk0 = fet_%d_32_table;\n", max);
	printf(indent);
	printf("\n");

	printf(indent);
	printf("for (y = 0; y != FET_MAX; y += 2 * FET_STEP) {\n");

	indent = "\t\t\t";

	printf(indent);
	printf("uint32_t k0 = pk0[0];\n");
	printf(indent);
	printf("uint32_t k1 = pk0[1];\n");
	printf(indent);
	printf("pk0 += 2;\n");

	printf(indent);
	printf("for (x = 0; x != FET_STEP; x++) {\n");

	indent = "\t\t\t\t";

	printf(indent);
	printf("uint32_t t0;\n");
	printf(indent);
	printf("uint32_t t1;\n");
	if (arch == FET_ARCH_GENERIC) {
		printf(indent);
		printf("uint64_t temp;\n");
	} else {
		printf(indent);
		printf("uint32_t temp;\n");
	}
	printf(indent);
	printf("\n");
	printf(indent);
	printf("t0 = data[y + x];\n");
	printf(indent);
	printf("t1 = data[y + x + FET_STEP];\n");
	printf(indent);
	printf("\n");
	mbin_fet_32_add_mul(indent, "t0", "t1", "k0", "temp", arch);
	printf(indent);
	printf("data[y + x] = temp;\n");
	printf(indent);
	printf("\n");
	mbin_fet_32_add_mul(indent, "t0", "t1", "k1", "temp", arch);
	printf(indent);
	printf("data[y + x + FET_STEP] = temp;\n");

	indent = "\t\t\t";

	printf(indent);
	printf("}\n");

	indent = "\t\t";

	printf(indent);
	printf("}\n");

	indent = "\t";

	printf(indent);
	printf("} while (0);\n");
}

void
mbin_fet_32_generate(uint8_t power, uint8_t arch)
{
	const char *br_type;
	uint32_t f;
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

	if (power < FET32_LIMIT) {
		f = 2;
		freq = 1U << (5 - power);
	} else {
		f = FET32_FACTOR;
		freq = 1U << (FET32_SHIFT - power);
	}
	for (y = 0; y != max; y++) {
		uint32_t factor;

		z = mbin_bitrev32(y << (32 - power));
		factor = mbin_fet_32_generate_power(f, freq * z);
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
		printf("\t" "/* round %d - r2 */\n", n);
		mbin_fet_32_generate_r2(1U << (power - n - 1), max, arch);
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
	    "fet_conv_%d_32(uint32_t *a, uint32_t *b, uint32_t *c)\n"
	    "{\n", max);
	printf("\t" "uint32_t x;\n");
	printf("\t" "for (x = 0; x != 0x%x; x++) {\n", max);
	printf("\t" "\t" "uint64_t temp;\n");
	printf("\t" "\t" "temp = ((uint64_t)a[x]) * ((uint64_t)b[x]);\n");
	printf("\t" "\t" "temp = (uint32_t)temp + (temp >> 32);\n");
	printf("\t" "\t" "temp = (uint32_t)temp + (temp >> 32);\n");
	printf("\t" "\t" "c[x] = temp;\n");
	printf("\t" "}\n");
	printf("}\n");

	printf("void\n"
	    "fet_forward_%d_32(uint32_t *data)\n"
	    "{\n", max);

	printf("\t" "uint32_t t1;\n\n");
	printf("\t" "%s x;\n\n", br_type);

	printf("\t" "fet_inverse_%d_32(data);\n", max);

	printf("\t" "t1 = data[0];\n");
	printf("\t" "t1 = (t1 << %u) | (t1 >> %u);\n", 16 - power, 16 + power);
	printf("\t" "t1 = (uint16_t)((t1 >> 16) - t1);\n");
	printf("\t" "data[0] = t1;\n");

	printf("\t" "for (x = 1; x != 0x%x; x++) {\n", (max / 2) + 1);
	printf("\t" "\t" "uint32_t temp;\n");
	printf("\t" "\t" "temp = data[x];\n");
	printf("\t" "\t" "t1 = data[0x%x-x];\n", max);
	printf("\t" "\t" "t1 = (t1 << %u) | (t1 >> %u);\n", 16 - power, 16 + power);
	printf("\t" "\t" "t1 = (uint16_t)((t1 >> 16) - t1);\n");
	printf("\t" "\t" "data[x] = t1;\n");
	printf("\t" "\t" "t1 = temp;\n");
	printf("\t" "\t" "t1 = (t1 << %u) | (t1 >> %u);\n", 16 - power, 16 + power);
	printf("\t" "\t" "t1 = (uint16_t)((t1 >> 16) - t1);\n");
	printf("\t" "\t" "data[0x%x-x] = t1;\n", max);
	printf("\t" "}\n");
	printf("}\n");
}

static void
mbin_fet_inverse_small_64_sub(uint64_t *data, uint8_t power, uint8_t step)
{
	uint64_t mask = (1ULL << power) - 1;
	uint32_t x;
	uint32_t y;
	uint32_t k;
	uint8_t s = 32 - mbin_sumbits32(power - 1);

	for (k = y = 0; y != power; k += 2, y += 2 * step) {
		uint32_t k0 = mbin_bitrev32(k << s);
		uint32_t k1 = k0 | (power / 2);

		for (x = 0; x != step; x++) {
			uint64_t t0;
			uint64_t t1;
			uint64_t temp;

			t0 = data[y + x];
			t1 = data[y + x + step];

			temp = t0 + (t1 << k0);
			temp = (temp & mask) + (temp >> power);
			temp = (temp & mask) + (temp >> power);
			data[y + x] = temp;

			temp = t0 + (t1 << k1);
			temp = (temp & mask) + (temp >> power);
			temp = (temp & mask) + (temp >> power);
			data[y + x + step] = temp;
		}
	}
}

void
mbin_fet_inverse_small_64(uint64_t *data, uint8_t power)
{
	uint8_t n;
	uint8_t i;
	uint8_t j;

	if (power > 5)
		return;

	for (n = 0; n != power; n++) {
		mbin_fet_inverse_small_64_sub(data, 1 << power,
		    1 << (power - n - 1));
	}

	/* In-place data order bit-reversal */

	for (i = 0; i != 1U << power; i++) {

		j = mbin_bitrev32(i << (32 - power));

		if (j < i) {
			uint64_t temp;

			temp = data[i];
			data[i] = data[j];
			data[j] = temp;
		}
	}
}

void
mbin_fet_conv_small_64(const uint64_t *a, const uint64_t *b,
    uint64_t *c, uint8_t power)
{
	uint8_t max = (1U << power);
	uint8_t x;
	uint64_t mask = (1ULL << max) - 1ULL;
	uint64_t temp;

	for (x = 0; x != max; x++) {
		temp = a[x] * b[x];
		temp = (temp & mask) + (temp >> max);
		temp = (temp & mask) + (temp >> max);
		c[x] = temp;
	}
}

void
mbin_fet_forward_small_64(uint64_t *data, uint8_t power)
{
	uint8_t max = (1U << power);
	uint8_t max_half = (max / 2);
	uint8_t x;
	uint64_t mask = (1ULL << max_half) - 1ULL;
	uint64_t temp;

	mbin_fet_inverse_small_64(data, power);

	for (x = 0; x != max; x++) {
		temp = data[x];
		temp = (temp << (max_half - power)) |
		    (temp >> (max_half + power));
		temp = mask & ((temp >> max_half) - temp);
		data[x] = temp;
	}

	for (x = 1; x != max_half; x++) {
		temp = data[x];
		data[x] = data[max - x];
		data[max - x] = temp;
	}
}

void
mbin_fet_cpy_64(uint64_t *dst, const uint64_t *src, uint32_t num)
{
#ifdef FET32_VERIFY
	printf("CPY %d\n", num);
#endif
	memcpy(dst, src, num * 8);
}

void
mbin_fet_add_64(const uint64_t *pa, const uint64_t *pb,
    uint64_t *pc, uint32_t num, uint8_t is_mod)
{
#ifdef FET32_VERIFY
	uint64_t temp[num];
	uint32_t x;
	uint32_t carry = 0;

	printf("ADD %d qwords %s\n", num, is_mod ? "mod" : "");

	memset(temp, 0, num * 8);

	for (x = 0; x != (num * 64); x++) {
		carry += ((pa[x / 64] & (1ULL << (x % 64))) ? 1 : 0) +
		    ((pb[x / 64] & (1ULL << (x % 64))) ? 1 : 0);
		if (carry & 1)
			temp[x / 64] |= (1ULL << (x % 64));
		carry /= 2;
	}

	memcpy(pc, temp, sizeof(temp));

	if (is_mod) {
		while (carry) {
			for (x = 0; x != (num * 64); x++) {
				carry += ((pc[x / 64] & (1ULL << (x % 64))) ? 1 : 0);
				if (carry & 1)
					pc[x / 64] |= (1ULL << (x % 64));
				else
					pc[x / 64] &= ~(1ULL << (x % 64));
				carry /= 2;
			}
		}
	}
#else
	uint64_t carry;
	uint64_t temp;
	uint32_t x;
	uint32_t z;

	if (num == 1) {
		pc[0] = pa[0] + pb[0];
		if (is_mod) {
			if (pc[0] < pa[0]) {
				pc[0] += 1;
				if (pc[0] < 1)
					pc[0] += 1;
			}
		}
		return;
	}
	carry = 0;
	z = num & ~3;

	for (x = 0; x != z; x += 4) {
		temp = carry + pa[x];
		pc[x] = temp + pb[x];
		carry = (pc[x] < temp) + (temp < carry);

		temp = carry + pa[x + 1];
		pc[x + 1] = temp + pb[x + 1];
		carry = (pc[x + 1] < temp) + (temp < carry);

		temp = carry + pa[x + 2];
		pc[x + 2] = temp + pb[x + 2];
		carry = (pc[x + 2] < temp) + (temp < carry);

		temp = carry + pa[x + 3];
		pc[x + 3] = temp + pb[x + 3];
		carry = (pc[x + 3] < temp) + (temp < carry);
	}

	for (; x != num; x++) {
		temp = carry + pa[x];
		pc[x] = temp + pb[x];
		carry = (pc[x] < temp) + (temp < carry);
	}

	if (is_mod == 0)
		return;

	while (carry) {
		for (x = 0; x != num; x++) {
			temp = carry + pc[x];
			carry = (temp < carry);
			pc[x] = temp;
		}
	}
#endif
}

void
mbin_fet_sub_64(const uint64_t *pa, const uint64_t *pb,
    uint64_t *pc, uint32_t num)
{
#ifdef FET32_VERIFY
	uint64_t temp[num];
	uint32_t x;
	uint32_t carry = 0;

	printf("SUB %d\n", num);

	memset(temp, 0, num * 8);

	for (x = 0; x != (num * 64); x++) {
		carry = ((pa[x / 64] & (1ULL << (x % 64))) ? 1 : 0) -
		    ((pb[x / 64] & (1ULL << (x % 64))) ? 1 : 0) - carry;
		if (carry & 1)
			temp[x / 64] |= (1ULL << (x % 64));
		if (carry & 2)
			carry = 1;
		else
			carry = 0;
	}

	memcpy(pc, temp, num * 8);

#else
	uint64_t carry;
	uint64_t temp;
	uint32_t x;
	uint32_t z;

	if (num == 1) {
		pc[0] = pa[0] - pb[0];
		return;
	}
	carry = 1;
	z = num & ~3;

	for (x = 0; x != z; x += 4) {
		temp = carry + pa[x];
		pc[x] = temp + (~pb[x]);
		carry = (pc[x] < temp) + (temp < carry);

		temp = carry + pa[x + 1];
		pc[x + 1] = temp + (~pb[x + 1]);
		carry = (pc[x + 1] < temp) + (temp < carry);

		temp = carry + pa[x + 2];
		pc[x + 2] = temp + (~pb[x + 2]);
		carry = (pc[x + 2] < temp) + (temp < carry);

		temp = carry + pa[x + 3];
		pc[x + 3] = temp + (~pb[x + 3]);
		carry = (pc[x + 3] < temp) + (temp < carry);
	}

	for (; x != num; x++) {
		temp = carry + pa[x];
		pc[x] = temp + (~pb[x]);
		carry = (pc[x] < temp) + (temp < carry);
	}
#endif
}

void
mbin_fet_rol_64(uint64_t *pa, uint32_t shift, uint32_t num)
{
#ifdef FET32_VERIFY
	uint64_t temp[num];
	uint32_t x;

	printf("ROL %d qwords %d bits\n", num, shift);

	memset(temp, 0, sizeof(temp));

	for (x = 0; x != (num * 64); x++) {
		if (pa[x / 64] & (1ULL << (x % 64)))
			temp[((x + shift) & ((num * 64) - 1)) / 64] |= (1ULL << ((x + shift) & 63));
	}

	memcpy(pa, temp, sizeof(temp));
#else
	uint8_t rem = (shift & 63);
	uint8_t mer = 64 - rem;
	uint32_t div = (shift / 64) & (num - 1);
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint64_t temp;
	uint64_t carry;

	if (num == 1) {
		shift &= 63;
		if (shift != 0)
			pa[0] = (pa[0] << shift) | (pa[0] >> (64 - shift));
		return;
	}
	if (rem) {
		carry = pa[num - 1];

		z = num & ~3;

		for (x = 0; x != z; x += 4) {
			temp = pa[x];
			pa[x] = (temp << rem) | (carry >> mer);
			carry = temp;
			temp = pa[x + 1];
			pa[x + 1] = (temp << rem) | (carry >> mer);
			carry = temp;
			temp = pa[x + 2];
			pa[x + 2] = (temp << rem) | (carry >> mer);
			carry = temp;
			temp = pa[x + 3];
			pa[x + 3] = (temp << rem) | (carry >> mer);
			carry = temp;
		}
		for (; x != num; x++) {
			temp = pa[x];
			pa[x] = (temp << rem) | (carry >> mer);
			carry = temp;
		}
	}
	if (div) {
		uint32_t delta = ((~div) & (div - 1)) + 1;

		for (x = 0; x != delta; x++) {
			y = x;
			carry = pa[y];
			do {
				y = (y + div) & (num - 1);
				temp = pa[y];
				pa[y] = carry;
				carry = temp;
			} while (x != y);
		}
	}
#endif
}

static void
mbin_fet_inverse_64_sub(uint64_t *data, uint32_t max, uint32_t step)
{
	uint32_t x;
	uint32_t y;
	uint32_t k;
	uint32_t num = max >> 6;
	uint8_t d = mbin_sumbits32(max - 1) - 6;
	uint8_t s = 32 - d - 6;
	uint64_t temp[num];

	for (k = y = 0; y != max; k += 2, y += 2 * step) {

		uint32_t k0 = mbin_bitrev32(k << s);
		uint32_t k1 = k0 | (max / 2);

		for (x = 0; x != step; x++) {

			uint64_t *p0 = data + ((y + x) << d);
			uint64_t *p1 = data + ((y + x + step) << d);

			mbin_fet_cpy_64(temp, p1, num);

			mbin_fet_rol_64(temp, k1, num);

			mbin_fet_add_64(p0, temp, p1, num, 1);

			mbin_fet_rol_64(temp, k0 - k1, num);

			mbin_fet_add_64(p0, temp, p0, num, 1);
		}
	}
}

void
mbin_fet_inverse_64(uint64_t *data, uint8_t power)
{
	uint8_t n;

	if (power < 6)
		return;

	for (n = 0; n != power; n++) {
		mbin_fet_inverse_64_sub(data,
		    1 << power,
		    1 << (power - n - 1));
	}
}

void
mbin_fet_bitrev_64(uint64_t *data, uint8_t power, uint32_t num)
{
	uint32_t max = (1 << power);
	uint32_t i;
	uint32_t j;
	uint8_t d = power - 6;
	uint8_t s = 32 - power;

	if (num == 0)
		return;

	if (num == 1) {

		/* In-place data order bit-reversal */

		for (i = 0; i != max; i++) {

			j = mbin_bitrev32(i << s);

			if (j < i) {
				uint64_t *p1 = data + (i << d);
				uint64_t *p2 = data + (j << d);
				uint64_t temp;

				temp = *p1;
				*p1 = *p2;
				*p2 = temp;
			}
		}
	} else {
		uint64_t temp[num];

		/* In-place data order bit-reversal */

		for (i = 0; i != max; i++) {

			j = mbin_bitrev32(i << s);

			if (j < i) {
				uint64_t *p1 = data + (i << d);
				uint64_t *p2 = data + (j << d);

				mbin_fet_cpy_64(temp, p1, num);
				mbin_fet_cpy_64(p1, p2, num);
				mbin_fet_cpy_64(p2, temp, num);
			}
		}
	}
}

void
mbin_fet_forward_64(uint64_t *data, uint8_t power)
{
	uint32_t num = (1 << power) >> 6;
	uint32_t max = (1 << power);
	uint32_t max_half = (max / 2);
	uint32_t x;
	uint8_t d = power - 6;

	if (num == 0)
		return;

	mbin_fet_inverse_64(data, power);

	if (num == 1) {
		for (x = 0; x != max; x++) {
			data[x] = (data[x] << (max_half - power)) | (data[x] >> (max_half + power));
			data[x] = (uint32_t)((data[x] >> 32) - data[x]);
		}

		mbin_fet_bitrev_64(data, power, 1);

		for (x = 1; x != max_half; x++) {
			uint64_t *p1 = data + (x << d);
			uint64_t *p2 = data + ((max - x) << d);
			uint64_t temp;

			temp = *p1;
			*p1 = *p2;
			*p2 = temp;
		}
	} else {
		for (x = 0; x != max; x++) {
			uint64_t *p1 = data + (x << d);
			uint64_t *p2 = data + (x << d) + (1 << (d - 1));

			mbin_fet_rol_64(p1, (max_half - power), num);
			mbin_fet_sub_64(p2, p1, p1, num / 2);
			memset(p2, 0, num * (8 / 2));	/* XXX can leave this */
		}

		num /= 2;		/* the upper half is zero */

		mbin_fet_bitrev_64(data, power, num);

		for (x = 1; x != max_half; x++) {
			uint64_t *p1 = data + (x << d);
			uint64_t *p2 = data + ((max - x) << d);
			uint64_t temp[num];

			mbin_fet_cpy_64(temp, p1, num);
			mbin_fet_cpy_64(p1, p2, num);
			mbin_fet_cpy_64(p2, temp, num);
		}
	}
}

void
mbin_fet_conv_64(const uint64_t *a, const uint64_t *b, uint64_t *c,
    mbin_fet_mul_64_t *func, uint8_t power, uint8_t arg)
{
	uint32_t num = (1 << power) >> 6;
	uint32_t max = (1 << power);
	uint32_t x;
	uint8_t d = power - 6;

	if (num == 0)
		return;

	for (x = 0; x != max; x++)
		func(a + (x << d), b + (x << d), c + (x << d), num, arg);
}

void
mbin_fet_write_64(uint64_t *ptr, uint64_t val, uint32_t *poff,
    uint32_t limit_bits, uint32_t max_bits)
{
	uint64_t temp;
	uint32_t off;
	uint32_t rem;
	uint32_t bits = 64;
	uint32_t delta;

	off = *poff;

	while (bits != 0) {
		rem = limit_bits - (off & (max_bits - 1U));

		if (rem == 0) {
			off += max_bits;
			off &= -max_bits;
			continue;
		}
		if (rem < bits) {
			temp = val & ((1ULL << rem) - 1ULL);
			val >>= rem;
			bits -= rem;
			delta = rem;
			rem = 0;
		} else {
			temp = val;
			rem -= bits;
			delta = bits;
			bits = 0;
		}

		if (off & 63) {
			ptr[(off / 64)] |= temp << (off & 63);
			ptr[(off / 64) + 1] |= temp >> (64 - (off & 63));
		} else {
			ptr[(off / 64)] |= temp;
		}

		off += delta;
	}

	*poff = off;
}

uint64_t
mbin_fet_read_64(uint64_t *ptr, uint32_t *poff,
    uint32_t off_bits, uint32_t limit_bits, uint32_t max_bits)
{
	uint32_t off;
	uint32_t rem;
	uint32_t bits = 64;
	uint32_t temp;
	uint64_t val = 0;

	off = *poff;

	while (bits != 0) {

		rem = limit_bits - (off & (max_bits - 1U));

		if (rem == 0) {
			off += max_bits;
			off &= -max_bits;
			continue;
		}
		temp = off + off_bits;
		if (temp & 63) {
			val |= (ptr[(temp / 64)] >> (temp & 63)) << (64 - bits);
			val |= (ptr[(temp / 64) + 1] << (64 - (temp & 63))) << (64 - bits);
		} else {
			val |= ptr[(temp / 64)] << (64 - bits);
		}

		if (rem < bits) {
			bits -= rem;
			if (bits != 0)
				val &= (1ULL << (64 - bits)) - 1ULL;
			off += rem;
			rem = 0;
		} else {
			rem -= bits;
			off += bits;
			bits = 0;
		}
	}

	*poff = off;

	return (val);
}

static mbin_fet_mul_64_t *mbin_fet_mul_64_ptr = &mbin_fet_mul_64;

void
mbin_fet_mul_64_set_ptr(mbin_fet_mul_64_t *ptr)
{
	mbin_fet_mul_64_ptr = ptr;
}

void
mbin_fet_mul_64_1(const uint64_t *a, const uint64_t *b,
    uint64_t *c, uint32_t num, uint8_t is_mod)
{
#ifdef __amd64__
	if (is_mod) {
		__asm(
		    "mov %1, %%rax\n"
		    "mov %2, %%rdx\n"
		    "mul %%rdx\n"
		    "add %%rdx, %%rax\n"
		    "adc $0, %%rax\n"
		    "mov %%rax, %0\n"
:		    "=m"(*c)
:		    "m"(*a), "m"(*b)
:		    "memory", "rax", "rdx"
		);
	} else {
		__asm(
		    "mov %2, %%rax\n"
		    "mov %3, %%rdx\n"
		    "mul %%rdx\n"
		    "mov %%rax, %0\n"
		    "mov %%rdx, %1\n"
:		    "=m"(c[0]), "=m"(c[1])
:		    "m"(*a), "m"(*b)
:		    "memory", "rax", "rdx"
		    );
	}
#else
	uint64_t p1;
	uint64_t p2;
	uint64_t p3;
	uint64_t p4;
	uint64_t carry;
	uint64_t temp[2];

	p1 = ((uint64_t)(uint32_t)a[0]) * ((uint64_t)(uint32_t)b[0]);
	p2 = ((uint64_t)(uint32_t)(a[0] >> 32)) * ((uint64_t)(uint32_t)b[0]);
	p3 = ((uint64_t)(uint32_t)a[0]) * ((uint64_t)(uint32_t)(b[0] >> 32));
	p4 = ((uint64_t)(uint32_t)(a[0] >> 32)) * ((uint64_t)(uint32_t)(b[0] >> 32));

	if (is_mod) {

		p2 = (p2 << 32) | (p2 >> 32);
		p3 = (p3 << 32) | (p3 >> 32);

		carry = 0;
		p1 = p1 + p2;
		if (p1 < p2)
			carry++;
		p1 = p1 + p3;
		if (p1 < p3)
			carry++;
		p1 = p1 + p4;
		if (p1 < p4)
			carry++;
		p1 = p1 + carry;
		if (p1 < carry) {
			p1 = p1 + 1;
			if (p1 < 1)
				p1 = p1 + 1;
		}
		c[0] = p1;
	} else {
		c[0] = p1;
		c[1] = 0;

		temp[0] = p2 << 32;
		temp[1] = p2 >> 32;

		mbin_fet_add_64(c, temp, c, 2, 0);

		temp[0] = p3 << 32;
		temp[1] = p3 >> 32;

		mbin_fet_add_64(c, temp, c, 2, 0);

		temp[0] = 0;
		temp[1] = p4;

		mbin_fet_add_64(c, temp, c, 2, 0);
	}
#endif
}

void
mbin_fet_mul_64_2(const uint64_t *a, const uint64_t *b,
    uint64_t *c, uint32_t num, uint8_t is_mod)
{
	uint64_t p1[2], p2[2], p3[2], p4[2];
	uint64_t temp[4];

	mbin_fet_mul_64_1(a + 0, b + 0, p1, 1, 0);
	mbin_fet_mul_64_1(a + 1, b + 0, p2, 1, 0);
	mbin_fet_mul_64_1(a + 0, b + 1, p3, 1, 0);
	mbin_fet_mul_64_1(a + 1, b + 1, p4, 1, 0);

	if (is_mod) {

		temp[0] = p2[0];
		p2[0] = p2[1];
		p2[1] = temp[0];

		temp[0] = p3[0];
		p3[0] = p3[1];
		p3[1] = temp[0];

		mbin_fet_add_64(p1, p2, c, 2, 1);
		mbin_fet_add_64(p3, p4, temp, 2, 1);
		mbin_fet_add_64(c, temp, c, 2, 1);
	} else {

		c[0] = p1[0];
		c[1] = p1[1];
		c[2] = 0;
		c[3] = 0;

		temp[0] = 0;
		temp[1] = p2[0];
		temp[2] = p2[1];
		temp[3] = 0;

		mbin_fet_add_64(c, temp, c, 4, 0);

		temp[0] = 0;
		temp[1] = p3[0];
		temp[2] = p3[1];
		temp[3] = 0;

		mbin_fet_add_64(c, temp, c, 4, 0);

		temp[0] = 0;
		temp[1] = 0;
		temp[2] = p4[0];
		temp[3] = p4[1];

		mbin_fet_add_64(c, temp, c, 4, 0);
	}

}

void
mbin_fet_mul_64(const uint64_t *a, const uint64_t *b,
    uint64_t *c, uint32_t num, uint8_t is_mod)
{
	uint64_t *t0;
	uint64_t *t1;
	uint64_t *t2;
	uint64_t *t3;
	uint64_t *t4;
	uint32_t bits;
	uint32_t tnum;
	uint32_t max_bits;
	uint32_t off;
	uint32_t x;
	uint8_t power;

	switch (num) {
	case 0:
		return;
	case 1:
		mbin_fet_mul_64_1(a, b, c, num, is_mod);
		return;
	case 2:
		mbin_fet_mul_64_2(a, b, c, num, is_mod);
		return;
	case 3:
		if (mbin_fet_mul_64_ptr == mbin_fet_mul_64)
			break;
		mbin_fet_mul_64_ptr(a, b, c, num, is_mod);
		return;
	case 4:
		if (mbin_fet_mul_64_ptr == mbin_fet_mul_64)
			break;
		mbin_fet_mul_64_ptr(a, b, c, num, is_mod);
		return;
	default:
		break;
	}

	for (power = 6;; power++) {
		bits = (1 << (power - 2)) - power - 1;

		if ((bits << power) >= (num * 64 * 2))
			break;
	}

	tnum = (1 << power) << (power - 6);
	max_bits = (1 << power);

	t0 = alloca(8 * tnum);
	t1 = alloca(8 * tnum);

	memset(t0, 0, 8 * tnum);
	memset(t1, 0, 8 * tnum);

	for (off = x = 0; x != num; x++)
		mbin_fet_write_64(t0, a[x], &off, bits, max_bits);

	for (off = x = 0; x != num; x++)
		mbin_fet_write_64(t1, b[x], &off, bits, max_bits);

	mbin_fet_inverse_64(t0, power);

	mbin_fet_inverse_64(t1, power);

	switch (max_bits / 64) {
	case 1:
		mbin_fet_conv_64(t0, t1, t0, &mbin_fet_mul_64_1, power, 1);
		break;
	case 2:
		mbin_fet_conv_64(t0, t1, t0, &mbin_fet_mul_64_2, power, 1);
		break;
	default:
		mbin_fet_conv_64(t0, t1, t0, mbin_fet_mul_64_ptr, power, 1);
		break;
	}

	mbin_fet_bitrev_64(t0, power, max_bits / 64);

	mbin_fet_forward_64(t0, power);

	t2 = alloca(2 * 8 * num);
	t3 = alloca(2 * 8 * num);
	t4 = alloca(2 * 8 * num);

	for (off = x = 0; x != (2 * num); x++)
		t2[x] = mbin_fet_read_64(t0, &off, 0, bits, max_bits);

	for (off = x = 0; x != (2 * num); x++)
		t3[x] = mbin_fet_read_64(t0, &off, bits, bits, max_bits);

	for (off = x = 0; x != (2 * num); x++)
		t4[x] = mbin_fet_read_64(t0, &off, 2 * bits, bits, max_bits);

	mbin_fet_rol_64(t3, bits, 2 * num);
	mbin_fet_rol_64(t4, 2 * bits, 2 * num);

	if (is_mod) {
#if 1
		mbin_fet_add_64(t3, t2, t2, 2 * num, 1);
		mbin_fet_add_64(t4, t2, t2, 2 * num, 1);
#else
		mbin_fet_add_64(t3, t2, t2, num, 1);
		mbin_fet_add_64(t4, t2, t2, num, 1);
		mbin_fet_add_64(t3 + num, t2 + num, t2 + num, num, 1);
		mbin_fet_add_64(t4 + num, t2 + num, t2 + num, num, 1);
#endif
		mbin_fet_add_64(t2, t2 + num, c, num, 1);
	} else {
		mbin_fet_add_64(t2, t3, c, 2 * num, 0);
		mbin_fet_add_64(t4, c, c, 2 * num, 0);
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
