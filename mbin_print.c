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
	const char *str = "abcdefgh";

	if (x == 0)
		putchar('1');
	else
		while (x) {
			if (x & 1)
				putchar(*str);
			str++;
			x /= 2;
		}
}

void
mbin_print16_abc(uint16_t x)
{
	const char *str = "abcdefghijklmnop";

	if (x == 0)
		putchar('1');
	else
		while (x) {
			if (x & 1)
				putchar(*str);
			str++;
			x /= 2;
		}
}

void
mbin_print32_abc(uint32_t x)
{
	const char *str = "abcdefghijklmnopqrstuvwxyz??????";

	if (x == 0)
		putchar('1');
	else
		while (x) {
			if (x & 1)
				putchar(*str);
			str++;
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

uint32_t
mbin_print_multi_analyse_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask, uint8_t do_xor)
{
	uint32_t tcount;
	uint32_t count;
	uint32_t m;
	uint32_t n;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t t;
	uint32_t u;
	uint8_t mbits;

	if (do_xor)
		mbin_transform_multi_xor_fwd_32x32(ptr, temp, mask);
	else
		mbin_transform_add_fwd_32x32(ptr, temp, mask);

	tcount = 0;
	mbits = mbin_sumbits32(mask);

	for (x = 1; x & mask; x *= 2) {
		printf("Level = 0x%08x\n", x);
		count = 0;
		y = 0;
		while (1) {
			if (temp[y] & x) {
				if (y == 0)
					m = 0;
				else {
					m = 0;
					for (t = mbits / 2; t != mbits; t++) {
						if (y & (1 << t))
							m |= (1 << t);
					}
				}

				/* get expression root */
				z = y ^ m;
				n = x;

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
						t = ((int32_t)(temp[u] &
						    (-n))) / (int32_t)n;
						if (t == 1);
						else if (t == 0xFFFFFFFF)
							printf("-");
						else
							printf("%d*", t);
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
