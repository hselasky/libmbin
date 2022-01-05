/*-
 * Copyright (c) 2021-2022 Hans Petter Selasky. All rights reserved.
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

#include "math_bin.h"

/*
 * This function computes the length of the following Lucas sequence
 * under modulus:
 *
 * a(n) = (2 / 3) * a(n-1) - a(n-2)
 *
 * Initial conditions are { 1, 1 / 3 ... }
 *
 * Without modulus the frequency of the resulting sine wave is given
 * by: 2 * pi / acos(1 / 3) ~ 5.1043 Hz
 *
 * The modular value must not be divisible by 3.
 */
uint32_t
mbin_lucas_step_count_mod_32(uint32_t mod)
{
	uint32_t a[3];
	uint32_t o[2];
	uint32_t r = 0;

	if ((mod % 3) == 0)
		return (0);

	a[0] = 1;
	a[1] = 1;
	while (a[1] % 3)
		a[1] += mod;
	a[1] /= 3;

	o[0] = a[0];
	o[1] = a[1];

	while (1) {
		a[2] = (3 * mod + 2 * a[1] - 3 * a[0]) % mod;
		while (a[2] % 3)
			a[2] += mod;
		a[2] /= 3;
		a[0] = a[1];
		a[1] = a[2];
		r++;
		if (a[0] == o[0] && a[1] == o[1])
			break;
	}
	return (r);
}

/*
 * This function computes the length of one circle step.
 */
uint32_t
mbin_lucas_step_length_squared_mod_32(uint32_t mod)
{
	uint32_t val = mod;

	if ((mod % 3) == 0)
		return (0);

	while (val % 3)
		val++;

	if ((mod % 3) == 1)
		val = ((2 * val) / 3);
	else
		val = (val / 3) + 1;

	return (val);
}

/*
 * This function computes PI squared under modulus.
 */
uint32_t
mbin_lucas_pi_squared_mod_32(uint32_t mod)
{
	uint32_t steps = mbin_lucas_step_count_mod_32(mod);
	uint32_t len = mbin_lucas_step_length_squared_mod_32(mod);

	return (len * steps * steps) % mod;
}
