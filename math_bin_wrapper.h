/*-
 * Copyright (c) 2013 Hans Petter Selasky
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

#ifndef _MATH_BIN_WRAPPER_H_
#define	_MATH_BIN_WRAPPER_H_

#include <math_bin.h>

class X2_64 {
public:
	uint64_t value;

	X2_64(void) {
		value = 0;
	}
	X2_64(const X2_64 &other) {
		value = other.value;
	}
	X2_64(const uint64_t &other) {
		value = other;
	}
	~X2_64(void) {

	}

	int operator== (const X2_64 &other) {
		return (value == other.value);
	}

	int operator>= (const X2_64 &other) {
		return (value >= other.value);
	}

	int operator<= (const X2_64 &other) {
		return (value <= other.value);
	}

	int operator> (const X2_64 &other) {
		return (value > other.value);
	}

	int operator< (const X2_64 &other) {
		return (value < other.value);
	}

	void operator= (const X2_64 &other) {
		if (&other == this)
			return;
		value = other.value;
	}

	void operator*= (const X2_64 &other) {
		value = mbin_xor2_mul_64(value, other.value);
	}

	const X2_64 operator* (const X2_64 &other) const {
		return (mbin_xor2_mul_64(value, other.value));
	}

	void operator%= (const X2_64 &other) {
		value = mbin_xor2_mod_64(value, other.value);
	}

	const X2_64 operator% (const X2_64 &other) const {
		return (mbin_xor2_mod_64(value, other.value));
	}

	void operator/= (const X2_64 &other) {
		value = mbin_xor2_div_64(value, other.value);
	}

	const X2_64 operator/ (const X2_64 &other) const {
		return (mbin_xor2_div_64(value, other.value));
	}

	void operator^= (const X2_64 &other) {
		value ^= other.value;
	}

	const X2_64 operator^ (const X2_64 &other) const {
		return (value ^ other.value);
	}

	void operator&= (const X2_64 &other) {
		value &= other.value;
	}

	const X2_64 operator& (const X2_64 &other) const {
		return (value & other.value);
	}

	void operator<<= (const uint8_t other) {
		value <<= other;
	}

	const X2_64 operator<< (const uint8_t other) const {
		return (value << other);
	}

	void operator>>= (const uint8_t other) {
		value >>= other;
	}

	const X2_64 operator>> (const uint8_t other) const {
		return (value >> other);
	}

	const X2_64 exp (const uint64_t other) const {
		return (mbin_xor2_exp_64(value, other));
	}

	const X2_64 expmod (const uint64_t other, const uint64_t mod) const {
		return (mbin_xor2_exp_mod_any_64(value, other, mod));
	}

	const X2_64 gcd(const uint64_t other) const {
		return (mbin_xor2_gcd_64(value, other));
	}
};

#endif					/* _MATH_BIN_WRAPPER_H_ */
