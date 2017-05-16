/**
 * @file MPInt.hpp
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

#ifndef _DILITHIUM_MP_INT_HPP
#define _DILITHIUM_MP_INT_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/ArrayRef.hpp>

#include <boost/assert.hpp>

namespace Dilithium
{
	// Multi-precision integer. Must <= 64 bits.
	class MPInt
	{
		static uint32_t constexpr MPINT_BITS_PER_WORD = sizeof(uint64_t) * 8;

	public:
		explicit MPInt()
			: bit_width_(1)
		{
		}
		MPInt(uint32_t num_bits, uint64_t val, bool is_signed = false);
		MPInt(uint32_t num_bits, std::string_view str, uint8_t radix);
		MPInt(MPInt const & rhs)
			: bit_width_(rhs.bit_width_), val_(rhs.val_)
		{
		}
		MPInt(MPInt&& rhs)
			: bit_width_(rhs.bit_width_), val_(rhs.val_)
		{
			rhs.bit_width_ = 0;
		}

		bool IsNegative() const
		{
			return (*this)[bit_width_ - 1];
		}
		bool IsAllOnesValue() const
		{
			return val_ == ~0ULL >> (MPINT_BITS_PER_WORD - bit_width_);
		}
		bool IsMaxValue() const
		{
			return this->IsAllOnesValue();
		}
		bool IsMaxSignedValue() const
		{
			return !this->IsNegative() && (this->CountPopulation() == bit_width_ - 1);
		}
		bool isMinValue() const
		{
			return !(*this);
		}
		bool IsMinSignedValue() const
		{
			return this->IsNegative() && this->IsPowerOf2();
		}
		bool IsPowerOf2() const;
		bool IsSignBit() const
		{
			return this->IsMinSignedValue();
		}

		uint64_t LimitedValue(uint64_t limit = ~0ULL) const
		{
			return this->ZExtValue() > limit ? limit : this->ZExtValue();
		}
		static MPInt AllOnesValue(uint32_t num_bits)
		{
			return MPInt(num_bits, UINT64_MAX, true);
		}
		static MPInt NullValue(uint32_t num_bits)
		{
			return MPInt(num_bits, 0);
		}

		static MPInt LowBitsSet(uint32_t num_bits, uint32_t lo_bits_set);
		static MPInt Splat(uint32_t new_len, MPInt const & v);

		uint64_t RawData() const
		{
			return val_;
		}

		void Assign(uint64_t rhs);

		MPInt operator++(int);
		MPInt& operator++();

		MPInt operator--(int);
		MPInt& operator--();

		MPInt operator~() const;
		MPInt operator-() const
		{
			return MPInt(bit_width_, 0) - (*this);
		}
		bool operator!() const
		{
			return !val_;
		}

		MPInt& operator=(MPInt const & rhs);
		MPInt& operator=(MPInt&& rhs);
		MPInt& operator+=(MPInt const & rhs);
		MPInt& operator-=(MPInt const & rhs);
		MPInt& operator*=(MPInt const & rhs);
		MPInt& operator&=(MPInt const & rhs);
		MPInt& operator|=(MPInt const & rhs);
		MPInt& operator|=(uint64_t rhs);
		MPInt& operator^=(MPInt const & rhs);
		MPInt& operator<<=(uint32_t shift);

		MPInt operator+(MPInt const & rhs) const;
		MPInt operator+(uint64_t rhs) const
		{
			return (*this) + MPInt(bit_width_, rhs);
		}
		MPInt operator-(MPInt const & rhs) const;
		MPInt operator-(uint64_t rhs) const
		{
			return (*this) - MPInt(bit_width_, rhs);
		}
		MPInt operator*(MPInt const & rhs) const;
		MPInt operator&(MPInt const & rhs) const;
		MPInt operator|(MPInt const & rhs) const;
		MPInt operator^(MPInt const & rhs) const;
		MPInt operator<<(uint32_t bits) const
		{
			return this->Shl(bits);
		}
		MPInt operator<<(MPInt const & bits) const
		{
			return this->Shl(bits);
		}

		bool operator[](uint32_t bit_pos) const;

		bool operator==(MPInt const & rhs) const;
		bool operator==(uint64_t val) const
		{
			return val_ == val;
		}

		bool operator!=(MPInt const & rhs) const
		{
			return !((*this) == rhs);
		}
		bool operator!=(uint64_t val) const
		{
			return !((*this) == val);
		}

		MPInt AShr(uint32_t shift) const;
		MPInt LShr(uint32_t shift) const;
		MPInt Shl(uint32_t shift) const;
		MPInt AShr(MPInt const & shift) const;
		MPInt LShr(MPInt const & shift) const;
		MPInt Shl(MPInt const & shift) const;

		MPInt Trunc(uint32_t width) const;
		MPInt SExt(uint32_t width) const;
		MPInt ZExt(uint32_t width) const;
		MPInt SExtOrTrunc(uint32_t width) const;
		MPInt ZExtOrSelf(uint32_t width) const;
		uint64_t ZExtValue() const
		{
			return val_;
		}
		int64_t SExtValue() const
		{
			return static_cast<int64_t>(val_ << (MPINT_BITS_PER_WORD - bit_width_)) >> (MPINT_BITS_PER_WORD - bit_width_);
		}

		void SetBit(uint32_t bit_pos);
		void ClearBit(uint32_t bit_pos);
		void ClearAllBits()
		{
			val_ = 0;
		}

		void FlipAllBits();

		uint32_t BitWidth() const
		{
			return bit_width_;
		}
		uint32_t ActiveBits() const
		{
			return bit_width_ - this->CountLeadingZeros();
		}

		uint32_t CountLeadingZeros() const;
		uint32_t CountPopulation() const;

		double BitsToDouble() const;
		float BitsToFloat() const;

		void Print(std::ostream& os, bool is_signed) const;

		void ToString(boost::container::small_vector_base<char>& str, uint32_t Radix, bool is_signed,
			bool format_as_c_literal = false) const;

	private:
		static uint64_t MaskBit(uint32_t bit_pos)
		{
			return 1ULL << bit_pos;
		}

		void ClearUnusedBits();

		void FromString(uint32_t num_bits, std::string_view str, uint8_t radix);

	private:
		uint64_t val_;
		uint32_t bit_width_;
	};

	inline bool operator==(uint64_t v1, MPInt const & v2)
	{
		return v2 == v1;
	}
	inline bool operator!=(uint64_t v1, MPInt const & v2)
	{
		return v2 != v1;
	}

	inline std::ostream &operator<<(std::ostream& os, MPInt const & mpi)
	{
		mpi.Print(os, true);
		return os;
	}
}


namespace std
{
	template <>
	struct hash<Dilithium::MPInt>
	{
		typedef std::size_t result_type;
		typedef Dilithium::MPInt argument_type;

		result_type operator()(argument_type const & rhs) const
		{
			return hash<uint64_t>()(rhs.RawData());
		}
	};

	template <>
	struct equal_to<Dilithium::MPInt>
	{
		typedef std::size_t result_type;
		typedef Dilithium::MPInt first_argument_type;
		typedef Dilithium::MPInt second_argument_type;

		result_type operator()(first_argument_type const & lhs, second_argument_type const & rhs) const
		{
			return (lhs.BitWidth() == rhs.BitWidth()) && (lhs == rhs);
		}
	};
}

#endif		// _DILITHIUM_MP_INT_HPP
