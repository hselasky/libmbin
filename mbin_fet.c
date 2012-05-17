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
 *
 * FET is an implementation of what I would call a "Roulade" (ROL+ADD)
 * theorem.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/endian.h>

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
mbin_fet_cpy_8(uint8_t *dst, const uint8_t *src, uint32_t bytes)
{
	memcpy(dst, src, bytes);
}

void
mbin_fet_mod_8(uint8_t *var, uint32_t bytes)
{
	uint32_t bytes_half = bytes / 2;

	if (mbin_fet_sub_8(var, var + bytes_half, var, 0, bytes_half)) {
		memset(var + bytes_half, 0, bytes_half);
		mbin_fet_add_carry_8(var, 0, bytes);
	} else {
		memset(var + bytes_half, 0, bytes_half);
	}
}

uint8_t
mbin_fet_add_carry_8(uint8_t *pa, uint8_t do_loop, uint32_t bytes)
{
	while (1) {
		uint64_t carry = 1;
		uint32_t x = 0;

#if defined(__i386__) || defined(__amd64__)
		for (; (x + 8) <= bytes; x += 8) {
			uint64_t temp;

			temp = le64toh(*(uint64_t *)(pa + x));
			if (++temp) {
				*(uint64_t *)(pa + x) = htole64(temp);
				carry = 0;
				break;
			}
			*(uint64_t *)(pa + x) = htole64(temp);
		}
#endif
		if (carry != 0) {
			for (; x != bytes; x++) {
				if (++pa[x]) {
					carry = 0;
					break;
				}
			}
		}
		if (carry == 0)
			break;
		if (do_loop == 0)
			return (carry);
	}
	return (0);
}

uint8_t
mbin_fet_sub_carry_8(uint8_t *pa, uint8_t do_loop, uint32_t bytes)
{
	while (1) {
		uint64_t carry = 1;
		uint32_t x = 0;

#if defined(__i386__) || defined(__amd64__)
		for (; (x + 8) <= bytes; x += 8) {
			uint64_t temp;

			temp = le64toh(*(uint64_t *)(pa + x));
			if (temp--) {
				*(uint64_t *)(pa + x) = htole64(temp);
				carry = 0;
				break;
			}
			*(uint64_t *)(pa + x) = htole64(temp);
		}
#endif
		if (carry != 0) {
			for (; x != bytes; x++) {
				if (pa[x]--) {
					carry = 0;
					break;
				}
			}
		}
		if (carry == 0)
			break;
		if (do_loop == 0)
			return (carry);
	}
	return (0);
}

uint8_t
mbin_fet_add_8(const uint8_t *pa, const uint8_t *pb, uint8_t *pc,
    uint8_t carry_in, uint32_t bytes)
{
	uint32_t x = 0;
	uint64_t carry = carry_in;

#if defined(__i386__) || defined(__amd64__)
	for (; (x + 8) <= bytes; x += 8) {
		uint64_t temp;
		uint64_t carry_out;

		temp = le64toh(*(uint64_t *)(pa + x));
		carry = carry + temp;
		carry_out = (carry < temp);
		temp = le64toh(*(uint64_t *)(pb + x));
		carry = carry + temp;
		carry_out += (carry < temp);
		*(uint64_t *)(pc + x) = htole64(carry);
		carry = carry_out;
	}
#endif
	for (; x != bytes; x++) {
		carry = pa[x] + pb[x] + carry;
		pc[x] = carry;
		carry >>= 8;
	}
	return (carry);
}

uint8_t
mbin_fet_sub_8(const uint8_t *pa, const uint8_t *pb,
    uint8_t *pc, uint8_t carry_in, uint32_t bytes)
{
	uint32_t x = 0;
	uint64_t carry = carry_in;

#if defined(__i386__) || defined(__amd64__)
	for (; (x + 8) <= bytes; x += 8) {
		uint64_t temp;
		uint64_t carry_out;

		temp = le64toh(*(uint64_t *)(pa + x));
		carry = temp - carry;
		carry_out = (carry > temp);
		temp = le64toh(*(uint64_t *)(pb + x));
		temp = carry - temp;
		carry_out += (temp > carry);
		*(uint64_t *)(pc + x) = htole64(temp);
		carry = carry_out;
	}
#endif
	for (; x != bytes; x++) {
		carry = pa[x] - pb[x] - carry;
		pc[x] = carry;
		carry >>= (sizeof(carry) * 8) - 1;
	}
	return (carry);
}

void
mbin_fet_rol_8(uint8_t *pa, uint32_t rol_bytes, uint32_t bytes)
{
	uint8_t temp[bytes] __aligned(8);

	rol_bytes &= (bytes - 1);

	if (rol_bytes == 0)
		return;

	memcpy(temp, pa + bytes - rol_bytes, rol_bytes);
	memcpy(temp + rol_bytes, pa, bytes - rol_bytes);
	memcpy(pa, temp, bytes);
}

