/*-
 * Copyright (c) 2011 Hans Petter Selasky. All rights reserved.
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

#include "math_bin.h"

struct mbin_complex_double
mbin_mul_complex_double(struct mbin_complex_double a,
    struct mbin_complex_double b)
{
	struct mbin_complex_double temp;

	temp.x = a.x * b.x - b.y * b.y;
	temp.y = a.x * b.y + a.y * b.x;

	return (temp);
}

struct mbin_complex_double
mbin_div_complex_double(struct mbin_complex_double a,
    struct mbin_complex_double b)
{
	struct mbin_complex_double temp;
	struct mbin_complex_double c;
	double div;

	c = b;
	c.y = -c.y;

	div = c.x * c.x + c.y * c.y;

	temp = mbin_mul_complex_double(a, c);
	temp.x /= div;
	temp.y /= div;

	return (temp);
}

struct mbin_complex_double
mbin_add_complex_double(struct mbin_complex_double a,
    struct mbin_complex_double b)
{
	struct mbin_complex_double temp;

	temp.x = a.x + b.x;
	temp.y = a.y + b.y;

	return (temp);
}

struct mbin_complex_double
mbin_sub_complex_double(struct mbin_complex_double a,
    struct mbin_complex_double b)
{
	struct mbin_complex_double temp;

	temp.x = a.x - b.x;
	temp.y = a.y - b.y;

	return (temp);
}

double
mbin_square_len_complex_double(struct mbin_complex_double t)
{
	return (t.x * t.x + t.y * t.y);
}
