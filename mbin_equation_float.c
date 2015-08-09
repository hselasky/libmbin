/*-
 * Copyright (c) 2015 Hans Petter Selasky. All rights reserved.
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
#include <math.h>

#include "math_bin.h"

struct mbin_eq_f32 *
mbin_eq_alloc_f32(uint32_t bits)
{
	struct mbin_eq_f32 *ptr;
	size_t size = sizeof(struct mbin_eq_f32) + (sizeof(float) * bits);

	ptr = malloc(size);
	if (ptr == NULL)
		errx(EX_SOFTWARE, "Out of memory");

	memset(ptr, 0, size);

	ptr->fdata = (float *)(ptr + 1);

	return (ptr);
}

void
mbin_eq_free_f32(mbin_eq_head_f32_t *phead, struct mbin_eq_f32 *ptr)
{
	if (ptr->entry.tqe_prev != NULL)
		TAILQ_REMOVE(phead, ptr, entry);
	free(ptr);
}

void
mbin_eq_free_head_f32(mbin_eq_head_f32_t *phead)
{
	struct mbin_eq_f32 *ptr;

	while ((ptr = TAILQ_FIRST(phead)))
		mbin_eq_free_f32(phead, ptr);
}

int
mbin_eq_simplify_f32(uint32_t total, mbin_eq_head_f32_t *phead, float zero)
{
	struct mbin_eq_f32 *ptr;
	struct mbin_eq_f32 *other;
	struct mbin_eq_f32 *next;
	float temp;
	uint32_t x;
	uint32_t y;

	TAILQ_FOREACH_SAFE(ptr, phead, entry, next) {

		for (y = 0; y != total; y++) {
			if (ptr->fdata[y] != 0.0)
				break;
		}
		if (y == total) {
			if (fabs(ptr->value) > zero)
				goto error;
			mbin_eq_free_f32(phead, ptr);
			continue;
		}
		temp = ptr->fdata[y];
		for (x = 0; x != total; x++)
			ptr->fdata[x] /= temp;
		ptr->value /= temp;
		ptr->fdata[y] = 1.0;

		TAILQ_FOREACH(other, phead, entry) {
			if (ptr == other)
				continue;
			if (other->fdata[y] == 0.0)
				continue;
			temp = other->fdata[y];
			for (x = 0; x != total; x++)
				other->fdata[x] -= temp * ptr->fdata[x];
			other->value -= temp * ptr->value;
			other->fdata[y] = 0.0;
		}
	}
	return (0);
error:
	return (-1);
}

int
mbin_eq_solve_f32(uint32_t total, mbin_eq_head_f32_t *phead, float zero)
{
	struct mbin_eq_f32 *ptr;
	uint32_t x;
	uint32_t y;

	if (mbin_eq_simplify_f32(total, phead, zero))
		return (-1);

	TAILQ_FOREACH(ptr, phead, entry) {
		for (x = y = 0; x != total; x++) {
			if (ptr->fdata[x] == 0.0)
				continue;
			if (++y > 1)
				ptr->fdata[x] = 0.0;
		}
	}
	return (0);
}

int
mbin_eq_solve_table_f32(const uint32_t *xtable,
    const float *ytable, uint32_t max, uint32_t ltotal,
    int32_t lorder, mbin_eq_head_f32_t *phead,
    mbin_eq_table_func_t *func, float zero)
{
	struct mbin_eq_f32 *ptr;
	struct mbin_eq_f32 *tmp;
	mbin_eq_head_f32_t rhead;
	uint32_t *bitmap;
	uint32_t *array;
	uint32_t total;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint8_t higher;

	if (func == NULL)
		func = &mbin_eq_solve_table_func_and_32;

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
		ptr = mbin_eq_alloc_f32(total);
		for (y = 0; y != total; y++) {
			if (func(xtable[x], bitmap[y]))
				ptr->fdata[y] = 1.0;
		}
		ptr->value = ytable[x];
		TAILQ_INSERT_TAIL(phead, ptr, entry);
	}

	/* solve equation */
	if (mbin_eq_solve_f32(total, phead, zero)) {
		mbin_eq_free_head_f32(phead);
		return (-1);
	}
	TAILQ_FOREACH(ptr, phead, entry) {
		if (fabs(ptr->value) <= zero)
			continue;
		for (x = 0; x != total; x++) {
			if (ptr->fdata[x] != 0.0)
				break;
		}
		if (x != total) {
			tmp = mbin_eq_alloc_f32(1);
			tmp->value = ptr->value;
			*(uint32_t *)tmp->fdata = bitmap[x];
			TAILQ_INSERT_TAIL(&rhead, tmp, entry);
		}
	}
	mbin_eq_free_head_f32(phead);
	TAILQ_CONCAT(phead, &rhead, entry);
	return (0);
}

static int
mbin_eq_sort_compare_f32(const void *pa, const void *pb)
{
	float fa = ((struct mbin_eq_f32 **)pa)[0][0].value;
	float fb = ((struct mbin_eq_f32 **)pb)[0][0].value;
	uint32_t ia = *(uint32_t *)(((struct mbin_eq_f32 **)pa)[0][0].fdata);
	uint32_t ib = *(uint32_t *)(((struct mbin_eq_f32 **)pb)[0][0].fdata);

	if (fabs(fa) > fabs(fb))
		return (1);
	else if (fabs(fa) < fabs(fb))
		return (-1);
	if (ia > ib)
		return (1);
	else if (ia < ib)
		return (-1);
	return (0);
}

void
mbin_eq_sort_f32(mbin_eq_head_f32_t *phead)
{
	struct mbin_eq_f32 **array;
	struct mbin_eq_f32 *ptr;
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

	qsort(array, y, sizeof(array[0]), mbin_eq_sort_compare_f32);

	TAILQ_INIT(phead);

	while (y--)
		TAILQ_INSERT_HEAD(phead, array[y], entry);

	free(array);
}

void
mbin_eq_print_f32(mbin_eq_head_f32_t *phead)
{
	struct mbin_eq_f32 *ptr;
	uint32_t y = 0;

	TAILQ_FOREACH(ptr, phead, entry) {
		if (ptr->value == 0.0)
			continue;
		printf("%f*", ptr->value);
		mbin_print32_abc(*(uint32_t *)ptr->fdata);
		printf(" +\n");
		y++;
	}
	printf("Count = %d\n", (int)y);
}
