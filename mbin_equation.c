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
	size_t size = sizeof(struct mbin_eq_32) + (2 * ((bits + 15) / 16));

	ptr = malloc(size);
	if (ptr == NULL)
		errx(EX_SOFTWARE, "Out of memory");

	memset(ptr, 0, size);

	ptr->bitdata = (uint16_t *)(ptr + 1);

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
mbin_eq_simplify_32(uint32_t total, mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 *ptr;
	struct mbin_eq_32 *other;
	struct mbin_eq_32 *next;
	uint32_t x;
	uint32_t y;

	/* round up */
	total += (-total) & 15;

	TAILQ_FOREACH_SAFE(ptr, phead, entry, next) {

		for (y = 0; y != total; y++) {
			if (ptr->bitdata[y / 16] == 0) {
				y |= 7;
				continue;
			}
			if (MBIN_EQ_BIT_GET(ptr->bitdata, y) != 0)
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
			for (x = 0; x != total; x += 16)
				other->bitdata[x / 16] ^= ptr->bitdata[x / 16];
			other->value ^= ptr->value;
		}
	}
	return (0);
error:
	return (-1);
}

int
mbin_eq_solve_32(uint32_t total, mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 *ptr;
	uint32_t x;
	uint32_t y;

	/* round up */
	total += (-total) & 15;

	if (mbin_eq_simplify_32(total, phead))
		return (-1);

	TAILQ_FOREACH(ptr, phead, entry) {
		for (x = y = 0; x != total; x++) {
			if (ptr->bitdata[x / 16] == 0) {
				x |= 15;
				continue;
			}
			if (MBIN_EQ_BIT_GET(ptr->bitdata, x) == 0)
				continue;
			if (++y > 1)
				MBIN_EQ_BIT_CLR(ptr->bitdata, x);
		}
	}
	return (0);
}

