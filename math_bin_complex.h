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

#ifndef _MATH_BIN_COMPLEX_H_
#define	_MATH_BIN_COMPLEX_H_

#include <stdint.h>

#include <sys/cdefs.h>

__BEGIN_DECLS

typedef struct mbin_complex_32 {
	int32_t	x;
	int32_t	y;
} c32_t;

static inline c32_t
c32_add(c32_t a, c32_t b)
{
	return (c32_t){
		a.x + b.x, a.y + b.y
	};
}

static inline c32_t
c32_sub(c32_t a, c32_t b)
{
	return (c32_t){
		a.x - b.x, a.y - b.y
	};
}

static inline c32_t
c32_mul(c32_t a, c32_t b)
{
	return (c32_t){
		a.x * b.x - a.y * b.y, a.x * b.y + b.x * a.y
	};
}

static inline c32_t
c32_exp(c32_t b, uint64_t exp)
{
	c32_t r = {1, 0};

	while (exp != 0) {
		if (exp & 1)
			r = c32_mul(r, b);
		b = c32_mul(b, b);
		exp /= 2;
	}
	return (r);
}

static inline c32_t
c32_mod(c32_t a, int32_t mod)
{
	a.x %= mod;
	a.y %= mod;
	a.x += mod;
	a.y += mod;
	a.x %= mod;
	a.y %= mod;
	return (a);
}

static inline c32_t
c32_mul_mod_9(c32_t a, c32_t b, int32_t mod)
{
	c32_t r = {9 * a.x * b.x - 8 * a.y * b.y, a.x * b.y + b.x * a.y};

	while (r.x % 3)
		r.x += mod;
	r.x /= 3;

	while (r.x % 3)
		r.x += mod;
	r.x /= 3;

	return (c32_mod(r, mod));
}

static inline c32_t
c32_exp_mod_9(c32_t b, uint64_t exp, int32_t mod)
{
	c32_t r = {1, 0};

	while (exp != 0) {
		if (exp & 1)
			r = c32_mul_mod_9(r, b, mod);
		b = c32_mul_mod_9(b, b);
		exp /= 2;
	}
	return (r);
}

typedef struct mbin_complex_double {
	double	x;
	double	y;
} cd_t;

static inline cd_t
cd_add(cd_t a, cd_t b)
{
	return (cd_t){
		a.x + b.x, a.y + b.y
	};
}

static inline cd_t
cd_sub(cd_t a, cd_t b)
{
	return (cd_t){
		a.x - b.x, a.y - b.y
	};
}

static inline cd_t
cd_mul(cd_t a, cd_t b)
{
	return (cd_t){
		a.x * b.x - a.y * b.y, a.x * b.y + b.x * a.y
	};
}

static inline cd_t
cd_div(cd_t a, cd_t b)
{
	double div = b.x * b.x + b.y * b.y;

	return (cd_t){
		((a.x * b.x) + (a.y * b.y)) / div,
		    ((a.y * b.x) - (a.x * b.y)) / div
	};
}

static inline cd_t
cd_exp(cd_t b, uint64_t exp)
{
	cd_t r = {1, 0};

	while (exp != 0) {
		if (exp & 1)
			r = cd_mul(r, b);
		b = cd_mul(b, b);
		exp /= 2;
	}
	return (r);
}

static inline cd_t
cd_mul_9(cd_t a, cd_t b)
{
	return (cd_t){
		a.x * b.x - (8.0 / 9.0) * a.y * b.y, a.x * b.y + b.x * a.y
	};
}

static inline cd_t
cd_exp_9(cd_t b, uint64_t exp)
{
	cd_t r = {1, 0};

	while (exp != 0) {
		if (exp & 1)
			r = cd_mul_9(r, b);
		b = cd_mul_9(b, b);
		exp /= 2;
	}
	return (r);
}

__END_DECLS

#endif					/* _MATH_BIN_COMPLEX_H_ */
