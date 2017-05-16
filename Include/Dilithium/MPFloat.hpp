/**
 * @file MPFloat.hpp
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

#ifndef _DILITHIUM_MP_FLOAT_HPP
#define _DILITHIUM_MP_FLOAT_HPP

#pragma once

#include <Dilithium/Half.hpp>
#include <Dilithium/MPInt.hpp>

#include <boost/assert.hpp>

namespace Dilithium
{
	struct FltSemantics;

	// Multi-precision float. Coulbe be half, single, or double.
	class MPFloat
	{
	public:
		static FltSemantics const IEEEHalf;
		static FltSemantics const IEEESingle;
		static FltSemantics const IEEEDouble;
		// No quad, ppc double double, and double extended, for now
		//static FltSemantics const IEEEquad;
		//static FltSemantics const PPCDoubleDouble;
		//static FltSemantics const x87DoubleExtended;

		// A Pseudo fltsemantic used to construct MPFloats that cannot conflict with anything real.
		static const FltSemantics Bogus;

		static uint32_t SemanticsPrecision(FltSemantics const & sem);

		// IEEE-754R 5.11: Floating Point Comparison Relations.
		enum CmpResult
		{
			CR_LessThan,
			CR_Equal,
			CR_GreaterThan,
			CR_Unordered
		};

		// IEEE-754R 7: Default exception handling.
		//
		// opUnderflow or opOverflow are always returned or-ed with opInexact.
		enum OpStatus
		{
			OS_OK = 0x00,
			OS_InvalidOp = 0x01,
			OS_DivByZero = 0x02,
			OS_Overflow = 0x04,
			OS_Underflow = 0x08,
			OS_Inexact = 0x10
		};

		// Category of internally-represented number.
		enum FltCategory
		{
			FC_Infinity,
			FC_NaN,
			FC_Normal,
			FC_Zero
		};

		// Convenience enum used to construct an uninitialized MPFloat.
		enum UninitializedTag
		{
			UT_Uninitialized
		};

		MPFloat(FltSemantics const & sem); // Default construct to 0.0
		MPFloat(FltSemantics const & sem, std::string_view str);
		MPFloat(FltSemantics const & sem, uint64_t value);
		MPFloat(FltSemantics const & sem, UninitializedTag tag);
		MPFloat(FltSemantics const & sem, MPInt const & val);
		explicit MPFloat(double d);
		explicit MPFloat(float f);
		MPFloat(MPFloat const & rhs);
		MPFloat(MPFloat&& rhs);
		~MPFloat();

		static MPFloat Zero(FltSemantics const & sem, bool negative = false);
		static MPFloat Inf(FltSemantics const & sem, bool negative = false);
		static MPFloat NaN(FltSemantics const & sem, bool negative = false, uint32_t type = 0)
		{
			if (type)
			{
				MPInt fill(64, type);
				return QNaN(sem, negative, &fill);
			}
			else
			{
				return QNaN(sem, negative, nullptr);
			}
		}
		static MPFloat QNaN(FltSemantics const & sem, bool negative = false, MPInt const * payload = nullptr)
		{
			return MakeNaN(sem, false, negative, payload);
		}
		static MPFloat SNaN(FltSemantics const & sem, bool negative = false, MPInt const  *payload = nullptr)
		{
			return MakeNaN(sem, true, negative, payload);
		}
		static MPFloat AllOnesValue(uint32_t bit_width);
		static uint32_t SizeInBits(FltSemantics const & sem);

		OpStatus Add(MPFloat const & rhs);
		OpStatus Subtract(MPFloat const & rhs);
		OpStatus Multiply(MPFloat const & rhs);
		OpStatus Divide(MPFloat const & rhs);
		OpStatus Mod(MPFloat const & rhs);

		MPFloat operator+(MPFloat const & rhs) const
		{
			MPFloat ret = *this;
			ret.Add(rhs);
			return ret;
		}
		MPFloat operator-(MPFloat const & rhs) const
		{
			MPFloat ret = *this;
			ret.Subtract(rhs);
			return ret;
		}
		MPFloat operator*(MPFloat const & rhs) const
		{
			MPFloat ret = *this;
			ret.Multiply(rhs);
			return ret;
		}
		MPFloat operator/(MPFloat const & rhs) const
		{
			MPFloat ret = *this;
			ret.Divide(rhs);
			return ret;
		}

		OpStatus Convert(FltSemantics const & to_sem, bool* loses_info);
		OpStatus ConvertToInteger(uint64_t* parts, uint32_t width, bool is_signed, bool* loses_info) const;
		OpStatus ConvertFromMPInt(MPInt const & val, bool is_signed);
		OpStatus ConvertFromString(std::string_view str);
		MPInt BitcastToMPInt() const;
		double ConvertToDouble() const;
		float ConvertToFloat() const;

		CmpResult Compare(MPFloat const & rhs) const;

		bool BitwiseIsEqual(MPFloat const & rhs) const;

		bool IsNegative() const;
		bool IsFinite() const
		{
			return !this->IsNaN() && !this->IsInfinity();
		}
		bool IsZero() const
		{
			return category_ == FC_Zero;
		}
		bool IsInfinity() const
		{
			return category_ == FC_Infinity;
		}
		bool IsNaN() const
		{
			return category_ == FC_NaN;
		}

		FltCategory Category() const
		{
			return category_;
		}
		FltSemantics const & Semantics() const
		{
			return *semantics_;
		}
		bool IsNonZero() const
		{
			return category_ != FC_Zero;
		}
		bool IsFiniteNonZero() const
		{
			return this->IsFinite() && !this->IsZero();
		}
		bool IsPosZero() const
		{
			return this->IsZero() && !this->IsNegative();
		}
		bool IsNegZero() const
		{
			return this->IsZero() && this->IsNegative();
		}

		MPFloat& operator=(MPFloat const & rhs);
		MPFloat& operator=(MPFloat&& rhs);

		bool operator==(MPFloat const & rhs) const = delete;

		friend std::size_t HashValue(MPFloat const & arg);

	private:
		void MakeNaN(bool snan = false, bool negative = false, MPInt const * fill = nullptr);
		static MPFloat MakeNaN(FltSemantics const & sem, bool snan, bool negative, MPInt const * fill);

		void UpdateCategory();

	private:
		FltSemantics const * semantics_;

		union FloatStorage
		{
			half float_half;
			float float_single;
			double float_double;

			FloatStorage() = default;
			FloatStorage(FloatStorage const & rhs)
				: float_double(rhs.float_double)
			{
			}

			FloatStorage& operator=(FloatStorage const & rhs)
			{
				if (this != &rhs)
				{
					float_double = rhs.float_double;
				}
				return *this;
			}
		};
		FloatStorage storage_;

		FltCategory category_ : 3;
	};

	std::size_t HashValue(MPFloat const & arg);
}

namespace std
{
	template <>
	struct hash<Dilithium::MPFloat>
	{
		typedef std::size_t result_type;
		typedef Dilithium::MPFloat argument_type;

		result_type operator()(argument_type const & rhs) const
		{
			return Dilithium::HashValue(rhs);
		}
	};

	template <>
	struct equal_to<Dilithium::MPFloat>
	{
		typedef std::size_t result_type;
		typedef Dilithium::MPFloat first_argument_type;
		typedef Dilithium::MPFloat second_argument_type;

		result_type operator()(first_argument_type const & lhs, second_argument_type const & rhs) const
		{
			return lhs.BitwiseIsEqual(rhs);
		}
	};
}

#endif		// _DILITHIUM_MP_FLOAT_HPP
