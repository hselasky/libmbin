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
    uint32_t fslice, uint32_t max)
{
	uint32_t x;
	uint32_t count;

	/* Do the XOR transform */
	mbin_transform_xor_fwd_32x32(ptr,
	    max - 1, fslice, 0x80000000);

	/* Reset count */
	count = 0;

	/* Print out result */
	for (x = 0; x != max; x++) {
		if (ptr[x] & 0x80000000) {
			mbin_print32_abc(x);
			printf("\n");
			count++;
		}
	}
	printf("\nCount:%u\n", count);
	return (count);
}
