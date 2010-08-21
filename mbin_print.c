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

/*
 * This file contains funtions for printing binary numbers.
 */
#include <stdio.h>
#include <stdint.h>

#include "math_bin.h"

void
mbin_print8(const char *fmt, uint8_t x)
{
	uint8_t mask = 0x80;

	if (fmt == NULL)
		fmt = "11111111";

	while (*fmt) {
		if (*fmt == '1') {
			putchar((x & mask) ? '1' : '.');
			mask /= 2;
		} else {
			putchar(*fmt);
		}
		fmt++;
	}
}

void
mbin_print16(const char *fmt, uint16_t x)
{
	uint16_t mask = 0x8000;

	if (fmt == NULL)
		fmt = "1111111111111111";

	while (*fmt) {
		if (*fmt == '1') {
			putchar((x & mask) ? '1' : '.');
			mask /= 2;
		} else {
			putchar(*fmt);
		}
		fmt++;
	}
}

void
mbin_print32(const char *fmt, uint32_t x)
{
	uint32_t mask = 0x80000000;

	if (fmt == NULL)
		fmt = "11111111111111111111111111111111";

	while (*fmt) {
		if (*fmt == '1') {
			putchar((x & mask) ? '1' : '.');
			mask /= 2;
		} else {
			putchar(*fmt);
		}
		fmt++;
	}
}

void
mbin_print64(const char *fmt, uint64_t x)
{
	uint64_t mask = 0x8000000000000000ULL;

	if (fmt == NULL) {
		fmt =
		    "11111111111111111111111111111111"
		    "11111111111111111111111111111111";
	}
	while (*fmt) {
		if (*fmt == '1') {
			putchar((x & mask) ? '1' : '.');
			mask /= 2;
		} else {
			putchar(*fmt);
		}
		fmt++;
	}
}

void
mbin_print8_abc(uint8_t x)
{
	mbin_print8_abc_mask(x, 0);
}

void
mbin_print8_abc_mask(uint8_t x, uint8_t m)
{
	const char *str = "abcdefgh";

	if (x == 0) {
		putchar('1');
	} else {
		while (x) {
			if (x & 1) {
				if (m & 1)
					putchar(*str + ('A' - 'a'));
				else
					putchar(*str);
			}
			str++;
			x /= 2;
			m /= 2;
		}
	}
}

void
mbin_print16_abc(uint16_t x)
{
	mbin_print16_abc_mask(x, 0);
}

void
mbin_print16_abc_mask(uint16_t x, uint16_t m)
{
	const char *str = "abcdefghijklmnop";

	if (x == 0) {
		putchar('1');
	} else {
		while (x) {
			if (x & 1) {
				if (m & 1)
					putchar(*str + ('A' - 'a'));
				else
					putchar(*str);
			}
			str++;
			x /= 2;
			m /= 2;
		}
	}
}

void
mbin_print32_abc(uint32_t x)
{
	mbin_print32_abc_mask(x, 0);
}

void
mbin_print32_abc_mask(uint32_t x, uint32_t m)
{
	const char *str = "abcdefghijklmnopqrstuvwxyz??????";

	if (x == 0) {
		putchar('1');
	} else {
		while (x) {
			if (x & 1) {
				if (m & 1)
					putchar(*str + ('A' - 'a'));
				else
					putchar(*str);
			}
			str++;
			x /= 2;
			m /= 2;
		}
	}
}

void
mbin_print32_const(uint32_t x)
{
	while (x) {
		if (x & 1) {
			printf("1, ");
		} else {
			printf("0, ");
		}
		x /= 2;
	}
}

uint32_t
mbin_print_xor_analyse_fwd_32x32(uint32_t *ptr,
    uint32_t mask, uint32_t fslices)
{
	uint32_t x;
	uint32_t count;
	uint32_t tcount;
	uint32_t fslice;

	tcount = 0;

	for (fslice = 1; fslice; fslice *= 2) {

		if (!(fslices & fslice))
			continue;

		if (fslices != fslice)
			printf("Level = 0x%08x\n", fslice);

		/* Do the XOR transform */
		mbin_transform_xor_fwd_32x32(ptr,
		    mask, fslice, 0x80000000);

		/* Reset count */
		count = 0;
		x = 0;

		/* Print out result */
		while (1) {
			if (ptr[x] & 0x80000000) {
				mbin_print32_abc(x);
				printf("\n");
				count++;
			}
			if (x == mask)
				break;
			x++;
		}
		printf("\nCount:%u\n\n", count);
		tcount += count;
	}
	return (tcount);
}

