/**
 * @file MathExtras.hpp
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

#ifndef _DILITHIUM_MATH_EXTRAS_HPP
#define _DILITHIUM_MATH_EXTRAS_HPP

#pragma once

#include <Dilithium/Util.hpp>

#include <limits>

namespace Dilithium
{
	namespace Detail
	{
		template <typename T, std::size_t SizeOfT>
		struct LeadingZerosCounter
		{
			static std::size_t Count(T val)
			{
				if (!val)
				{
					return std::numeric_limits<T>::digits;
				}
				else
				{
					std::size_t zero_bits = 0;
					for (T shift = std::numeric_limits<T>::digits >> 1; shift; shift >>= 1)
					{
						T tmp = val >> shift;
						if (tmp)
						{
							val = tmp;
						}
						else
						{
							zero_bits |= shift;
						}
					}
					return zero_bits;
				}
			}
		};
	}

	template <typename T>
	std::size_t CountLeadingZeros(T val)
	{
		static_assert(std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed,
			"Only unsigned integral types are allowed.");
		return Detail::LeadingZerosCounter<T, sizeof(T)>::Count(val);
	}

	inline bool IsPowerOfTwo32(uint32_t val)
	{
		return val && !(val & (val - 1));
	}

	inline uint32_t Log2_32(uint32_t val)
	{
		return 31 - static_cast<uint32_t>(CountLeadingZeros(val));
	}
}

#endif		// _DILITHIUM_MATH_EXTRAS_HPP
