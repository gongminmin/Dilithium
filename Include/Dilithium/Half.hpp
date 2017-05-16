/**
 * @file Half.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of Dilithium
 * For the latest info, see https://github.com/gongminmin/Dilithium
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Minmin Gong. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _DILITHIUM_HALF_HPP
#define _DILITHIUM_HALF_HPP

#pragma once

#include <boost/operators.hpp>
#include <limits>

#define HALF_MIN		5.96046448e-08f	// Smallest positive half

#define HALF_NRM_MIN	6.10351562e-05f	// Smallest positive normalized half

#define HALF_MAX		65504.0f		// Largest positive half

#define HALF_EPSILON	0.00097656f		// Smallest positive e for which
										// half (1.0 + e) != half (1.0)

#define HALF_MANT_DIG	11				// Number of digits in mantissa
										// (significand + hidden leading 1)

#define HALF_DIG		2				// Number of base 10 digits that
										// can be represented without change

#define HALF_RADIX		2				// Base of the exponent

#define HALF_MIN_EXP	-13				// Minimum negative integer such that
										// HALF_RADIX raised to the power of
										// one less than that integer is a
										// normalized half

#define HALF_MAX_EXP	16				// Maximum positive integer such that
										// HALF_RADIX raised to the power of
										// one less than that integer is a
										// normalized half

#define HALF_MIN_10_EXP	-4				// Minimum positive integer such
										// that 10 raised to that power is
										// a normalized half

#define HALF_MAX_10_EXP	4				// Maximum positive integer such
										// that 10 raised to that power is
										// a normalized half

namespace Dilithium
{
	// 1s5e10m
	class half final : boost::addable<half,
						boost::subtractable<half,
						boost::multipliable<half,
						boost::dividable<half,
						boost::equality_comparable<half>>>>>
	{
	public:
		half() noexcept = default;

		explicit half(float f) noexcept;

		operator float() const noexcept;

		// returns +infinity
		static half PosInf() noexcept;

		// returns -infinity
		static half NegInf() noexcept;

		// returns a NAN with the bit pattern 0111111111111111
		static half QNaN() noexcept;

		// returns a NAN with the bit pattern 0111110111111111
		static half SNaN() noexcept;

		half const & operator+=(half const & rhs) noexcept;
		half const & operator-=(half const & rhs) noexcept;
		half const & operator*=(half const & rhs) noexcept;
		half const & operator/=(half const & rhs) noexcept;

		half& operator=(half const & rhs) noexcept;

		half const operator+() const noexcept;
		half const operator-() const noexcept;

		bool operator==(half const & rhs) noexcept;

	private:
		uint16_t value_;
	};
}

namespace std
{
	template <>
	class numeric_limits<Dilithium::half>
	{
	public:
		static bool const is_specialized = true;
		static int const digits = HALF_MANT_DIG;
		static int const digits10 = HALF_DIG;
		static bool const is_signed = true;
		static bool const is_integer = false;
		static bool const is_exact = false;
		static int const radix = HALF_RADIX;
		static int const min_exponent = HALF_MIN_EXP;
		static int const min_exponent10 = HALF_MIN_10_EXP;
		static int const max_exponent = HALF_MAX_EXP;
		static int const max_exponent10 = HALF_MAX_10_EXP;
		static bool const has_infinity = true;
		static bool const has_quiet_NaN = true;
		static bool const has_signaling_NaN = true;
		static float_denorm_style const has_denorm = denorm_present;
		static bool const has_denorm_loss = false;
		static bool const is_iec559 = false;
		static bool const is_bounded = false;
		static bool const is_modulo = false;
		static bool const traps = true;
		static bool const tinyness_before = false;
		static float_round_style const round_style = round_to_nearest;

		static Dilithium::half min() noexcept
		{
			return Dilithium::half(HALF_NRM_MIN);
		}
		static Dilithium::half max() noexcept
		{
			return Dilithium::half(HALF_MAX);
		}
		static Dilithium::half epsilon() noexcept
		{
			return Dilithium::half(HALF_EPSILON);
		}
		static Dilithium::half round_error() noexcept
		{
			return Dilithium::half(HALF_EPSILON / 2);
		}

		static Dilithium::half infinity() noexcept
		{
			return Dilithium::half::PosInf();
		}
		static Dilithium::half quiet_NaN() noexcept
		{
			return Dilithium::half::QNaN();
		}
		static Dilithium::half signaling_NaN() noexcept
		{
			return Dilithium::half::SNaN();
		}
		static Dilithium::half denorm_min() noexcept
		{
			return Dilithium::half(HALF_MIN);
		}
	};
}

#endif			// _DILITHIUM_HALF_HPP
