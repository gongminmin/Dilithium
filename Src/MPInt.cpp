/**
 * @file MPInt.cpp
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

#include <Dilithium/Dilithium.hpp>
#include <Dilithium/MPInt.hpp>
#include <Dilithium/MathExtras.hpp>
#include <Dilithium/SmallString.hpp>

namespace
{
	uint32_t Digit(char c_digit, uint8_t radix)
	{
		uint32_t r;
		if (radix == 16)
		{
			r = c_digit - '0';
			if (r <= 9)
			{
				return r;
			}

			r = c_digit - 'A';
			if (r <= radix - 11U)
			{
				return r + 10;
			}

			r = c_digit - 'a';
			if (r <= radix - 11U)
			{
				return r + 10;
			}

			radix = 10;
		}

		r = c_digit - '0';
		if (r < radix)
		{
			return r;
		}

		return 0xFFFFFFFFU;
	}
}

namespace Dilithium 
{
	MPInt::MPInt(uint32_t num_bits, uint64_t val, bool is_signed)
		: bit_width_(num_bits), val_(val)
	{
		DILITHIUM_UNUSED(is_signed);
		BOOST_ASSERT_MSG(bit_width_, "bitwidth too small");
		this->ClearUnusedBits();
	}

	MPInt::MPInt(uint32_t num_bits, std::string_view str, uint8_t radix)
		: val_(0), bit_width_(num_bits)
	{
		BOOST_ASSERT_MSG(bit_width_, "Bitwidth too small");
		this->FromString(num_bits, str, radix);
	}

	bool MPInt::IsPowerOf2() const
	{
		return IsPowerOfTwo64(val_);
	}

	MPInt MPInt::LowBitsSet(uint32_t num_bits, uint32_t lo_bits_set)
	{
		BOOST_ASSERT_MSG(lo_bits_set <= num_bits, "Too many bits to set!");
		if (lo_bits_set == 0)
		{
			return MPInt(num_bits, 0);
		}
		if (lo_bits_set == MPINT_BITS_PER_WORD)
		{
			return MPInt(num_bits, UINT64_MAX);
		}
		if (lo_bits_set <= MPINT_BITS_PER_WORD)
		{
			return MPInt(num_bits, UINT64_MAX >> (MPINT_BITS_PER_WORD - lo_bits_set));
		}
		return AllOnesValue(num_bits).LShr(num_bits - lo_bits_set);
	}

	MPInt MPInt::Splat(uint32_t new_len, MPInt const & v)
	{
		BOOST_ASSERT_MSG(new_len >= v.BitWidth(), "Can't splat to smaller bit width!");

		MPInt ret = v.ZExtOrSelf(new_len);
		for (uint32_t i = v.BitWidth(); i < new_len; i <<= 1)
		{
			ret |= ret << i;
		}

		return ret;
	}

	void MPInt::Assign(uint64_t rhs)
	{
		val_ = rhs;
		this->ClearUnusedBits();
	}

	MPInt MPInt::operator++(int)
	{
		MPInt ret(*this);
		++ (*this);
		return ret;
	}

	MPInt& MPInt::operator++()
	{
		++ val_;
		this->ClearUnusedBits();
		return *this;
	}

	MPInt MPInt::operator--(int)
	{
		MPInt ret(*this);
		-- (*this);
		return ret;
	}

	MPInt& MPInt::operator--()
	{
		-- val_;
		this->ClearUnusedBits();
		return *this;
	}

	MPInt MPInt::operator~() const
	{
		MPInt ret(*this);
		ret.FlipAllBits();
		return ret;
	}

	MPInt& MPInt::operator=(MPInt const & rhs)
	{
		if (this != &rhs)
		{
			val_ = rhs.val_;
			bit_width_ = rhs.bit_width_;
			this->ClearUnusedBits();
		}
		return *this;
	}

	MPInt& MPInt::operator=(MPInt&& rhs)
	{
		if (this != &rhs)
		{
			val_ = rhs.val_;
			bit_width_ = rhs.bit_width_;
			rhs.bit_width_ = 0;
		}
		return *this;
	}

	MPInt& MPInt::operator+=(MPInt const & rhs)
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		val_ += rhs.val_;
		this->ClearUnusedBits();
		return *this;
	}

	MPInt& MPInt::operator-=(MPInt const & rhs)
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		val_ -= rhs.val_;
		this->ClearUnusedBits();
		return *this;
	}

	MPInt& MPInt::operator*=(MPInt const & rhs)
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		val_ *= rhs.val_;
		this->ClearUnusedBits();
		return *this;
	}

	MPInt& MPInt::operator&=(MPInt const & rhs)
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		val_ &= rhs.val_;
		return *this;
	}

	MPInt& MPInt::operator|=(MPInt const & rhs)
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		val_ |= rhs.val_;
		return *this;
	}

	MPInt& MPInt::operator|=(uint64_t rhs)
	{
		val_ |= rhs;
		this->ClearUnusedBits();
		return *this;
	}

	MPInt& MPInt::operator^=(MPInt const & rhs)
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		val_ ^= rhs.val_;
		this->ClearUnusedBits();
		return *this;
	}

	MPInt& MPInt::operator<<=(uint32_t shift)
	{
		BOOST_ASSERT_MSG(shift <= bit_width_, "Invalid shift amount");
		if (shift >= bit_width_)
		{
			val_ = 0;
		}
		else
		{
			val_ <<= shift;
		}
		return *this;
	}

	MPInt MPInt::operator+(MPInt const & rhs) const
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		return MPInt(bit_width_, val_ + rhs.val_);
	}

	MPInt MPInt::operator-(MPInt const & rhs) const
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		return MPInt(bit_width_, val_ - rhs.val_);
	}

	MPInt MPInt::operator*(MPInt const & rhs) const
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		return MPInt(bit_width_, val_ * rhs.val_);
	}

	MPInt MPInt::operator&(MPInt const & rhs) const
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		return MPInt(bit_width_, val_ & rhs.val_);
	}

	MPInt MPInt::operator|(MPInt const & rhs) const
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		return MPInt(bit_width_, val_ | rhs.val_);
	}

	MPInt MPInt::operator^(MPInt const & rhs) const
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Bit widths must be the same");
		return MPInt(bit_width_, val_ ^ rhs.val_);
	}

	bool MPInt::operator[](uint32_t bit_pos) const
	{
		BOOST_ASSERT_MSG(bit_pos < bit_width_, "Bit position out of bounds!");
		return (this->MaskBit(bit_pos) & val_) != 0;
	}

	bool MPInt::operator==(MPInt const & rhs) const
	{
		BOOST_ASSERT_MSG(bit_width_ == rhs.bit_width_, "Comparison requires equal bit widths");
		return val_ == rhs.val_;
	}

	MPInt MPInt::AShr(MPInt const & shift) const
	{
		return this->AShr(static_cast<uint32_t>(shift.LimitedValue(bit_width_)));
	}

	MPInt MPInt::AShr(uint32_t shift) const
	{
		BOOST_ASSERT_MSG(shift <= bit_width_, "Invalid shift amount");
		if (shift == 0)
		{
			return *this;
		}
		else if (shift == bit_width_)
		{
			return MPInt(bit_width_, 0);
		}
		else
		{
			uint32_t sign_bit = MPINT_BITS_PER_WORD - bit_width_;
			return MPInt(bit_width_, (((static_cast<int64_t>(val_) << sign_bit) >> sign_bit) >> shift));
		}
	}

	MPInt MPInt::LShr(MPInt const & shift) const
	{
		return this->LShr(static_cast<uint32_t>(shift.LimitedValue(bit_width_)));
	}

	MPInt MPInt::Shl(uint32_t shift) const
	{
		BOOST_ASSERT_MSG(shift <= bit_width_, "Invalid shift amount");
		if (shift >= bit_width_)
		{
			return MPInt(bit_width_, 0);
		}
		else
		{
			return MPInt(bit_width_, val_ << shift);
		}
	}

	MPInt MPInt::LShr(uint32_t shift) const
	{
		if (shift >= bit_width_)
		{
			return MPInt(bit_width_, 0);
		}
		else
		{
			return MPInt(bit_width_, this->val_ >> shift);
		}
	}

	MPInt MPInt::Shl(MPInt const & shift) const
	{
		return this->Shl(static_cast<uint32_t>(shift.LimitedValue(bit_width_)));
	}

	MPInt MPInt::Trunc(uint32_t width) const
	{
		BOOST_ASSERT_MSG(width < bit_width_, "Invalid MPInt Truncate request");
		BOOST_ASSERT_MSG(width, "Can't truncate to 0 bits");

		return MPInt(width, val_);
	}

	MPInt MPInt::SExt(uint32_t width) const
	{
		BOOST_ASSERT_MSG(width > bit_width_, "Invalid MPInt SignExtend request");

		uint64_t val = val_ << (MPINT_BITS_PER_WORD - bit_width_);
		val = static_cast<int64_t>(val >> (width - bit_width_));
		return MPInt(width, val >> (MPINT_BITS_PER_WORD - width));
	}

	MPInt MPInt::ZExt(uint32_t width) const
	{
		BOOST_ASSERT_MSG(width > bit_width_, "Invalid MPInt ZeroExtend request");
		return MPInt(width, val_);
	}

	MPInt MPInt::SExtOrTrunc(uint32_t width) const
	{
		if (bit_width_ < width)
		{
			return this->SExt(width);
		}
		else if (bit_width_ > width)
		{
			return this->Trunc(width);
		}
		else
		{
			return *this;
		}
	}

	MPInt MPInt::ZExtOrSelf(uint32_t width) const
	{
		if (bit_width_ < width)
		{
			return this->ZExt(width);
		}
		else
		{
			return *this;
		}
	}

	void MPInt::SetBit(uint32_t bit_pos)
	{
		val_ |= this->MaskBit(bit_pos);
	}

	void MPInt::ClearBit(uint32_t bit_pos)
	{
		val_ &= ~this->MaskBit(bit_pos);
	}

	void MPInt::FlipAllBits()
	{
		val_ ^= UINT64_MAX;
		this->ClearUnusedBits();
	}

	uint32_t MPInt::CountLeadingZeros() const
	{
		uint32_t unused_bits = MPINT_BITS_PER_WORD - bit_width_;
		return static_cast<uint32_t>(Dilithium::CountLeadingZeros(val_) - unused_bits);
	}

	uint32_t MPInt::CountPopulation() const
	{
		return Dilithium::CountPopulation(val_);
	}

	void MPInt::FromString(uint32_t num_bits, std::string_view str, uint8_t radix)
	{
		BOOST_ASSERT_MSG(!str.empty(), "Invalid string length");
		BOOST_ASSERT_MSG((radix == 2) || (radix == 8) || (radix == 10) || (radix == 16), "Radix should be 2, 8, 10, or 16!");

		std::string_view::iterator p = str.begin();
		size_t slen = str.size();
		bool is_neg = *p == '-';
		if (*p == '-' || *p == '+')
		{
			++ p;
			-- slen;
			BOOST_ASSERT_MSG(slen, "String is only a sign, needs a value.");
		}
		BOOST_ASSERT_MSG((slen <= num_bits) || (radix != 2), "Insufficient bit width");
		BOOST_ASSERT_MSG(((slen - 1) * 3 <= num_bits) || (radix != 8), "Insufficient bit width");
		BOOST_ASSERT_MSG(((slen - 1) * 4 <= num_bits) || (radix != 16), "Insufficient bit width");
		BOOST_ASSERT_MSG((((slen - 1) * 64) / 22 <= num_bits) || (radix != 10), "Insufficient bit width");

		uint32_t shift = (radix == 16) ? 4 : ((radix == 8) ? 3 : ((radix == 2) ? 1 : 0));

		MPInt mp_digit(bit_width_, 0);
		MPInt mp_radix(bit_width_, radix);

		for (std::string_view::iterator e = str.end(); p != e; ++ p)
		{
			uint32_t digit = Digit(*p, radix);
			BOOST_ASSERT_MSG(digit < radix, "Invalid character in digit string");

			if (slen > 1)
			{
				if (shift)
				{
					*this <<= shift;
				}
				else
				{
					*this *= mp_radix;
				}
			}

			mp_digit.val_ = digit;
			*this += mp_digit;
		}
		if (is_neg)
		{
			-- (*this);
			this->FlipAllBits();
		}
	}

	void MPInt::ToString(boost::container::small_vector_base<char>& str, uint32_t radix, bool is_signed, bool format_as_c_literal) const
	{
		BOOST_ASSERT_MSG(radix == 2 || radix == 8 || radix == 10 || radix == 16, "Radix should be 2, 8, 10, or 16!");

		char const * prefix = "";
		if (format_as_c_literal)
		{
			switch (radix)
			{
			case 2:
				// Binary literals are a non-standard extension added in gcc 4.3:
				// http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
				prefix = "0b";
				break;
			case 8:
				prefix = "0";
				break;
			case 10:
				break; // No prefix
			case 16:
				prefix = "0x";
				break;

			default:
				DILITHIUM_UNREACHABLE("Invalid radix!");
			}
		}

		if (*this == 0)
		{
			while (*prefix)
			{
				str.push_back(*prefix);
				++ prefix;
			};
			str.push_back('0');
			return;
		}

		static char const digits[] = "0123456789ABCDEF";

		char buff[65];
		char* buff_ptr = buff + 65;

		uint64_t n;
		if (!is_signed)
		{
			n = this->ZExtValue();
		}
		else
		{
			int64_t i = this->SExtValue();
			if (i >= 0)
			{
				n = i;
			}
			else
			{
				str.push_back('-');
				n = static_cast<uint64_t>(-i);
			}
		}

		while (*prefix)
		{
			str.push_back(*prefix);
			++ prefix;
		};

		while (n)
		{
			*-- buff_ptr = digits[n % radix];
			n /= radix;
		}
		str.insert(str.end(), buff_ptr, buff + 65);
	}

	void MPInt::Print(std::ostream& os, bool is_signed) const
	{
		SmallString<40> s;
		this->ToString(s, 10, is_signed, false);
		os << s;
	}

	double MPInt::BitsToDouble() const
	{
		union
		{
			uint64_t i;
			double d;
		} t;
		t.i = val_;
		return t.d;
	}

	float MPInt::BitsToFloat() const
	{
		union
		{
			uint32_t i;
			float f;
		} t;
		t.i = static_cast<uint32_t>(val_);
		return t.f;
	}

	void MPInt::ClearUnusedBits()
	{
		if (bit_width_ > 0)
		{
			uint64_t mask = ~uint64_t(0ULL) >> (MPINT_BITS_PER_WORD - bit_width_);
			val_ &= mask;
		}
	}
}