int
mbin_eq_solve_func_32(mbin_eq_func_t *func_a, mbin_eq_func_t *func_b,
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
		for (y = 0; y != total; y += 16) {
			if (ptr->bitdata[y / 16] != 0) {
				y += mbin_sumbits16(mbin_lsb16(ptr->bitdata[y / 16]) - 1);
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

int
mbin_eq_solve_table_32(const uint32_t *xtable,
    const uint32_t *ytable, uint32_t max, uint32_t ltotal,
    int32_t lorder, uint32_t valmask, mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 *ptr;
	struct mbin_eq_32 *tmp;
	mbin_eq_head_32_t rhead;
	uint32_t *bitmap;
	uint32_t *array;
	uint32_t total;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint8_t higher;

	TAILQ_INIT(&rhead);
	TAILQ_INIT(phead);

	if (lorder < 0) {
		higher = 1;
		lorder = -lorder;
	} else {
		higher = 0;
	}

	if (lorder > ltotal)
		lorder = ltotal;

	/* allocate offset array */
	x = (ltotal + 1) * sizeof(array[0]);
	array = alloca(x);
	memset(array, 255, x);

	/* compute number of bits needed */
	for (total = x = 0; x <= lorder; x++) {
		if (array[x] == -1U) {
			array[x] = total;
			total += mbin_coeff_32(ltotal, x);
		}
		if (array[ltotal - x] == -1U && higher != 0) {
			array[ltotal - x] = total;
			total += mbin_coeff_32(ltotal, ltotal - x);
		}
	}

	bitmap = alloca(total * sizeof(bitmap[0]));

	/* build variable map */
	x = 1 << ltotal;
	for (y = 0; y != x; y++) {
		z = mbin_sumbits32(y);
		if (array[z] == -1U)
			continue;
		bitmap[array[z]++] = y;
	}

	/* build equation */
	for (x = 0; x != max; x++) {
		ptr = mbin_eq_alloc_32(total);
		for (y = 0; y != total; y++) {
			if ((xtable[x] & bitmap[y]) == bitmap[y])
				MBIN_EQ_BIT_SET(ptr->bitdata, y);
		}
		ptr->value = (ytable[x] & valmask) ? 1 : 0;
		TAILQ_INSERT_TAIL(phead, ptr, entry);
	}

	/* solve equation */
	if (mbin_eq_solve_32(total, phead)) {
		mbin_eq_free_head_32(phead);
		return (-1);
	}
	TAILQ_FOREACH(ptr, phead, entry) {
		if (ptr->value == 0)
			continue;
		for (x = 0; x != total; x += 16) {
			if (ptr->bitdata[x / 16] == 0)
				continue;
			for (; x != total; x++) {
				if (MBIN_EQ_BIT_GET(ptr->bitdata, x))
					break;
			}
			tmp = mbin_eq_alloc_32(32);
			tmp->value = 1;
			*(uint32_t *)tmp->bitdata = bitmap[x];
			TAILQ_INSERT_TAIL(&rhead, tmp, entry);
			break;
		}
	}
	mbin_eq_free_head_32(phead);
	TAILQ_CONCAT(phead, &rhead, entry);
	return (0);
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
mbin_eq_print_func_32(mbin_eq_head_32_t *phead, const uint32_t size)
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

static int
mbin_eq_sort_compare_32(const void *_pa, const void *_pb)
{
	struct mbin_eq_32 *pa = *(void **)_pa;
	struct mbin_eq_32 *pb = *(void **)_pb;
	int32_t diff = *(uint32_t *)pa->bitdata - *(uint32_t *)pb->bitdata;

	if (diff < 0)
		return (-1);
	else if (diff > 0)
		return (1);
	else
		return (0);
}

void
mbin_eq_sort_32(mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 **array;
	struct mbin_eq_32 *ptr;
	uint32_t y;

	y = 0;
	TAILQ_FOREACH(ptr, phead, entry)
	    y++;

	if (y == 0)
		return;

	array = malloc(y * sizeof(array[0]));
	if (array == NULL)
		return;

	y = 0;
	TAILQ_FOREACH(ptr, phead, entry)
	    array[y++] = ptr;

	qsort(array, y, sizeof(array[0]), mbin_eq_sort_compare_32);

	TAILQ_INIT(phead);

	while (y--)
		TAILQ_INSERT_HEAD(phead, array[y], entry);

	free(array);
}

void
mbin_eq_print_32(mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 *ptr;
	uint32_t y = 0;

	TAILQ_FOREACH(ptr, phead, entry) {
		if (ptr->value == 0)
			continue;
		mbin_print32_abc(*(uint32_t *)ptr->bitdata);
		printf(" ^\n");
		y++;
	}
	printf("Count = %d\n", (int)y);
}

static void
mbin_eq_print_factored_sub_32(struct mbin_eq_32 **array, uint32_t max, uint32_t mask)
{
	uint32_t stats[32];
	struct mbin_eq_32 *temp[max];
	uint32_t value;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint8_t any = 0;

	while (1) {
		memset(stats, 0, sizeof(stats));

		for (x = 0; x != max; x++) {
			if (array[x] == NULL)
				continue;
			value = *(uint32_t *)array[x]->bitdata;
			for (z = 0; z != 32; z++) {
				if ((mask >> z) & 1)
					continue;
				if ((value >> z) & 1)
					stats[z]++;
			}
		}
		for (x = z = 0; z != 32; z++) {
			if (stats[z] > stats[x])
				x = z;
		}
		if (stats[x] == 0) {
			if (any)
				printf(")");
			return;
		}
		if (!any) {
			printf("(");
			any = 1;
		} else {
			printf("^");
		}
		for (z = y = 0; y != max; y++) {
			if (array[y] == NULL)
				continue;
			value = *(uint32_t *)array[y]->bitdata;
			if ((value >> x) & 1) {
				temp[z++] = array[y];
				array[y] = NULL;
			}
		}
		mbin_print32_abc(1 << x);
		mbin_eq_print_factored_sub_32(temp, z, mask | (1 << x));
	}
}

void
mbin_eq_print_factored_32(mbin_eq_head_32_t *phead)
{
	struct mbin_eq_32 *ptr;
	struct mbin_eq_32 **array;
	uint32_t y = 0;

	TAILQ_FOREACH(ptr, phead, entry) {
		if (ptr->value == 0)
			continue;
		y++;
	}

	if (y == 0)
		return;

	array = alloca(sizeof(array[0]) * y);

	y = 0;
	TAILQ_FOREACH(ptr, phead, entry) {
		if (ptr->value == 0)
			continue;
		array[y++] = ptr;
	}

	mbin_eq_print_factored_sub_32(array, y, 0);
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