static void
mbin_print_number_factorise(uint32_t *temp, uint32_t mask,
    uint32_t val, uint32_t level, uint8_t mbits, uint8_t do_xor)
{
	uint32_t myfact;
	uint32_t factor;
	uint32_t x;
	uint8_t prev;

	factor = temp[val];
	temp[val] = 0;

	myfact = factor;

	if (myfact & 0x80000000) {
		printf("-");
		myfact = -myfact;
	}
	printf("0x%x*(", myfact / level);
	mbin_print32_abc(val);
	prev = 1;

	while (factor) {
		x = 0;
		while (1) {
			if (temp[x] == factor) {
				temp[x] = 0;
				if (prev)
					printf(" ^ ");
				mbin_print32_abc(x);
				prev = 1;
			}
			if (x == mask)
				break;
			x++;
		}
		if (!(factor & mask))
			break;
		printf(",");
		prev = 0;
		factor *= 2;
	}

	printf(")\n");
}

static void
mbin_print_multi_factorise(uint32_t *temp, uint32_t mask,
    uint32_t val, uint32_t level, uint8_t mbits, uint8_t do_xor)
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t an;
	uint32_t bn;
	uint32_t am;
	uint32_t bm;
	uint32_t lmask;
	uint32_t hmask;
	uint32_t aodd;
	uint32_t bodd;
	uint8_t ao;
	uint8_t bo;
	uint8_t co;
	uint8_t op;

	lmask = (1 << (mbits / 2)) - 1;
	hmask = ~lmask & mask;

	a = val & lmask;
	b = val & hmask;
	c = temp[a | b];

	if (a != (b >> (mbits / 2))) {

		/* not factorisable */
		printf("(");
		mbin_print32_const((int32_t)c / (int32_t)level);
		printf(")*(");
		mbin_print32_abc(a);
		printf(")*(");
		mbin_print32_abc(b);
		printf(")\n");

		if (do_xor)
			temp[a | b] ^= c;
		else
			temp[a | b] -= c;
		return;
	}
	co = 0;
	aodd = a;
	bodd = b;
	while (1) {
		if ((2 * aodd) & ~lmask)
			break;
		if ((2 * bodd) & ~hmask)
			break;
		if ((co + 1) == (mbits / 2))
			break;
		aodd *= 2;
		bodd *= 2;
		co++;
		if (temp[(aodd & lmask) | (bodd & hmask)]) {
			continue;
		}
		aodd |= 1;
		bodd |= 1 << (mbits / 2);
		if (temp[(aodd & lmask) | (bodd & hmask)]) {
			continue;
		}
		co--;
		aodd /= 2;
		bodd /= 2;
		break;
	}

	op = 0;

repeat:

	for (ao = co, am = c;; am *= 2, ao--) {
		an = (aodd >> ao) & lmask;
		for (bo = co, bm = am;; bm *= 2, bo--) {
			bn = (bodd >> bo) & hmask;
			if (op == 0) {
				if (!(temp[an | bn] & bm))
					goto print_and;
			} else {
				if (do_xor)
					temp[an | bn] ^= bm;
				else
					temp[an | bn] -= bm;
			}
			if (bo == 0)
				break;
			if (bn == 0)
				break;
		}
		if (ao == 0)
			break;
		if (an == 0)
			break;
	}

	if (op == 0) {
		op = 1;
		goto repeat;
	}
	printf("(");
	mbin_print32_const((int32_t)c / (int32_t)level);
	printf(")*(");
	for (ao = co;; ao--) {
		an = (aodd >> ao) & lmask;
		mbin_print32_abc(an);
		printf(", ");
		if (ao == 0)
			break;
		if (an == 0)
			break;
	}
	printf(")*(");

	for (bo = co;; bo--) {
		bn = (bodd >> bo) & hmask;
		mbin_print32_abc(bn);
		printf(", ");
		if (bo == 0)
			break;
		if (bn == 0)
			break;
	}
	printf(")\n");
	return;

