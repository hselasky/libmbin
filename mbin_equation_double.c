/*-
 * Copyright (c) 2015 Hans Petter Selasky
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

struct mbin_eq_d64 *
mbin_eq_alloc_d64(uint32_t bits)
{
	struct mbin_eq_d64 *ptr;
	size_t size = sizeof(struct mbin_eq_d64) + (sizeof(double) * bits);

	ptr = malloc(size);
	if (ptr == NULL)
		errx(EX_SOFTWARE, "Out of memory");

	memset(ptr, 0, size);

	ptr->fdata = (double *)(ptr + 1);

	return (ptr);
}

void
mbin_eq_free_d64(mbin_eq_head_d64_t *phead, struct mbin_eq_d64 *ptr)
{
	if (ptr->entry.tqe_prev != NULL)
		TAILQ_REMOVE(phead, ptr, entry);
	free(ptr);
}

void
mbin_eq_free_head_d64(mbin_eq_head_d64_t *phead)
{
	struct mbin_eq_d64 *ptr;

	while ((ptr = TAILQ_FIRST(phead)))
		mbin_eq_free_d64(phead, ptr);
}

int
mbin_eq_simplify_d64(uint32_t total, mbin_eq_head_d64_t *phead, double zero)
{
	struct mbin_eq_d64 *ptr;
	struct mbin_eq_d64 *other;
	struct mbin_eq_d64 *next;
	double temp;
	uint32_t x;
	uint32_t y;

	TAILQ_FOREACH_SAFE(ptr, phead, entry, next) {

		for (y = 0; y != total; y++) {
			if (fabs(ptr->fdata[y]) > zero)
				break;
			ptr->fdata[y] = 0.0;
		}
		if (y == total) {
			if (fabs(ptr->value) > zero)
				goto error;
			mbin_eq_free_d64(phead, ptr);
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
mbin_eq_solve_d64(uint32_t total, mbin_eq_head_d64_t *phead, double zero)
{
	struct mbin_eq_d64 *ptr;
	uint32_t x;
	uint32_t y;

	if (mbin_eq_simplify_d64(total, phead, zero))
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