void
mbin_fet_rol_bit_8(uint8_t *pa, uint32_t rol_bits, uint32_t bytes)
{
	uint32_t x;

	rol_bits &= 7;
	if (rol_bits == 0)
		return;

	if (bytes & 7) {
		uint8_t carry;
		uint8_t temp;

		carry = pa[bytes - 1];
		for (x = 0; x != bytes; x++) {
			temp = pa[x];
			pa[x] = (temp << rol_bits) | (carry >> (8 - rol_bits));
			carry = temp;
		}
	} else {
		uint64_t carry;
		uint64_t temp;

		carry = le64toh(*(uint64_t *)(pa + bytes - 8));
		for (x = 0; x != bytes; x += 8) {
			temp = le64toh(*(uint64_t *)(pa + x));
			carry = (temp << rol_bits) | (carry >> (64 - rol_bits));
			*(uint64_t *)(pa + x) = htole64(carry);
			carry = temp;
		}
	}
}

void
mbin_fet_shl_cpy_bit_8(uint8_t *pc, const uint8_t *pa, uint32_t shl_bits, uint32_t bytes)
{
	uint32_t x;
	uint64_t carry;
	uint64_t temp;

	if (shl_bits == 0) {
		memcpy(pc, pa, bytes);
		return;
	}
	carry = 0;
	x = 0;

#if defined(__i386__) || defined(__amd64__)
	for (; (x + 8) <= bytes; x += 8) {
		temp = le64toh(*(const uint64_t *)(pa + x));
		carry = (temp << shl_bits) | (carry >> (64 - shl_bits));
		*(uint64_t *)(pc + x) = htole64(carry);
		carry = temp;
	}

	carry >>= (64 - 8);
#endif

	for (; x <= bytes; x++) {
		temp = pa[x];
		pc[x] = (temp << shl_bits) | (carry >> (8 - shl_bits));
		carry = temp;
	}
}

static void
mbin_fet_inverse_8_sub(uint8_t *data, uint8_t power_size,
    uint8_t n, uint32_t power_var)
{
	uint32_t max = (1 << power_size);
	uint32_t step = (1 << (power_size - n - 1));
	uint32_t x;
	uint32_t y;
	uint32_t k;
	uint32_t bytes_half = 1U << (power_var - 4);
	uint8_t d = power_var - 3;
	uint8_t s = 32 - power_var;
	uint8_t c;
	uint8_t e;
	uint8_t temp[bytes_half + 1] __aligned(8);

	for (k = 0, y = 0; y != max; k += 2, y += 2 * step) {

		uint32_t shift = mbin_bitrev32(k << s);
		uint32_t shift_bytes = shift / 8;

		for (x = 0; x != step; x++) {
			uint8_t *p0 = data + ((y + x) << d);
			uint8_t *p1 = data + ((y + x + step) << d);

			mbin_fet_shl_cpy_bit_8(temp, p1, shift & 7, bytes_half + 1);

			/* p1 = p0 + (p1 << (shift + bytes_half)) */

			c = mbin_fet_sub_8(p0 + shift_bytes, temp, p1 + shift_bytes, 0, bytes_half - shift_bytes);

			e = p0[bytes_half];
			p1[bytes_half] = 0;
			c = mbin_fet_add_8(p0, temp + bytes_half - shift_bytes, p1, c, shift_bytes);
			c = mbin_fet_add_8(p1 + shift_bytes, temp + bytes_half, p1 + shift_bytes, c, 1);
			if (c)
				c = mbin_fet_add_carry_8(p1 + shift_bytes + 1, 0, bytes_half - shift_bytes - 1);
			if (c)
				c = mbin_fet_sub_carry_8(p1, 0, bytes_half);
			if (e)
				c += mbin_fet_sub_carry_8(p1, 0, bytes_half);
			while (c--)
				mbin_fet_add_carry_8(p1, 0, bytes_half + 1);

			/* p0 = p0 - (p1 << (shift + bytes_half)) */

			c = mbin_fet_add_8(p0 + shift_bytes, temp, p0 + shift_bytes, 0, bytes_half - shift_bytes);

			c += p0[bytes_half];
			p0[bytes_half] = 0;
			c = mbin_fet_sub_8(p0, temp + bytes_half - shift_bytes, p0, c, shift_bytes);
			c = mbin_fet_sub_8(p0 + shift_bytes, temp + bytes_half, p0 + shift_bytes, c, 1);
			if (c) {
				c = mbin_fet_sub_carry_8(p0 + shift_bytes + 1, 0, bytes_half - shift_bytes - 1);
				if (c)
					mbin_fet_add_carry_8(p0, 0, bytes_half + 1);
			}
		}
	}
}

