/*-
 * Copyright (c) 2021 Hans Petter Selasky. All rights reserved.
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
 * This file implements helper functions for modular equation sets.
 */

#include <stdio.h>
#include <stdint.h>

#include "math_bin.h"

static bool
mbin_eq_mod_is_valid(int32_t *pmod, size_t num)
{
	for (size_t x = 0; x != num; x++) {
		if ((pmod[num] % pmod[x]) == 0)
			return (false);
	}
	return (true);
}

void
mbin_eq_mod_gen_32(int32_t *pmod, size_t num, uint64_t *pmax, uint64_t *psum)
{
	uint64_t max;
	uint64_t sum;

	if (pmax == NULL)
		pmax = &max;
	if (psum == NULL)
		psum = &sum;

	if (num == 0)
		goto done;

	/* select first prime number */
	pmod[0] = 2;

	for (size_t x = 1; x != num; x++) {
		pmod[x] = pmod[x - 1];

		/* find special prime number */
		do {
			pmod[x]++;
		} while (mbin_eq_mod_is_valid(pmod, x) == false);
	}
done:
	max = 1;
	sum = 0;

	for (size_t x = 0; x != num; x++)
		max *= pmod[x];
	for (size_t x = 0; x != num; x++)
		sum += max / pmod[x];

	*pmax = max;
	*psum = sum % max;
}

void
mbin_eq_mod_solve_32(void *ptr, int32_t *pmod, size_t max)
{
	typedef int32_t table_t[max][2 * max];
	table_t *table = ptr;
	int32_t temp;

	for (size_t x = 0; x != max; x++) {
		for (size_t y = 0; y != max; y++) {
			if (table[0][x][y] == 0)
				continue;
			temp = table[0][x][y];

			/* get absolute modulus */
			if (temp < 0)
				temp += pmod[y];

			/* compute multiplicative inverse */
			temp = mbin_power_mod_32(temp, pmod[y] - 2, pmod[y]);

			if (temp != 1) {
				/* normalize equation */
				for (size_t z = 0; z != 2 * max; z++)
					table[0][x][z] = (table[0][x][z] * temp) % pmod[z % max];
			}

			for (size_t z = 0; z != max; z++) {
				if (z == x || table[0][z][y] == 0)
					continue;

				temp = table[0][z][y];
				for (size_t t = 0; t != 2 * max; t++) {
					table[0][z][t] -= table[0][x][t] * temp;
					table[0][z][t] %= pmod[t % max];
				}
			}
			break;
		}
	}
}

void
mbin_eq_mod_flip_32(void *ptr, size_t max)
{
	typedef int32_t table_t[max][2 * max];
	table_t *table = ptr;
	int32_t temp;

	for (size_t x = 0; x != max; x++) {
		for (size_t y = 0; y != max; y++) {
			temp = table[0][x][y];
			table[0][x][y] = table[0][x][y + max];
			table[0][x][y + max] = temp;
		}
	}
}

void
mbin_eq_mod_print_32(const void *ptr, int32_t *pmod, size_t max, bool sign)
{
	typedef int32_t table_t[max][2 * max];
	const table_t *table = ptr;

	for (size_t x = 0; x != max; x++) {
		for (size_t y = 0; y != 2 * max; y++) {
			int32_t v = table[0][x][y];
			if (v < 0)
				v += pmod[y % max];
			if (sign && v > (pmod[y % max] / 2))
				v -= pmod[y % max];
			if (y == max)
				printf(" || ");
			printf("%d ", v);
		}
		printf("\n");
	}
}

void
mbin_eq_mod_decompose_32(int32_t *ptr, int32_t *pmod, size_t max)
{
	for (size_t x = 0; x != max; x++) {
		for (size_t y = x + 1; y != max; y++) {
			ptr[y] -= ptr[x];
			ptr[y] %= pmod[y];
			ptr[y] *= (int32_t)mbin_power_mod_32(
			    pmod[x], pmod[y] - 2, pmod[y]);
			ptr[y] %= pmod[y];
		}
	}
}

void
mbin_eq_mod_compose_32(int32_t *ptr, int32_t *pmod, size_t max)
{
	for (size_t x = 0; x != max; x++) {
		for (size_t y = x + 1; y != max; y++) {
			ptr[y] *= pmod[x];
			ptr[y] += ptr[x];
			ptr[y] %= pmod[y];
		}
	}
}
