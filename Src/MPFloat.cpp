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
#include <Dilithium/MPFloat.hpp>
#include <Dilithium/Hashing.hpp>

namespace Dilithium
{
	// Represents floating point arithmetic semantics.
	struct FltSemantics
	{
		// The largest E such that 2^E is representable; this matches the definition of IEEE 754.
		int16_t max_exponent;

		// The smallest E such that 2^E is a normalized number; this matches the definition of IEEE 754.
		int16_t min_exponent;

		// Number of bits in the significand. This includes the integer bit.
		uint32_t precision;

		// Number of bits actually used in the semantics.
		uint32_t size_in_bits;
	};

	FltSemantics constexpr MPFloat::IEEEHalf = { 15, -14, 11, 16 };
	FltSemantics constexpr MPFloat::IEEESingle = { 127, -126, 24, 32 };
	FltSemantics constexpr MPFloat::IEEEDouble = { 1023, -1022, 53, 64 };
	//FltSemantics constexpr MPFloat::IEEEQuad = { 16383, -16382, 113, 128 };
	//FltSemantics constexpr MPFloat::x87DoubleExtended = { 16383, -16382, 64, 80 };
	FltSemantics constexpr MPFloat::Bogus = { 0, 0, 0, 0 };

	void MPFloat::MakeNaN(bool snan, bool negative, MPInt const * fill)
	{
		category_ = FC_NaN;

		if (semantics_ == &IEEEHalf)
		{
			union
			{
				uint16_t i;
				half f;
			} t;
			t.f = snan ? std::numeric_limits<half>::signaling_NaN() : std::numeric_limits<half>::quiet_NaN();
			t.i |= negative ? 0x8000U : 0;
			if (fill != nullptr)
			{
				t.i |= fill->RawData();
			}
			storage_.float_half = t.f;
		}
		else if (semantics_ == &IEEESingle)
		{
			union
			{
				uint32_t i;
				float f;
			} t;
			t.f = snan ? std::numeric_limits<float>::signaling_NaN() : std::numeric_limits<float>::quiet_NaN();
			t.i |= negative ? 0x80000000U : 0;
			if (fill != nullptr)
			{
				t.i |= fill->RawData();
			}
			storage_.float_single = t.f;
		}
		else if (semantics_ == &IEEEDouble)
		{
			union
			{
				uint64_t i;
				double f;
			} t;
			t.f = snan ? std::numeric_limits<double>::signaling_NaN() : std::numeric_limits<double>::quiet_NaN();
			t.i |= negative ? 0x8000000000000000ULL : 0;
			if (fill != nullptr)
			{
				t.i |= fill->RawData();
			}
			storage_.float_double = t.f;
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
	}

	MPFloat MPFloat::MakeNaN(FltSemantics const & sem, bool snan, bool negative, MPInt const * fill)
	{
		MPFloat value(sem, UT_Uninitialized);
		value.MakeNaN(snan, negative, fill);
		return value;
	}

	MPFloat& MPFloat::operator=(MPFloat const & rhs)
	{
		if (this != &rhs)
		{
			semantics_ = rhs.semantics_;
			storage_ = rhs.storage_;
			category_ = rhs.category_;
		}
		return *this;
	}

	MPFloat& MPFloat::operator=(MPFloat&& rhs)
	{
		if (this != &rhs)
		{
			semantics_ = rhs.semantics_;
			storage_ = rhs.storage_;
			category_ = rhs.category_;

			rhs.semantics_ = &Bogus;
		}
		return *this;
	}

	bool MPFloat::BitwiseIsEqual(MPFloat const & rhs) const
	{
		if (this == &rhs)
		{
			return true;
		}
		if ((semantics_ != rhs.semantics_) || (category_ != rhs.category_))
		{
			return false;
		}
		if ((category_ == FC_Zero) || (category_ == FC_Infinity))
		{
			return true;
		}
		else
		{
			if (semantics_ == &IEEEHalf)
			{
				return storage_.float_half == rhs.storage_.float_half;
			}
			else if (semantics_ == &IEEESingle)
			{
				return storage_.float_single == rhs.storage_.float_single;
			}
			else if (semantics_ == &IEEEDouble)
			{
				return storage_.float_double == rhs.storage_.float_double;
			}
			else
			{
				DILITHIUM_NOT_IMPLEMENTED;
			}
		}
	}

	bool MPFloat::IsNegative() const
	{
		if (semantics_ == &IEEEHalf)
		{
			return std::signbit(static_cast<float>(storage_.float_half));
		}
		else if (semantics_ == &IEEESingle)
		{
			return std::signbit(storage_.float_single);
		}
		else if (semantics_ == &IEEEDouble)
		{
			return std::signbit(storage_.float_double);
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
	}

	MPFloat::MPFloat(FltSemantics const & sem, uint64_t value)
	{
		semantics_ = &sem;
		category_ = FC_Normal;
		if (semantics_ == &IEEEHalf)
		{
			storage_.float_half = half(std::ldexp(static_cast<float>(value), sem.precision - 1));
		}
		else if (semantics_ == &IEEESingle)
		{
			storage_.float_single = std::ldexp(static_cast<float>(value), sem.precision - 1);
		}
		else if (semantics_ == &IEEEDouble)
		{
			storage_.float_double = std::ldexp(static_cast<double>(value), sem.precision - 1);
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
	}

	MPFloat::MPFloat(FltSemantics const & sem)
	{
		semantics_ = &sem;
		category_ = FC_Zero;
		if (semantics_ == &IEEEHalf)
		{
			storage_.float_half = half(0.0f);
		}
		else if (semantics_ == &IEEESingle)
		{
			storage_.float_single = 0.0f;
		}
		else if (semantics_ == &IEEEDouble)
		{
			storage_.float_double = 0.0;
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
	}

	MPFloat::MPFloat(FltSemantics const & sem, UninitializedTag tag)
	{
		DILITHIUM_UNUSED(tag);
		semantics_ = &sem;
	}

	MPFloat::MPFloat(FltSemantics const & sem, std::string_view str)
	{
		semantics_ = &sem;
		this->ConvertFromString(str);
	}

	MPFloat::MPFloat(MPFloat const & rhs)
		: semantics_(rhs.semantics_), storage_(rhs.storage_), category_(rhs.category_)
	{
	}

	MPFloat::MPFloat(MPFloat&& rhs)
		: semantics_(&Bogus)
	{
		*this = std::move(rhs);
	}

	MPFloat::~MPFloat()
	{
	}

	uint32_t MPFloat::SemanticsPrecision(FltSemantics const & sem)
	{
		return sem.precision;
	}

	MPFloat::OpStatus MPFloat::Add(MPFloat const & rhs)
	{
		if (semantics_ == &IEEEHalf)
		{
			storage_.float_half += rhs.storage_.float_half;
		}
		else if (semantics_ == &IEEESingle)
		{
			storage_.float_single += rhs.storage_.float_single;
		}
		else if (semantics_ == &IEEEDouble)
		{
			storage_.float_double += rhs.storage_.float_double;
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		this->UpdateCategory();

		return OS_OK;
	}

	MPFloat::OpStatus MPFloat::Subtract(MPFloat const & rhs)
	{
		if (semantics_ == &IEEEHalf)
		{
			storage_.float_half -= rhs.storage_.float_half;
		}
		else if (semantics_ == &IEEESingle)
		{
			storage_.float_single -= rhs.storage_.float_single;
		}
		else if (semantics_ == &IEEEDouble)
		{
			storage_.float_double -= rhs.storage_.float_double;
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		this->UpdateCategory();

		return OS_OK;
	}

	MPFloat::OpStatus MPFloat::Multiply(MPFloat const & rhs)
	{
		if (semantics_ == &IEEEHalf)
		{
			storage_.float_half *= rhs.storage_.float_half;
		}
		else if (semantics_ == &IEEESingle)
		{
			storage_.float_single *= rhs.storage_.float_single;
		}
		else if (semantics_ == &IEEEDouble)
		{
			storage_.float_double *= rhs.storage_.float_double;
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		this->UpdateCategory();

		return OS_OK;
	}

	MPFloat::OpStatus MPFloat::Divide(MPFloat const & rhs)
	{
		if (semantics_ == &IEEEHalf)
		{
			storage_.float_half /= rhs.storage_.float_half;
		}
		else if (semantics_ == &IEEESingle)
		{
			storage_.float_single /= rhs.storage_.float_single;
		}
		else if (semantics_ == &IEEEDouble)
		{
			storage_.float_double /= rhs.storage_.float_double;
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		this->UpdateCategory();

		return OS_OK;
	}

	MPFloat::OpStatus MPFloat::Mod(MPFloat const & rhs)
	{
		if (semantics_ == &IEEEHalf)
		{
			storage_.float_half = half(fmod(storage_.float_half, rhs.storage_.float_half));
		}
		else if (semantics_ == &IEEESingle)
		{
			storage_.float_single = fmod(storage_.float_single, rhs.storage_.float_single);
		}
		else if (semantics_ == &IEEEDouble)
		{
			storage_.float_double = fmod(storage_.float_double, rhs.storage_.float_double);
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		this->UpdateCategory();

		return OS_OK;
	}

	MPFloat::CmpResult MPFloat::Compare(MPFloat const & rhs) const
	{
		CmpResult result;

		BOOST_ASSERT(semantics_ == rhs.semantics_);

		if (semantics_ == &IEEEHalf)
		{
			float fl = storage_.float_half;
			float fr = rhs.storage_.float_half;
			if (std::isless(fl, fr))
			{
				result = CR_LessThan;
			}
			else if (std::isgreater(fl, fr))
			{
				result = CR_GreaterThan;
			}
			else if (std::isunordered(fl, fr))
			{
				result = CR_Unordered;
			}
			else
			{
				result = CR_Equal;
			}
		}
		else if (semantics_ == &IEEESingle)
		{
			if (std::isless(storage_.float_single, rhs.storage_.float_single))
			{
				result = CR_LessThan;
			}
			else if (std::isgreater(storage_.float_single, rhs.storage_.float_single))
			{
				result = CR_GreaterThan;
			}
			else if (std::isunordered(storage_.float_single, rhs.storage_.float_single))
			{
				result = CR_Unordered;
			}
			else
			{
				result = CR_Equal;
			}
		}
		else if (semantics_ == &IEEEDouble)
		{
			if (std::isless(storage_.float_double, rhs.storage_.float_double))
			{
				result = CR_LessThan;
			}
			else if (std::isgreater(storage_.float_double, rhs.storage_.float_double))
			{
				result = CR_GreaterThan;
			}
			else if (std::isunordered(storage_.float_double, rhs.storage_.float_double))
			{
				result = CR_Unordered;
			}
			else
			{
				result = CR_Equal;
			}
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		return result;
	}

	MPFloat::OpStatus MPFloat::Convert(FltSemantics const & to_semantics, bool* loses_info)
	{
		bool li;
		if (semantics_ == &IEEEHalf)
		{
			if (&to_semantics == &IEEEHalf)
			{
				li = false;
			}
			else if (&to_semantics == &IEEESingle)
			{
				half h = storage_.float_half;
				storage_.float_single = h;
				li = false;
			}
			else if (&to_semantics == &IEEEDouble)
			{
				half h = storage_.float_half;
				storage_.float_double = h;
				li = false;
			}
			else
			{
				DILITHIUM_NOT_IMPLEMENTED;
			}
		}
		else if (semantics_ == &IEEESingle)
		{
			if (&to_semantics == &IEEEHalf)
			{
				float f = storage_.float_single;
				storage_.float_half = half(f);
				li = true;
			}
			else if (&to_semantics == &IEEESingle)
			{
				li = false;
			}
			else if (&to_semantics == &IEEEDouble)
			{
				float f = storage_.float_single;
				storage_.float_double = f;
				li = false;
			}
			else
			{
				DILITHIUM_NOT_IMPLEMENTED;
			}
		}
		else if (semantics_ == &IEEEDouble)
		{
			if (&to_semantics == &IEEEHalf)
			{
				float d = static_cast<float>(storage_.float_double);
				storage_.float_half = half(d);
				li = true;
			}
			else if (&to_semantics == &IEEESingle)
			{
				float d = static_cast<float>(storage_.float_double);
				storage_.float_single = d;
				li = true;
			}
			else if (&to_semantics == &IEEEDouble)
			{
				li = false;
			}
			else
			{
				DILITHIUM_NOT_IMPLEMENTED;
			}
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		if (loses_info)
		{
			*loses_info = li;
		}

		this->UpdateCategory();

		return OS_OK;
	}

	MPFloat::OpStatus MPFloat::ConvertToInteger(uint64_t* parts, uint32_t width, bool is_signed, bool* loses_info) const
	{
		DILITHIUM_UNUSED(width);
		DILITHIUM_UNUSED(is_signed);

		if (semantics_ == &MPFloat::IEEEHalf)
		{
			*parts = static_cast<uint64_t>(static_cast<float>(storage_.float_half));
		}
		else if (semantics_ == &MPFloat::IEEESingle)
		{
			*parts = static_cast<uint64_t>(storage_.float_single);
		}
		else if (semantics_ == &MPFloat::IEEEDouble)
		{
			*parts = static_cast<uint64_t>(storage_.float_double);
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		if (loses_info)
		{
			*loses_info = false;
		}

		return OS_OK;
	}

	MPFloat::OpStatus MPFloat::ConvertFromMPInt(MPInt const & val, bool is_signed)
	{
		DILITHIUM_UNUSED(is_signed);

		if (semantics_ == &MPFloat::IEEEHalf)
		{
			storage_.float_half = half(static_cast<float>(val.RawData()));
		}
		else if (semantics_ == &MPFloat::IEEESingle)
		{
			storage_.float_single = static_cast<float>(val.RawData());
		}
		else if (semantics_ == &MPFloat::IEEEDouble)
		{
			storage_.float_double = static_cast<double>(val.RawData());
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		this->UpdateCategory();

		return OS_OK;
	}

	MPFloat::OpStatus MPFloat::ConvertFromString(std::string_view str)
	{
		std::string s = std::string(str);
		if (semantics_ == &IEEEHalf)
		{
			storage_.float_half = half(std::stof(s));
		}
		else if (semantics_ == &IEEESingle)
		{
			storage_.float_single = std::stof(s);
		}
		else if (semantics_ == &IEEEDouble)
		{
			storage_.float_double = std::stod(s);
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		this->UpdateCategory();

		return OS_OK;
	}

	std::size_t HashValue(MPFloat const & arg)
	{
		std::size_t ret = 0;
		if (arg.semantics_ == &MPFloat::IEEEHalf)
		{
			union
			{
				uint16_t i;
				half f;
			} t;
			t.f = arg.storage_.float_half;
			ret = boost::hash_value(t.i);
		}
		else if (arg.semantics_ == &MPFloat::IEEESingle)
		{
			ret = boost::hash_value(arg.storage_.float_single);
		}
		else if (arg.semantics_ == &MPFloat::IEEEDouble)
		{
			ret = boost::hash_value(arg.storage_.float_double);
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
		return ret;
	}

	MPInt MPFloat::BitcastToMPInt() const
	{
		if (semantics_ == &MPFloat::IEEEHalf)
		{
			union
			{
				uint16_t i;
				half f;
			} t;
			t.f = storage_.float_half;
			return MPInt(sizeof(t.i) * CHAR_BIT, t.i);
		}
		else if (semantics_ == &MPFloat::IEEESingle)
		{
			union
			{
				uint32_t i;
				float f;
			} t;
			t.f = storage_.float_single;
			return MPInt(sizeof(t.i) * CHAR_BIT, t.i);
		}
		else if (semantics_ == &MPFloat::IEEEDouble)
		{
			union
			{
				uint64_t i;
				double f;
			} t;
			t.f = storage_.float_double;
			return MPInt(sizeof(t.i) * CHAR_BIT, t.i);
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
	}

	float MPFloat::ConvertToFloat() const
	{
		BOOST_ASSERT_MSG(semantics_ == &IEEESingle, "Float semantics are not IEEESingle");
		MPInt val = this->BitcastToMPInt();
		return val.BitsToFloat();
	}

	double MPFloat::ConvertToDouble() const
	{
		BOOST_ASSERT_MSG(semantics_ == &IEEEDouble, "Float semantics are not IEEEDouble");
		MPInt val = this->BitcastToMPInt();
		return val.BitsToDouble();
	}

	MPFloat MPFloat::Zero(FltSemantics const & sem, bool negative)
	{
		MPFloat ret(sem, UT_Uninitialized);
		if (&sem == &IEEEHalf)
		{
			ret.storage_.float_half = half(negative ? -0.0f : 0.0f);
		}
		else if (&sem == &IEEESingle)
		{
			ret.storage_.float_single = negative ? -0.0f : 0.0f;
		}
		else if (&sem == &IEEEDouble)
		{
			ret.storage_.float_double = negative ? -0.0 : 0.0;
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
		return ret;
	}

	MPFloat MPFloat::Inf(FltSemantics const & sem, bool negative)
	{
		MPFloat ret(sem, UT_Uninitialized);
		if (&sem == &IEEEHalf)
		{
			ret.storage_.float_half = negative ? -std::numeric_limits<half>::infinity() : std::numeric_limits<half>::infinity();
		}
		else if (&sem == &IEEESingle)
		{
			ret.storage_.float_single = negative ? -std::numeric_limits<float>::infinity() : std::numeric_limits<float>::infinity();
		}
		else if (&sem == &IEEEDouble)
		{
			ret.storage_.float_double = negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
		return ret;
	}

	MPFloat MPFloat::AllOnesValue(uint32_t bit_width)
	{
		switch (bit_width)
		{
		case 16:
			return MPFloat(IEEEHalf, MPInt::AllOnesValue(bit_width));
		case 32:
			return MPFloat(IEEESingle, MPInt::AllOnesValue(bit_width));
		case 64:
			return MPFloat(IEEEDouble, MPInt::AllOnesValue(bit_width));

		default:
			DILITHIUM_UNREACHABLE("Unknown floating bit width");
		}
	}

	uint32_t MPFloat::SizeInBits(FltSemantics const & sem)
	{
		return sem.size_in_bits;
	}

	MPFloat::MPFloat(FltSemantics const & sem, MPInt const & val)
	{
		semantics_ = &sem;
		if (semantics_ == &MPFloat::IEEEHalf)
		{
			union
			{
				uint16_t i;
				half f;
			} t;
			t.i = static_cast<uint16_t>(val.RawData());
			storage_.float_half = t.f;
		}
		else if (semantics_ == &MPFloat::IEEESingle)
		{
			union
			{
				uint32_t i;
				float f;
			} t;
			t.i = static_cast<uint32_t>(val.RawData());
			storage_.float_single = t.f;
		}
		else if (semantics_ == &MPFloat::IEEEDouble)
		{
			union
			{
				uint64_t i;
				double f;
			} t;
			t.i = val.RawData();
			storage_.float_double = t.f;
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		this->UpdateCategory();
	}

	MPFloat::MPFloat(float f)
	{
		semantics_ = &IEEESingle;
		storage_.float_single = f;

		this->UpdateCategory();
	}

	MPFloat::MPFloat(double d)
	{
		semantics_ = &IEEEDouble;
		storage_.float_double = d;

		this->UpdateCategory();
	}

	void MPFloat::UpdateCategory()
	{
		if (semantics_ == &IEEEHalf)
		{
			if (storage_.float_half == std::numeric_limits<half>::infinity())
			{
				category_ = FC_Infinity;
			}
			else if ((storage_.float_half == std::numeric_limits<half>::quiet_NaN()) || (storage_.float_half == std::numeric_limits<half>::signaling_NaN()))
			{
				category_ = FC_NaN;
			}
			else if (storage_.float_half == half(0.0))
			{
				category_ = FC_Zero;
			}
			else
			{
				category_ = FC_Normal;
			}
		}
		else if (semantics_ == &IEEESingle)
		{
			if (std::isinf(storage_.float_single))
			{
				category_ = FC_Infinity;
			}
			else if (std::isnan(storage_.float_single))
			{
				category_ = FC_NaN;
			}
			else if (storage_.float_single == 0.0f)
			{
				category_ = FC_Zero;
			}
			else
			{
				category_ = FC_Normal;
			}
		}
		else if (semantics_ == &IEEEDouble)
		{
			if (std::isinf(storage_.float_double))
			{
				category_ = FC_Infinity;
			}
			else if (std::isnan(storage_.float_double))
			{
				category_ = FC_NaN;
			}
			else if (storage_.float_double == 0.0)
			{
				category_ = FC_Zero;
			}
			else
			{
				category_ = FC_Normal;
			}
		}
		else
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
	}
}
