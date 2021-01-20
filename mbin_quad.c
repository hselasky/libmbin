/*-
 * Copyright (c) 2011-2021 Hans Petter Selasky. All rights reserved.
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

#include "math_bin.h"

/*
 * Quad structures contain the integer representation for a number
 * which can be expanded like this:
 *
 * v[0] + v[1] * sqrt(3) + v[2] * sqrt(5) + v[3] * sqrt(3*5)
 */
void
mbin_mul_quad_double(const struct mbin_quad_double *pa,
		     const struct mbin_quad_double *pb,
		     struct mbin_quad_double *pc)
{
	const double temp[4] = {
		pa->v[0] * pb->v[0] +
		pa->v[1] * pb->v[1] +
		pa->v[2] * pb->v[2] +
		pa->v[0] * pb->v[0],

		pa->v[0] * pb->v[1] +
		pa->v[1] * pb->v[0] +
		pa->v[2] * pb->v[3] * 5 +
		pa->v[3] * pb->v[2] * 5,

		pa->v[0] * pb->v[2] +
		pa->v[2] * pb->v[0] +
		pa->v[1] * pb->v[3] * 3 +
		pa->v[3] * pb->v[1] * 3,

		pa->v[0] * pb->v[3] +
		pa->v[3] * pb->v[0] +
		pa->v[1] * pb->v[2] +
		pa->v[2] * pb->v[1],
	};

	memcpy(pc, temp, sizeof(temp));
}

void
mbin_add_quad_double(const struct mbin_quad_double *pa,
		     const struct mbin_quad_double *pb,
		     struct mbin_quad_double *pc)
{
	pc->v[0] = pa->v[0] + pb->v[0];
	pc->v[1] = pa->v[1] + pb->v[1];
	pc->v[2] = pa->v[2] + pb->v[2];
	pc->v[3] = pa->v[3] + pb->v[3];
}

void
mbin_sub_quad_double(const struct mbin_quad_double *pa,
		     const struct mbin_quad_double *pb,
		     struct mbin_quad_double *pc)
{
	pc->v[0] = pa->v[0] - pb->v[0];
	pc->v[1] = pa->v[1] - pb->v[1];
	pc->v[2] = pa->v[2] - pb->v[2];
	pc->v[3] = pa->v[3] - pb->v[3];
}

void
mbin_mul_complex_quad_double(const struct mbin_complex_quad_double *pa,
			     const struct mbin_complex_quad_double *pb,
			     struct mbin_complex_quad_double *pc)
{
	struct mbin_quad_double mul[4];

	mbin_mul_quad_double(&pa->x, &pb->x, mul + 0);
	mbin_mul_quad_double(&pa->y, &pb->y, mul + 1);

	mbin_mul_quad_double(&pa->x, &pb->y, mul + 2);
	mbin_mul_quad_double(&pa->y, &pb->x, mul + 3);

	mbin_sub_quad_double(mul + 0, mul + 1, &pc->x);
	mbin_add_quad_double(mul + 2, mul + 3, &pc->y);
}

void
mbin_add_complex_quad_double(const struct mbin_complex_quad_double *pa,
			     const struct mbin_complex_quad_double *pb,
			     struct mbin_complex_quad_double *pc)
{
	pc->x.v[0] = pa->x.v[0] + pb->x.v[0];
	pc->x.v[1] = pa->x.v[1] + pb->x.v[1];
	pc->x.v[2] = pa->x.v[2] + pb->x.v[2];
	pc->x.v[3] = pa->x.v[3] + pb->x.v[3];

	pc->y.v[0] = pa->y.v[0] + pb->y.v[0];
	pc->y.v[1] = pa->y.v[1] + pb->y.v[1];
	pc->y.v[2] = pa->y.v[2] + pb->y.v[2];
	pc->y.v[3] = pa->y.v[3] + pb->y.v[3];
}

void
mbin_sub_complex_quad_double(const struct mbin_complex_quad_double *pa,
		     const struct mbin_complex_quad_double *pb,
		     struct mbin_complex_quad_double *pc)
{
	pc->x.v[0] = pa->x.v[0] - pb->x.v[0];
	pc->x.v[1] = pa->x.v[1] - pb->x.v[1];
	pc->x.v[2] = pa->x.v[2] - pb->x.v[2];
	pc->x.v[3] = pa->x.v[3] - pb->x.v[3];

	pc->y.v[0] = pa->y.v[0] - pb->y.v[0];
	pc->y.v[1] = pa->y.v[1] - pb->y.v[1];
	pc->y.v[2] = pa->y.v[2] - pb->y.v[2];
	pc->y.v[3] = pa->y.v[3] - pb->y.v[3];
}

void
mbin_add_conj_complex_quad_double(const struct mbin_complex_quad_double *pa,
				  const struct mbin_complex_quad_double *pb,
				  struct mbin_complex_quad_double *pc)
{
	pc->x.v[0] = pa->x.v[0] + pb->y.v[0];
	pc->x.v[1] = pa->x.v[1] + pb->y.v[1];
	pc->x.v[2] = pa->x.v[2] + pb->y.v[2];
	pc->x.v[3] = pa->x.v[3] + pb->y.v[3];

	pc->y.v[0] = pa->y.v[0] - pb->x.v[0];
	pc->y.v[1] = pa->y.v[1] - pb->x.v[1];
	pc->y.v[2] = pa->y.v[2] - pb->x.v[2];
	pc->y.v[3] = pa->y.v[3] - pb->x.v[3];
}

void
mbin_sub_conj_complex_quad_double(const struct mbin_complex_quad_double *pa,
				  const struct mbin_complex_quad_double *pb,
				  struct mbin_complex_quad_double *pc)
{
	pc->x.v[0] = pa->x.v[0] - pb->y.v[0];
	pc->x.v[1] = pa->x.v[1] - pb->y.v[1];
	pc->x.v[2] = pa->x.v[2] - pb->y.v[2];
	pc->x.v[3] = pa->x.v[3] - pb->y.v[3];

	pc->y.v[0] = pa->y.v[0] + pb->x.v[0];
	pc->y.v[1] = pa->y.v[1] + pb->x.v[1];
	pc->y.v[2] = pa->y.v[2] + pb->x.v[2];
	pc->y.v[3] = pa->y.v[3] + pb->x.v[3];
}
