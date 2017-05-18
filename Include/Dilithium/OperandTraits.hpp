/**
 * @file OperandTraits.hpp
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

#ifndef _DILITHIUM_OPERAND_TRAITS_HPP
#define _DILITHIUM_OPERAND_TRAITS_HPP

#pragma once

namespace Dilithium
{
	template <typename SubClass, uint32_t ARITY>
	struct FixedNumOperandTraits
	{
		static Use* OpBegin(SubClass* u)
		{
			return static_cast<User*>(u)->OperandList();
		}
		static Use* OpEnd(SubClass* u)
		{
			return static_cast<User*>(u)->OperandList() + ARITY;
		}
		static uint32_t NumOperands(User const * u)
		{
			DILITHIUM_UNUSED(u);
			return ARITY;
		}
	};

	template <typename SubClass, uint32_t ARITY = 1>
	struct OptionalOperandTraits : public FixedNumOperandTraits<SubClass, ARITY>
	{
		static uint32_t NumOperands(User const * u)
		{
			return U->NumOperands();
		}
	};

	template <typename SubClass>
	struct VariadicOperandTraits
	{
		static Use* OpBegin(SubClass* u)
		{
			return static_cast<User*>(u)->OperandList();
		}
		static Use* OpEnd(SubClass* u)
		{
			return static_cast<User*>(u)->OperandList() + u->NumOperands();
		}
		static uint32_t NumOperands(User const * u)
		{
			return u->NumOperands();
		}
	};

	#define DEFINE_TRANSPARENT_OPERAND_ACCESSORS(CLASS, VALUECLASS)			\
	public:																	\
		uint32_t NumOperands() const										\
		{																	\
			return OperandTraits<CLASS>::NumOperands(this);					\
		}																	\
		VALUECLASS* Operand(uint32_t idx) const								\
		{																	\
			BOOST_ASSERT_MSG(idx < OperandTraits<CLASS>::NumOperands(this), "Operand() out of range!"); \
			return cast_or_null<VALUECLASS>(OperandTraits<CLASS>::OpBegin(const_cast<CLASS*>(this))[idx].Get()); \
		}																	\
		void Operand(uint32_t idx, VALUECLASS* val)							\
		{																	\
			BOOST_ASSERT_MSG(idx < OperandTraits<CLASS>::NumOperands(this), "Operand() out of range!"); \
			OperandTraits<CLASS>::OpBegin(this)[idx].Set(val);				\
		}																	\
		const_op_iterator OpBegin() const									\
		{																	\
			return OperandTraits<CLASS>::OpBegin(const_cast<CLASS*>(this));	\
		}																	\
		op_iterator OpBegin()												\
		{																	\
			return OperandTraits<CLASS>::OpBegin(this);						\
		}																	\
		const_op_iterator OpEnd() const										\
		{																	\
			return OperandTraits<CLASS>::OpEnd(const_cast<CLASS*>(this));	\
		}																	\
		op_iterator OpEnd()													\
		{																	\
			return OperandTraits<CLASS>::OpEnd(this);						\
		}																	\
																			\
	protected:																\
		template <int INDEX>												\
		Use const & Op() const												\
		{																	\
			return this->OpFrom<INDEX>(this);								\
		}																	\
		template <int INDEX>												\
		Use& Op()															\
		{																	\
			return this->OpFrom<INDEX>(this);								\
		}
}

#endif		// _DILITHIUM_OPERAND_TRAITS_HPP
