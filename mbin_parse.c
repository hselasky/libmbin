/*-
 * Copyright (c) 2009 Hans Petter Selasky
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
#include <ctype.h>

#include "math_bin.h"

uint32_t
mbin_parse32_abc(const char *ptr, const char *end)
{
	uint32_t temp = 0;
	char c;

	while (1) {
		if (ptr == end)
			break;
		if (*ptr == 0)
			break;
		c = *ptr++;
		if (c >= 'a' && c <= 'z')
			temp |= 1 << (c - 'a');
		else if (c >= 'A' && c <= 'Z')
			temp |= 1 << (c - 'A');
	}
	return (temp);
}

void
mbin_parse32_add(const char *ptr, uint32_t *ptable, uint32_t mask)
{
	if (ptr == NULL)
		return;
	while (ptr[0]) {
		if (ptr[0] == '(') {
			mbin_parse32_factor(ptr, ptable, mask, 0, 0, 0);
		}
		/* skip line */
		while (1) {
			if (ptr[0] == 0)
				break;
			if (ptr[0] == '\n') {
				ptr++;
				break;
			}
			ptr++;
		}
	}
}

void
mbin_parse32_xor(const char *ptr, uint32_t *ptable, uint32_t mask)
{
	if (ptr == NULL)
		return;
	while (ptr[0]) {
		if (ptr[0] == '(') {
			mbin_parse32_factor(ptr, ptable, mask, 0, 0, 1);
		}
		/* skip line */
		while (1) {
			if (ptr[0] == 0)
				break;
			if (ptr[0] == '\n') {
				ptr++;
				break;
			}
			ptr++;
		}
	}
}

void
mbin_parse32_factor(const char *ptr, uint32_t *ptable, uint32_t mask,
    uint32_t var, uint8_t level, uint8_t is_xor)
{
	const char *old;
	char any;
	uint32_t temp;

	if (ptr[0] != '(')
		return;

	/* skip beginning parenthesis */
	ptr++;
	old = ptr;
	any = 0;

	while (1) {
		if (ptr[0] == 0 || ptr[0] == '\n') {
			break;
		}
		if (ptr[0] == '^' || ptr[0] == ',' || ptr[0] == ')') {
			if (any) {
				any = 0;
				temp = var | mbin_parse32_abc(old, ptr);
				old = ptr + 1;
				while (1) {
					if (old[0] == 0 || old[0] == '\n') {
						/*
						 * final
						 */
						if (ptable == NULL) {
							printf("(");
							if (level < 32)
								mbin_print32_const(1UL << level);
							printf(")*(");
							mbin_print32_abc(temp & mask);
							printf(",)\n");
						} else {
							if (level < 32) {
								if (is_xor)
									ptable[temp & mask] ^= (1UL << level);
								else
									ptable[temp & mask] -= (1UL << level);
							}
						}
						break;
					}
					if (old[0] == '(') {
						/*
						 * recurse
						 */
						mbin_parse32_factor(old,
						    ptable, mask, temp, level, is_xor);
						break;
					}
					old++;
				}
			}
			if (ptr[0] == ')') {
				break;
			}
			if (ptr[0] == ',') {
				level++;
			}
			old = ++ptr;
		} else if (isspace(ptr[0]) || (ptr[0] == '0')) {
			/* skip whitespace and zeros */
			ptr++;
		} else {
			/* expression */
			any = 1;
			ptr++;
		}
	}
}
