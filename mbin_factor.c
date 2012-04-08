/*-
 * Copyright (c) 2012 Hans Petter Selasky. All rights reserved.
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "math_bin.h"

uint32_t
mbin_xor_factor8_var2mask(int8_t var)
{
	if (var < 0)
		return (1U << (-var - 1));
	else if (var > 0)
		return (1U << (var - 1));
	else
		return (0);
}

char
mbin_xor_factor8_var2char(int8_t var)
{
	if (var < 0) {
		return ('A' + (-var - 1));
	} else if (var > 0) {
		return ('a' + var - 1);
	} else {
		return ('1');
	}
}

static struct mbin_xor_factor_leaf *
mbin_xor_factor8_leaf_new(void)
{
	struct mbin_xor_factor_leaf *ptr;

	ptr = malloc(sizeof(struct mbin_xor_factor_leaf));
	if (ptr != NULL) {
		memset(ptr, 0, sizeof(*ptr));
		TAILQ_INIT(&ptr->children);
	}
	return (ptr);
}

void
mbin_xor_factor8_leaf_free(struct mbin_xor_factor_leaf *ptr)
{
	struct mbin_xor_factor_leaf *child;

	if (ptr == NULL)
		return;

	while ((child = TAILQ_FIRST(&ptr->children)) != NULL) {
		TAILQ_REMOVE(&ptr->children, child, entry);
		mbin_xor_factor8_leaf_free(child);
	}
	free(ptr);
}

static void
mbin_xor_factor8_leaf_insert(struct mbin_xor_factor_leaf *parent,
    struct mbin_xor_factor_leaf *child)
{
	TAILQ_INSERT_TAIL(&parent->children, child, entry);
	parent->nchildren++;
}

static uint32_t
mbin_xor_factor8_move_expr(uint8_t *src, uint8_t *dst, uint32_t mask,
    uint32_t xm, int8_t var, uint8_t how)
{
	uint32_t x;
	uint32_t count = 0;

	mask &= ~xm;

	x = 0;

	do {
		if (src[(x & mask) | xm]) {
			if (src[(x & mask)] && (how & 1)) {
				if (var < 1) {
					src[(x & mask) | xm] = 0;
					src[(x & mask)] = 0;
					dst[(x & mask)] = 1;
					count++;
				}
			} else {
				if (var > 0) {
					src[(x & mask) | xm] = 0;
					dst[(x & mask)] = 1;
					count++;
				}
			}
		}
		x |= ~mask;
		x++;

	} while (x & mask);

	return (count);
}

static void
mbin_xor_factor8_build_hist(const uint8_t *src, uint32_t *dst,
    uint32_t mask, uint8_t how)
{
	uint32_t x;
	uint32_t m;
	uint32_t n;

	x = 0;
	do {
		if (src[x & mask])
			dst[0]++;

		x |= ~mask;
		x++;

	} while (x & mask);

	for (n = 0; n != 32; n++) {

		if (!(mask & (1U << n)))
			continue;

		m = 1U << n;

		mask &= ~m;

		x = 0;
		do {
			if (src[(x & mask) | m]) {
				if (src[(x & mask)] && (how & 1)) {
					dst[1 + 32 + n] += 2;
				} else {
					dst[1 + n] += 1;
				}
			}
			x |= ~mask;
			x++;

		} while (x & mask);

		mask |= m;
	}
}

static void
mbin_xor_factor8_build_factor(uint8_t *input, uint32_t mask,
    struct mbin_xor_factor_leaf *parent, uint8_t *pc, uint8_t how)
{
	struct mbin_xor_factor_leaf *child;
	struct mbin_xor_factor_leaf *grandchild;
	uint32_t max = 1 << mbin_sumbits32(mask);
	uint32_t hist[64 + 1];
	uint8_t conv[32];
	uint8_t *factor = malloc(max);
	uint8_t *src = malloc(max);
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t count;
	int8_t var;

	for (x = y = 0; x != 32; x++) {
		if (mask & (1 << x)) {
			conv[y] = pc[x];
			y++;
		}
	}
	for (; y != 32; y++)
		conv[y] = 255;

	x = 0;
	y = 0;
	do {
		src[y] = input[x & mask];
		y++;

		x |= ~mask;
		x++;
	} while (x & mask);

	mask = max - 1;

	if (src[0] != 0) {
		child = mbin_xor_factor8_leaf_new();
		child->var = 0;
		child->desc = mbin_xor_factor8_var2char(child->var);
		mbin_xor_factor8_leaf_insert(parent, child);
		src[0] = 0;
	}
	while (1) {

		memset(hist, 0, sizeof(hist));

		mbin_xor_factor8_build_hist(src, hist, mask, how);

		if (hist[0] == 0)
			break;

		/* find most frequently occurring variable */

		if (how & 2) {
			for (x = y = 1; x != (32 + 1); x++) {
				if (hist[x]) {
					y = x;
				} else if (hist[x + 32]) {
					y = x + 32;
				}
			}
		} else {
			for (x = y = 1; x != (64 + 1); x++) {
				if ((hist[y] < hist[x]) ||
				    ((hist[y] == hist[x]) &&
				    ((y - 1) % 32) < ((x - 1) % 32))) {
					y = x;
				}
			}
		}

		if (y >= (32 + 1))
			var = 32 - y;
		else
			var = y;

		hist[y] = 0;

		z = mbin_xor_factor8_var2mask(var);

		memset(factor, 0, max);

		count = mbin_xor_factor8_move_expr(src, factor,
		    mask, z, var, how);

		if (var < 0 && (count & 1))
			src[z] ^= 1;

		/* remove single bits */

		if (src[z] != 0) {
			x = (y - 1) % 32;
			child = mbin_xor_factor8_leaf_new();
			child->var = conv[x] + 1;
			child->desc = mbin_xor_factor8_var2char(child->var);
			mbin_xor_factor8_leaf_insert(parent, child);
			src[z] = 0;
		}
		/* optimise */

		for (x = 0; x != 32; x++) {
			if (hist[x + 1] == 0 && hist[x + 1 + 32] == 0) {
				mask &= ~(1U << x);
			}
		}

		/* collect factor */

		child = mbin_xor_factor8_leaf_new();
		if (var == 0) {
			child->var = 0;
		} else if (var < 0) {
			child->var = -conv[-var - 1] - 1;
		} else {
			child->var = conv[var - 1] + 1;
		}
		child->desc = mbin_xor_factor8_var2char(child->var);

		mbin_xor_factor8_build_factor(factor, mask & ~z,
		    child, conv, how);

		/* check if anything was left */

		if (child->nchildren != 0) {
			mbin_xor_factor8_leaf_insert(parent, child);
			if (child->nchildren == 1 &&
			    TAILQ_FIRST(&child->children)->var == 0) {
				grandchild = TAILQ_FIRST(&child->children);
				TAILQ_REMOVE(&child->children, grandchild, entry);
				mbin_xor_factor8_leaf_free(grandchild);
			}
		} else {
			mbin_xor_factor8_leaf_free(child);
		}
	}

	free(factor);
	free(src);
}

