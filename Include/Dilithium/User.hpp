/**
 * @file User.hpp
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

#ifndef _DILITHIUM_USER_HPP
#define _DILITHIUM_USER_HPP

#pragma once

#include <Dilithium/Value.hpp>

#include <boost/range/iterator_range.hpp>

namespace Dilithium
{
	class User : public Value
	{
	public:
		typedef Use* op_iterator;
		typedef Use const * const_op_iterator;
		typedef boost::iterator_range<op_iterator> op_range;
		typedef boost::iterator_range<const_op_iterator> const_op_range;

	public:
		Value* Operand(uint32_t idx) const;

		const_op_range Operands() const;
		op_range Operands();

		// DILITHIUM_NOT_IMPLEMENTED
	};

	template<>
	struct simplify_type<User::op_iterator>
	{
		typedef Value* SimpleType;
		static SimpleType getSimplifiedValue(User::op_iterator& val)
		{
			return val->Get();
		}
	};
	template<>
	struct simplify_type<User::const_op_iterator>
	{
		typedef Value /*const*/ * SimpleType;
		static SimpleType getSimplifiedValue(User::const_op_iterator& val)
		{
			return val->Get();
		}
	};
}

#endif		// _DILITHIUM_USER_HPP
