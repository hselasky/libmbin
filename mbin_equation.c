/*-
 * Copyright (c) 2014 Hans Petter Selasky. All rights reserved.
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
#include <string.h>
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

	memset(ptr, 0, size);

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
mbin_eq_simplify_32(const uint32_t total, mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 *ptr;
	struct mbin_eq_32 *other;
	struct mbin_eq_32 *next;
	uint32_t y;

	TAILQ_FOREACH_SAFE(ptr, phead, entry, next) {

		for (y = 0; y != total; y += 8) {
			if (ptr->bitdata[y / 8] != 0) {
				y += mbin_sumbits8(mbin_lsb8(ptr->bitdata[y / 8]) - 1);
				break;
			}
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
			for (y = 0; y != total; y += 8)
				other->bitdata[y / 8] ^= ptr->bitdata[y / 8];
			other->value ^= ptr->value;
		}
	}
	return (0);
error:
	return (-1);
}

int
mbin_eq_solve_32(mbin_eq_func_t *func_a, mbin_eq_func_t *func_b,
    mbin_eq_func_t *func_r, const uint32_t size, mbin_eq_head_32_t *phead, const uint8_t op)
{
	uint32_t total = ((MBIN_EQ_FILTER_SIZE(size) + 7) / 8) * 8;
	struct mbin_eq_32 *ptr;
	struct mbin_eq_32 *other;
	struct mbin_eq_32 *next;
	mbin_eq_head_32_t thead;
	uint32_t x, y, t, u, j;

	TAILQ_INIT(phead);

	TAILQ_INIT(&thead);

	for (x = 0; x != size; x++) {
		uint32_t fx;
		uint32_t fy;

		fx = func_b(x) & (size - 1);
		for (y = x; (x + y) != (2 * size); y++) {
			fy = func_a(y) & (size - 1);

			ptr = mbin_eq_alloc_32(total);

			switch (op) {
			case 0:
				ptr->value = func_r(x + y) & (size - 1);
				break;
			case 1:
				ptr->value = func_r(x * y) & (size - 1);
				break;
			case 2:
				ptr->value = func_r(x ^ y) & (size - 1);
				break;
			case 3:
				ptr->value = func_r(x & y) & (size - 1);
				break;
			case 4:
				ptr->value = func_r(x | y) & (size - 1);
				break;
			case 5:
				ptr->value = func_r(mbin_xor2_mul_64(x, y)) & (size - 1);
				break;
			default:
				break;
			}
			for (j = t = 0; t != size; t++) {
				for (u = 0; u != size; u++, j++) {
					if ((fx & t) == t && (fy & u) == u)
						MBIN_EQ_BIT_SET(ptr->bitdata, j);
				}
			}
			TAILQ_INSERT_TAIL(&thead, ptr, entry);
		}
	}

	/* solve equation set */
	if (mbin_eq_simplify_32(total, &thead) < 0)
		goto error;

	/* sort solution */
	TAILQ_FOREACH_SAFE(ptr, &thead, entry, next) {
		if (ptr->value == 0) {
			mbin_eq_free_32(&thead, ptr);
			continue;
		}
		for (y = 0; y != total; y += 8) {
			if (ptr->bitdata[y / 8] != 0) {
				y += mbin_sumbits8(mbin_lsb8(ptr->bitdata[y / 8]) - 1);
				break;
			}
		}
		if (y == total)
			goto error;

		other = mbin_eq_alloc_32(32);
		other->value = ptr->value;
		*(uint32_t *)other->bitdata = y;
		TAILQ_INSERT_TAIL(phead, other, entry);
	}
	mbin_eq_free_head_32(&thead);
	return (0);

error:
	mbin_eq_free_head_32(&thead);
	mbin_eq_free_head_32(phead);
	return (-1);
}

static int
mbin_eq_sort_compare(const void *pa, const void *pb)
{
	int32_t ret = (((struct mbin_eq_32 **)pa)[0][0].value -
	    ((struct mbin_eq_32 **)pb)[0][0].value);

	if (ret == 0) {
		ret = *(uint32_t *)(((struct mbin_eq_32 **)pa)[0][0].bitdata) -
		    *(uint32_t *)(((struct mbin_eq_32 **)pb)[0][0].bitdata);
	}
	return (ret < 0 ? -1 : (ret > 0 ? 1 : 0));
}

void
mbin_eq_sort_by_value_32(mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 *ptr;
	uint32_t c, x;

	c = 0;
	TAILQ_FOREACH(ptr, phead, entry)
	    c++;

	struct mbin_eq_32 *pptr[c];

	c = 0;
	TAILQ_FOREACH(ptr, phead, entry)
	    pptr[c++] = ptr;

	qsort(pptr, c, sizeof(void *), mbin_eq_sort_compare);

	TAILQ_INIT(phead);

	for (x = 0; x != c; x++)
		TAILQ_INSERT_TAIL(phead, pptr[x], entry);
}

void
mbin_eq_print_32(mbin_eq_head_32_t *phead, const uint32_t size)
{
	struct mbin_eq_32 *ptr;
	uint32_t t, u, c;

	c = 0;
	TAILQ_FOREACH(ptr, phead, entry) {
		t = *(uint32_t *)ptr->bitdata % size;
		u = *(uint32_t *)ptr->bitdata / size;
		mbin_print16_abc(t);
		printf(" & ");
		mbin_print16_abc(u);
		printf(" = 0x%08x; C=", ptr->value);
		mbin_print16_abc(t & u);
		if (t == u)
			printf(" EQ");
		printf("\n");
		c++;
	}
	printf("Count = %d\n", c);
}

void
mbin_eq_print_code_32(mbin_eq_head_32_t *phead, const uint32_t size, const char *name)
{
	struct mbin_eq_32 *ptr;
	uint32_t t, u, c;

	printf("uint32_t\n"
	    "%s(uint32_t a, uint32_t b)\n"
	    "{\n"
	    "\t" "uint32_t r = 0;\n", name);

	c = 0;
	TAILQ_FOREACH(ptr, phead, entry) {
		printf("\t" "if (");
		t = *(uint32_t *)ptr->bitdata % size;
		u = *(uint32_t *)ptr->bitdata / size;
		printf("(((a & 0x%x) == 0x%x) & "
		    "((b & 0x%x) == 0x%x))", t, t, u, u);
		printf(") r ^= 0x%08x;\n", ptr->value);
		c++;
	}
	printf("\t" "/* Count = %d */\n"
	    "\t" "return (r & 0x%x);\n"
	    "}\n", c, size - 1);
}

uint32_t
mbin_eq_func_32(mbin_eq_head_32_t *phead, const uint32_t size,
    const uint32_t a, const uint32_t b)
{
	struct mbin_eq_32 *ptr;
	uint32_t t, u, r;

	r = 0;
	TAILQ_FOREACH(ptr, phead, entry) {
		t = *(uint32_t *)ptr->bitdata % size;
		u = *(uint32_t *)ptr->bitdata / size;
		if (((a & t) == t) & ((b & u) == u))
			r ^= ptr->value;
	}
	return (r & (size - 1));
}
