/**
 * @file Constants.hpp
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

#ifndef _DILITHIUM_CONSTANTS_HPP
#define _DILITHIUM_CONSTANTS_HPP

#pragma once

#include <Dilithium/ArrayRef.hpp>
#include <Dilithium/Constant.hpp>

namespace Dilithium
{
	class IntegerType;

	class ConstantInt : public Constant
	{
	public:
		static Constant* Get(Type* ty, uint64_t v, bool is_signed = false);
		static ConstantInt* Get(IntegerType* ty, uint64_t v, bool is_signed = false);

		static bool classof(Value const * val)
		{
			return val->GetValueId() == ConstantIntVal;
		}
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class ConstantVector : public Constant
	{
	public:
		static Constant* Get(ArrayRef<Constant*> elems);
		static Constant* GetSplat(uint32_t num_elem, Constant* elem);

		static bool classof(Value const * val)
		{
			return val->GetValueId() == ConstantVectorVal;
		}

	private:
		static Constant* GetImpl(ArrayRef<Constant*> v);
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class ConstantExpr : public Constant
	{
	public:
		uint32_t Opcode() const
		{
			return this->GetSubclassDataFromValue();
		}

		static bool classof(Value const * val)
		{
			return val->GetValueId() == ConstantExprVal;
		}
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class UndefValue : public Constant
	{
	public:
		static UndefValue* Get(Type* ty);
		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_CONSTANTS_HPP