print_and:
	printf("(");
	mbin_print32_const((int32_t)c / (int32_t)level);
	printf(")*(");
	for (bo = co, ao = co, bm = c, am = c;;
	    bm *= 2, am *= 2, bo--, ao--) {
		an = (aodd >> ao) & lmask;
		bn = (bodd >> bo) & hmask;
		if (do_xor)
			temp[an | bn] ^= bm;
		else
			temp[an | bn] -= bm;

		mbin_print32_abc(an | bn);
		printf(",");
		if (bo == 0)
			break;
		if (bn == 0)
			break;
		if (ao == 0)
			break;
		if (an == 0)
			break;
	}
	printf(")\n");
}

static void
mbin_print_simple_factorise(uint32_t *temp, uint32_t mask,
    uint32_t val, uint32_t level, uint8_t mbits,
    uint8_t do_xor, uint8_t do_mask)
{
	uint32_t m;
	uint32_t n;
	uint32_t z;
	uint32_t t;
	uint32_t u;
	uint32_t hmask;

	hmask = (-(1 << (mbits / 2))) & mask;

	if (val == 0)
		m = 0;
	else
		m = mbin_msb32(val);

	/* get expression root */
	z = val ^ m;
	n = level;

	mbin_print32_abc(z);

	printf("(");

	/* print factored expression */
	while (n & mask) {

		m &= mask;

		u = z | m;

		if (!(temp[u] & -n))
			break;

		if (temp[u] & n) {
#if 1
			/* print out */
			t = temp[u];
			if (do_mask)
				t &= mask;
			t = ((int32_t)(t &
			    (-n))) / (int32_t)n;
			if (t == 1);
			else if (t == 0xFFFFFFFF)
				printf("-");
			else
				printf("0x%x*", t);
			temp[u] &= (n - 1);
#else
			temp[u] &= ~n;
#endif
			mbin_print32_abc(m);
			printf(",");
		} else {
			printf(" ,");
		}

		m *= 2;
		n *= 2;
	}

	printf(")\n");
}

uint32_t
mbin_print_multi_analyse_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    const char *pdelete, uint32_t mask, uint8_t do_xor)
{
	uint32_t tcount;
	uint32_t count;
	uint32_t x;
	uint32_t y;
	uint8_t mbits;

	if (do_xor & 0x10) {
		mbin_transform_gte_fwd_32x32(ptr, temp, mask);
		if (pdelete != NULL) {
			printf("Removing GTE statements is not supported\n");
			return (0);
		}
	} else if (do_xor & 0x01) {
		mbin_transform_multi_xor_fwd_32x32(ptr, temp, mask);
		mbin_parse32_xor(pdelete, temp, mask);
	} else {
		mbin_transform_add_fwd_32x32(ptr, temp, mask);
		mbin_parse32_add(pdelete, temp, mask);
	}

	tcount = 0;
	mbits = mbin_sumbits32(mask);

	if (do_xor & 0x40) {
		y = 0;
		while (1) {
			if (temp[y]) {
				printf("0x%08x * ", temp[y]);
				mbin_print32_abc(y);
				printf("\n");
				tcount++;
			}
			if (y == mask)
				break;
			y++;
		}
		return (tcount);
	}
	for (x = 1; x & mask; x *= 2) {
		printf("Level = 0x%08x\n", x);
		count = 0;
		y = 0;
		while (1) {

			if (temp[y] & x) {
				if (do_xor & 0x20) {
					mbin_print_number_factorise(temp,
					    mask, y, x, mbits, (do_xor & 0x01));
				} else if (do_xor & 0x80) {
					mbin_print_multi_factorise(temp,
					    mask, y, x, mbits, (do_xor & 0x01));
				} else {
					mbin_print_simple_factorise(temp,
					    mask, y, x, mbits, (do_xor & 0x01),
					    (do_xor & 0x02));
				}
				count++;
			}
			if (y == mask)
				break;
			y++;
		}

		printf("\nCount = %u\n\n", count);

		tcount += count;
	}
	return (tcount);
}
