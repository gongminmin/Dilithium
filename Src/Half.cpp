/**
 * @file Half.cpp
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
#include <Dilithium/Half.hpp>

namespace Dilithium
{
	half::half(float f) noexcept
	{
		union FNI
		{
			float f;
			int32_t i;
		} fni;
		fni.f = f;
		int32_t i = fni.i;

		int32_t s = (i >> 16) & 0x00008000;
		int32_t e = ((i >> 23) & 0x000000FF) - (127 - 15);
		int32_t m = i & 0x007FFFFF;

		if (e <= 0)
		{
			if (e < -10)
			{
				value_ = 0;
			}
			else
			{
				m = (m | 0x00800000) >> (1 - e);

				if (m & 0x00001000)
				{
					m += 0x00002000;
				}

				value_ = static_cast<uint16_t>(s | (m >> 13));
			}
		}
		else
		{
			if (0xFF - (127 - 15) == e)
			{
				e = 31;
			}
			else
			{
				if (m & 0x00001000)
				{
					m += 0x00002000;

					if (m & 0x00800000)
					{
						m = 0;		// overflow in significand,
						e += 1;		// adjust exponent
					}
				}
			}

			value_ = static_cast<uint16_t>(s | (e << 10) | (m >> 13));
		}
	}

	half::operator float() const noexcept
	{
		int32_t ret;

		int32_t s = ((value_ & 0x8000) >> 15) << 31;
		int32_t e = (value_ & 0x7C00) >> 10;
		int32_t m = value_ & 0x03FF;

		if (0 == e)
		{
			if (m != 0)
			{
				// Denormalized number -- renormalize it

				while (!(m & 0x00000400))
				{
					m <<= 1;
					e -= 1;
				}

				e += 1;
				m &= ~0x00000400;
			}
		}
		else
		{
			if (31 == e)
			{
				if (m != 0)
				{
					// Nan -- preserve sign and significand bits
					e = 0xFF - (127 - 15);
				}
			}
		}

		// Normalized number
		e += 127 - 15;
		m <<= 13;

		ret = s | (e << 23) | m;

		union INF
		{
			int32_t i;
			float f;
		} inf;
		inf.i = ret;
		return inf.f;
	}

	half half::PosInf() noexcept
	{
		half h;
		h.value_ = 0x7C00;
		return h;
	}

	half half::NegInf() noexcept
	{
		half h;
		h.value_ = 0xFC00;
		return h;
	}

	half half::QNaN() noexcept
	{
		half h;
		h.value_ = 0x7FFF;
		return h;
	}

	half half::SNaN() noexcept
	{
		half h;
		h.value_ = 0x7DFF;
		return h;
	}


	half const & half::operator+=(half const & rhs) noexcept
	{
		*this = half(float(*this) + float(rhs));
		return *this;
	}

	half const & half::operator-=(half const & rhs) noexcept
	{
		*this = half(float(*this) - float(rhs));
		return *this;
	}

	half const & half::operator*=(half const & rhs) noexcept
	{
		*this = half(float(*this) * float(rhs));
		return *this;
	}

	half const & half::operator/=(half const & rhs) noexcept
	{
		*this = half(float(*this) / float(rhs));
		return *this;
	}

	half& half::operator=(half const & rhs) noexcept
	{
		if (this != &rhs)
		{
			value_ = rhs.value_;
		}
		return *this;
	}

	half const half::operator+() const noexcept
	{
		return *this;
	}

	half const half::operator-() const noexcept
	{
		half temp(*this);
		temp.value_ = -temp.value_;
		return temp;
	}

	bool half::operator==(half const & rhs) noexcept
	{
		return value_ == rhs.value_;
	}
}
