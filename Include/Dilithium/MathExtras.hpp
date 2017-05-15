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

#ifdef _MSC_VER
	#include <intrin.h>
#endif

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

#if defined(__GNUC__) || defined(_MSC_VER)
		template <typename T>
		struct LeadingZerosCounter<T, 4>
		{
			static std::size_t Count(T val)
			{
				if (val == 0)
				{
					return 32;
				}

#if __has_builtin(__builtin_clz)
				return __builtin_clz(val);
#elif defined(_MSC_VER)
				unsigned long index;
				_BitScanReverse(&index, val);
				return index ^ 31;
#endif
			}
		};

#if !defined(_MSC_VER) || defined(_M_X64)
		template <typename T>
		struct LeadingZerosCounter<T, 8>
		{
			static std::size_t Count(T val)
			{
				if (val == 0)
				{
					return 64;
				}

#if __has_builtin(__builtin_clzll)
				return __builtin_clzll(val);
#elif defined(_MSC_VER)
				unsigned long index;
				_BitScanReverse64(&index, val);
				return index ^ 63;
#endif
			}
		};
#endif
#endif

		template <typename T, std::size_t SizeOfT>
		struct PopulationCounter
		{
			static uint32_t Count(T val)
			{
				static_assert(SizeOfT <= 4, "Not implemented!");
#if defined(__GNUC__)
				return __builtin_popcount(val);
#else
				uint32_t v = val;
				v = v - ((v >> 1) & 0x55555555);
				v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
				return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
#endif
			}
		};

		template <typename T>
		struct PopulationCounter<T, 8>
		{
			static uint32_t Count(T val)
			{
#if __GNUC__ >= 4
				return __builtin_popcountll(val);
#else
				uint64_t v = val;
				v = v - ((v >> 1) & 0x5555555555555555ULL);
				v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
				v = (v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
				return static_cast<uint32_t>((v * 0x0101010101010101ULL) >> 56);
#endif
			}
		};
	}

	template <typename T>
	inline std::size_t CountLeadingZeros(T val)
	{
		static_assert(std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed,
			"Only unsigned integral types are allowed.");
		return Detail::LeadingZerosCounter<T, sizeof(T)>::Count(val);
	}

	template <typename T>
	inline uint32_t CountPopulation(T val)
	{
		static_assert(std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed,
			"Only unsigned integral types are allowed.");
		return Detail::PopulationCounter<T, sizeof(T)>::Count(val);
	}

	template <uint32_t N>
	inline bool IsUInt(uint64_t val)
	{
		return (N >= 64) || (val < (1ULL << N));
	}

	inline bool IsPowerOfTwo32(uint32_t val)
	{
		return val && !(val & (val - 1));
	}

	inline bool IsPowerOfTwo64(uint64_t val)
	{
		return val && !(val & (val - 1LL));
	}

	inline uint32_t Log2_32(uint32_t val)
	{
		return 31 - static_cast<uint32_t>(CountLeadingZeros(val));
	}

	inline uint64_t NextPowerOf2(uint64_t val)
	{
		val |= (val >> 1);
		val |= (val >> 2);
		val |= (val >> 4);
		val |= (val >> 8);
		val |= (val >> 16);
		val |= (val >> 32);
		return val + 1;
	}

	inline uint64_t RoundUpToAlignment(uint64_t value, uint64_t align)
	{
		return (value + align - 1) / align * align;
	}
}

#endif		// _DILITHIUM_MATH_EXTRAS_HPP
