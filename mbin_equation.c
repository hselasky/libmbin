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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysexits.h>
#include <err.h>

#include "math_bin.h"

struct mbin_eq_32 *
mbin_eq_alloc_32(uint32_t bits)
{
	struct mbin_eq_32 *ptr;
	size_t size = sizeof(struct mbin_eq_32) + ((bits + 7) / 8);

	ptr = malloc(size);
	if (ptr == NULL)
		errx(EX_SOFTWARE, "Out of memory");

	ptr->bitdata = (uint8_t *)(ptr + 1);

	return (ptr);
}

void
mbin_eq_free_32(mbin_eq_head_32_t *phead, struct mbin_eq_32 *ptr)
{
	if (ptr->entry.tqe_prev != NULL)
		TAILQ_REMOVE(phead, ptr, entry);
	free(ptr);
}

void
mbin_eq_free_head_32(mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 *ptr;

	while ((ptr = TAILQ_FIRST(phead)))
		mbin_eq_free_32(phead, ptr);
}

int
mbin_eq_solve_32(const uint32_t *func, const uint32_t size, mbin_eq_head_32_t *phead)
{
	uint32_t total = MBIN_EQ_FILTER_SIZE(size);
	struct mbin_eq_32 *ptr;
	struct mbin_eq_32 *other;
	struct mbin_eq_32 *next;
	uint32_t x, y, t, u, j;

	total = ((total + 7) / 8) * 8;

	TAILQ_INIT(phead);

	for (x = 0; x != size; x++) {
		for (y = x; (x + y) != (2 * size); y++) {
			ptr = mbin_eq_alloc_32(total);
			ptr->value = func[x + y];
			for (j = t = 0; t != size; t++) {
				for (u = t; u != size; u++, j++) {
					if (t == u) {
						if ((func[x] & t) == t && (func[y] & u) == u)
							MBIN_EQ_BIT_SET(ptr->bitdata, j);
					} else {
						if ((func[x] & t) == t && (func[y] & u) == u)
							MBIN_EQ_BIT_XOR(ptr->bitdata, j);
						if ((func[y] & t) == t && (func[x] & u) == u)
							MBIN_EQ_BIT_XOR(ptr->bitdata, j);
					}
				}
			}
			TAILQ_INSERT_TAIL(phead, ptr, entry);
		}
	}

repeat:
	/* solve equation set */
	TAILQ_FOREACH_SAFE(ptr, phead, entry, next) {

		if (ptr->flags != 0)
			continue;

		ptr->flags = 1;

		for (y = 0; y != total; y += 8) {
			if (ptr->bitdata[y / 8] == 0)
				continue;
			for (; y != total; y++) {
				if (MBIN_EQ_BIT_GET(ptr->bitdata, y))
					break;
			}
			break;
		}
		if (y == total) {
			if (ptr->value != 0)
				goto error;
			mbin_eq_free_32(phead, ptr);
			continue;
		}
		TAILQ_FOREACH(other, phead, entry) {
			if (ptr == other)
				continue;
			if (MBIN_EQ_BIT_GET(other->bitdata, y) == 0)
				continue;
			for (t = 0; t != total; t += 8)
				other->bitdata[t / 8] ^= ptr->bitdata[t / 8];
			other->value ^= ptr->value;
			other->flags = 0;
		}
	}

	/* sort solution */
	TAILQ_FOREACH_SAFE(ptr, phead, entry, next) {
		for (u = y = 0; y != total; y += 8)
			u += mbin_sumbits8(ptr->bitdata[y / 8]);

		if (u != 1) {
			if (u == 0) {
				if (ptr->value != 0)
					goto error;
				mbin_eq_free_32(phead, ptr);
				continue;
			}
			for (u = y = 0; y != total; y++) {
				u += (MBIN_EQ_BIT_GET(ptr->bitdata, y) != 0);
				if (u == 2) {
					/*
					 * More than one variable, set
					 * a variable to zero:
					 */
					TAILQ_FOREACH(ptr, phead, entry) {
						if (MBIN_EQ_BIT_GET(ptr->bitdata, y) == 0)
							continue;
						MBIN_EQ_BIT_CLR(ptr->bitdata, y);
						ptr->flags = 0;
					}
					goto repeat;
				}
			}
			goto error;
		}
	}
	return (0);

error:
	mbin_eq_free_head_32(phead);
	return (-1);
}

void
mbin_eq_print_32(mbin_eq_head_32_t *phead, const uint32_t size)
{
	struct mbin_eq_32 *ptr;
	uint32_t j, t, u, c;

	c = 0;
	TAILQ_FOREACH(ptr, phead, entry) {
		if (ptr->value == 0)
			continue;
		for (j = t = 0; t != size; t++) {
			for (u = t; u != size; u++, j++) {
				if (MBIN_EQ_BIT_GET(ptr->bitdata, j) == 0)
					continue;
				if (t == u) {
					mbin_print16_abc(t);
					printf(" & ");
					mbin_print16_abc(u);
				} else {
					printf("(");
					mbin_print16_abc(t);
					printf(" & ");
					mbin_print16_abc(u);
					printf(") ^ (");
					mbin_print16_abc(u);
					printf(" & ");
					mbin_print16_abc(t);
					printf(")");

				}
			}
		}
		printf(" = 0x%08x\n", ptr->value);
		c++;
	}
	printf("Count = %d\n", c);
}
