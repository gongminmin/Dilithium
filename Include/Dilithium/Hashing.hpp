/**
 * @file Hashing.hpp
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

#ifndef _DILITHIUM_HASHING_HPP
#define _DILITHIUM_HASHING_HPP

#include <Dilithium/CXX17/string_view.hpp>

#include <functional>

#include <boost/functional/hash.hpp>

namespace boost
{
	template <typename T>
	std::size_t hash_value(std::basic_string_view<T> const & v)
	{
		return boost::hash_range(v.begin(), v.end());
	}
}

namespace std
{
	template <typename T1, typename T2>
	struct hash<std::pair<T1, T2>>
	{
		typedef std::size_t result_type;
		typedef std::pair<T1, T2> argument_type;

		result_type operator()(argument_type const & rhs) const
		{
			return boost::hash_value(rhs);
		}
	};
}

#endif		// _DILITHIUM_CASTING_HPP