struct mbin_xor_factor_leaf *
mbin_xor_factor8_build_tree(uint8_t *src, uint8_t lmax, uint8_t how)
{
	struct mbin_xor_factor_leaf *ptr = mbin_xor_factor8_leaf_new();
	uint32_t mask = (1 << lmax) - 1;
	uint8_t conv[32];
	uint8_t x;

	for (x = 0; x != 32; x++)
		conv[x] = x;

	mbin_xor_factor8_build_factor(src, mask, ptr,
	    conv, how);

	return (ptr);
}

void
mbin_xor_factor8_print_tree(struct mbin_xor_factor_leaf *ptr, uint8_t level)
{
	struct mbin_xor_factor_leaf *child;
	uint8_t first;

	if (level != 0) {
		if (TAILQ_FIRST(&ptr->children))
			printf("(");
		printf("%c", ptr->desc);
		first = 1;
	} else {
		first = 2;
	}

	TAILQ_FOREACH(child, &ptr->children, entry) {
		if (first == 1) {
			printf("&");
			if (TAILQ_NEXT(child, entry)) {
				printf("(");
				first = 0;
			}
		}
		mbin_xor_factor8_print_tree(child, level + 1);
		if (TAILQ_NEXT(child, entry))
			printf("^");
	}

	if (first == 0)
		printf(")");

	if (level != 0) {
		if (TAILQ_FIRST(&ptr->children))
			printf(")");
	}
}

void
mbin_xor_factor8_compress_tree(struct mbin_xor_factor_leaf *parent,
    uint32_t *pout, uint32_t *pbit, uint8_t bits, uint8_t how, uint8_t val)
{
	struct mbin_xor_factor_leaf *child = NULL;
	int m;
	int has_var;
	int var;

	if (!(how & 2))
		return;

	for (m = 0; m != bits; m++) {

		if (parent) {
			TAILQ_FOREACH(child, &parent->children, entry) {
				var = child->var;
				if ((child->nchildren == 0) &&
				    ((var == (m + 1)) || (-var == (m + 1))))
					break;
			}
		}
		if (child) {
			has_var = 1;
		} else {
			has_var = 0;
		}
		if (parent) {
			TAILQ_FOREACH(child, &parent->children, entry) {
				var = child->var;
				if ((child->nchildren != 0) && (var == (m + 1)))
					break;
			}
		}
		if (child || has_var) {
			mbin_put_bits32(pout, pbit, 1, 1);
			if (m) {
				mbin_xor_factor8_compress_tree(child, pout,
				    pbit, m, how, has_var);
			}
		} else {
			mbin_put_bits32(pout, pbit, 1, 0);
		}
		if (how & 1) {
			if (parent) {
				TAILQ_FOREACH(child, &parent->children, entry) {
					var = child->var;
					if ((child->nchildren != 0) && (-var == (m + 1)))
						break;
				}
				if (child) {
					mbin_put_bits32(pout, pbit, 1, 1);
					if (m) {
						mbin_xor_factor8_compress_tree(child, pout,
						    pbit, m, how, 0);
					}
				} else {
					mbin_put_bits32(pout, pbit, 1, 0);
				}
			}
		}
	}

	if (parent) {
		TAILQ_FOREACH(child, &parent->children, entry) {
			var = child->var;
			if ((child->nchildren == 0) && (var <= 0))
				val ^= 1;
		}
	}
	mbin_put_bits32(pout, pbit, 1, val);
}