void
mbin_fet_inverse_8(uint8_t *data, uint8_t power_size, uint8_t power_var)
{
	uint8_t n;

	for (n = 0; n != power_size; n++)
		mbin_fet_inverse_8_sub(data, power_size, n, power_var);
}

void
mbin_fet_bitrev_8(uint8_t *data, uint8_t power_size, uint8_t power_var)
{
	uint32_t max = (1U << power_size);
	uint32_t i;
	uint32_t j;
	uint8_t d = power_var - 3;
	uint8_t s = 32 - power_size;
	uint8_t temp[1U << d] __aligned(8);

	/* In-place data order bit-reversal */

	for (i = 0; i != max; i++) {

		j = mbin_bitrev32(i << s);

		if (j < i) {
			uint8_t *p1 = data + (i << d);
			uint8_t *p2 = data + (j << d);

			mbin_fet_cpy_8(temp, p1, 1U << d);
			mbin_fet_cpy_8(p1, p2, 1U << d);
			mbin_fet_cpy_8(p2, temp, 1U << d);
		}
	}
}

void
mbin_fet_forward_8(uint8_t *data, uint8_t power_size, uint8_t power_var)
{
	uint32_t size_max = (1U << power_size);
	uint32_t size_half = (1U << (power_size - 1));
	uint32_t var_max = (1U << (power_var - 3));
	uint32_t var_half = (1U << (power_var - 4));
	uint32_t x;
	uint32_t rol_bits = ((1U << power_var) - power_size);
	uint8_t *p1;
	uint8_t *p2;

	uint8_t temp[var_half] __aligned(8);

	mbin_fet_inverse_8(data, power_size, power_var);

	p1 = data;

	for (x = 0; x != size_max; x++) {
		mbin_fet_rol_8(p1, rol_bits / 8, var_max);
		mbin_fet_rol_bit_8(p1, rol_bits & 7, var_max);

		mbin_fet_mod_8(p1, var_max);

		p1 += var_max;
	}

	mbin_fet_bitrev_8(data, power_size, power_var);

	p1 = data + var_max;
	p2 = data + (var_max << power_size) - var_max;

	for (x = 1; x != size_half; x++) {
		mbin_fet_cpy_8(temp, p1, var_half);
		mbin_fet_cpy_8(p1, p2, var_half);
		mbin_fet_cpy_8(p2, temp, var_half);
		p1 += var_max;
		p2 -= var_max;
	}
}

void
mbin_fet_conv_8(const uint8_t *a, const uint8_t *b, uint8_t *c,
    mbin_fet_mul_8_t *func, uint8_t power_size, uint8_t power_var)
{
	uint32_t max = (1U << power_size);
	uint32_t x;
	uint32_t bytes = (1 << (power_var - 3));
	uint8_t temp[2 * bytes] __aligned(8);

	for (x = 0; x != max; x++) {

		func(a, b, temp, bytes);

		if (mbin_fet_add_8(temp, temp + bytes, c, 0, bytes))
			mbin_fet_add_carry_8(c, 1, bytes);

		mbin_fet_mod_8(c, bytes);

		a += bytes;
		b += bytes;
		c += bytes;
	}
}

void
mbin_fet_write_8(uint8_t *dst, const uint8_t *src,
    uint32_t cpy_limit, uint32_t dst_zero,
    uint32_t src_bytes, uint32_t dst_bytes)
{
	uint32_t off;
	uint8_t *dst_end = dst + dst_bytes;

	for (off = 0; (off + cpy_limit) <= src_bytes; off += cpy_limit) {
		mbin_fet_cpy_8(dst, src + off, cpy_limit);
		memset(dst + cpy_limit, 0, dst_zero);
		dst += dst_zero + cpy_limit;
	}

	mbin_fet_cpy_8(dst, src + off, src_bytes - off);
	dst += src_bytes - off;
	memset(dst, 0, dst_end - dst);
}

void
mbin_fet_read_8(const uint8_t *dst, uint8_t *src,
    uint32_t cpy_limit, uint32_t dst_zero,
    uint32_t src_bytes)
{
	uint32_t off;

	for (off = 0; (off + cpy_limit) <= src_bytes; off += cpy_limit) {
		mbin_fet_cpy_8(src + off, dst, cpy_limit);
		dst += dst_zero + cpy_limit;
	}

	mbin_fet_cpy_8(src + off, dst, src_bytes - off);
}

static mbin_fet_mul_8_t *mbin_fet_mul_8_ptr = &mbin_fet_mul_8;

void
mbin_fet_mul_8_set_ptr(mbin_fet_mul_8_t *ptr)
{
	mbin_fet_mul_8_ptr = ptr;
}

void
mbin_fet_mul_8_1(const uint8_t *a, const uint8_t *b,
    uint8_t *c, uint32_t bytes)
{
	uint32_t temp;

	temp = a[0] * b[0];

	c[0] = temp;
	c[1] = temp >> 8;
}

void
mbin_fet_mul_8_2(const uint8_t *a, const uint8_t *b,
    uint8_t *c, uint32_t bytes)
{
	uint32_t temp;
	uint32_t ta, tb;

	ta = le16toh(*((uint16_t *)a));
	tb = le16toh(*((uint16_t *)b));

	temp = ta * tb;

	*((uint32_t *)c) = htole32(temp);
}

void
mbin_fet_mul_8_4(const uint8_t *a, const uint8_t *b,
    uint8_t *c, uint32_t bytes)
{
	uint64_t temp;
	uint32_t ta, tb;

	ta = le32toh(*((uint32_t *)a));
	tb = le32toh(*((uint32_t *)b));

	temp = ((uint64_t)ta) * ((uint64_t)tb);

	*((uint64_t *)c) = htole64(temp);
}

void
mbin_fet_mul_8_8(const uint8_t *a, const uint8_t *b,
    uint8_t *c, uint32_t bytes)
{
	uint8_t temp[16] __aligned(16);
	uint8_t last[16] __aligned(16);

	memset(last, 0, sizeof(last));

	mbin_fet_mul_8_4(a, b, last, 4);

	memset(temp, 0, sizeof(temp));
	mbin_fet_mul_8_4(a + 4, b, temp + 4, 4);
	mbin_fet_add_8(last, temp, last, 0, 16);

	memset(temp, 0, sizeof(temp));
	mbin_fet_mul_8_4(a, b + 4, temp + 4, 4);
	mbin_fet_add_8(last, temp, last, 0, 16);

	memset(temp, 0, sizeof(temp));
	mbin_fet_mul_8_4(a + 4, b + 4, temp + 8, 4);
	mbin_fet_add_8(last, temp, last, 0, 16);

	memcpy(c, last, 16);
}

void
mbin_fet_mul_8(const uint8_t *a, const uint8_t *b,
    uint8_t *c, uint32_t in_bytes)
{
	uint8_t *t0;
	uint8_t *t1;
	uint8_t *t2;
	uint32_t x;
	uint32_t y;
	uint32_t limit_bytes;
	uint32_t zero_bytes;
	uint8_t power_size;
	uint8_t power_var;

	switch (in_bytes) {
	case 0:
		/* need at least one byte */
		return;
	case 1:
		mbin_fet_mul_8_1(a, b, c, in_bytes);
		return;
	case 2:
		mbin_fet_mul_8_2(a, b, c, in_bytes);
		return;
	case 4:
		mbin_fet_mul_8_4(a, b, c, in_bytes);
		return;
	case 8:
		mbin_fet_mul_8_8(a, b, c, in_bytes);
		return;
	default:
		break;
	}

	for (power_size = 6;; power_size++) {

		uint32_t total_bytes;

		limit_bytes = ((1U << (power_size - 2)) - power_size) / 8;

		total_bytes = limit_bytes << power_size;

		if (total_bytes < (in_bytes * 2))
			continue;

		power_var = power_size;

		while ((total_bytes / 2) >= (in_bytes * 2)) {
			power_size--;
			total_bytes = limit_bytes << power_size;
		}
		break;
	}

#ifdef FET_DEBUG
	printf("MUL ps=%d pv=%d limit=%d\n", power_size, power_var, limit_bytes);
#endif

	x = 1U << (power_size + power_var - 3);
	zero_bytes = (1U << (power_var - 3)) - limit_bytes;

	t0 = alloca(x);
	t1 = alloca(x);

	mbin_fet_write_8(t0, a, limit_bytes, zero_bytes, in_bytes, x);
	mbin_fet_write_8(t1, b, limit_bytes, zero_bytes, in_bytes, x);

	mbin_fet_inverse_8(t0, power_size, power_var);
	mbin_fet_inverse_8(t1, power_size, power_var);

	mbin_fet_conv_8(t0, t1, t0, mbin_fet_mul_8_ptr, power_size, power_var);

	mbin_fet_bitrev_8(t0, power_size, power_var);

	mbin_fet_forward_8(t0, power_size, power_var);

	t2 = alloca(2 * in_bytes);

	mbin_fet_read_8(t0, c, limit_bytes, zero_bytes, 2 * in_bytes);

	for (y = limit_bytes; y < ((limit_bytes + zero_bytes) / 2); y += limit_bytes) {

		mbin_fet_read_8(t0 + y, t2,
		    limit_bytes, zero_bytes, 2 * in_bytes);

		mbin_fet_rol_8(t2, y, 2 * in_bytes);

		mbin_fet_add_8(t2, c, c, 0, 2 * in_bytes);
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
