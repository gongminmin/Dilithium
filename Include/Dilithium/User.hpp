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
#include <Dilithium/OperandTraits.hpp>

#include <vector>

#include <boost/range/iterator_range.hpp>

namespace Dilithium
{
	template <typename T>
	struct OperandTraits;

	class User : public Value
	{
	public:
		typedef Use* op_iterator;
		typedef Use const * const_op_iterator;
		typedef boost::iterator_range<op_iterator> op_range;
		typedef boost::iterator_range<const_op_iterator> const_op_range;

	public:
		~User() override;

		Use* OperandList()
		{
			return num_user_operands_ > 0 ? operands_.data() : nullptr;
		}
		Use const * OperandList() const
		{
			return const_cast<User*>(this)->OperandList();
		}

		Value* Operand(uint32_t idx) const;
		void Operand(uint32_t idx, Value* val);

		Use const & OperandUse(uint32_t idx) const;
		Use& getOperandUse(uint32_t idx);

		uint32_t NumOperands() const
		{
			return num_user_operands_;
		}

		void GlobalVariableOrFunctionNumOperands(uint32_t num_ops)
		{
			BOOST_ASSERT_MSG(num_ops <= 1, "GlobalVariable or Function can only have 0 or 1 operands");
			num_user_operands_ = num_ops;
		}

		op_iterator OpBegin()
		{
			return this->OperandList();
		}
		const_op_iterator OpBegin() const
		{
			return this->OperandList();
		}
		op_iterator OpEnd()
		{
			return this->OperandList() + num_user_operands_;
		}
		const_op_iterator OpEnd() const
		{
			return this->OperandList() + num_user_operands_;
		}

		const_op_range Operands() const
		{
			return const_op_range(this->OpBegin(), this->OpEnd());
		}
		op_range Operands()
		{
			return op_range(this->OpBegin(), this->OpEnd());
		}

		void DropAllReferences();

	protected:
		User(Type* ty, uint32_t vty, uint32_t num_ops, uint32_t num_uses);

		template <int INDEX, typename U>
		static Use& OpFrom(U const * that)
		{
			auto non_const_that = const_cast<U*>(that);
			auto uses = INDEX < 0 ? OperandTraits<U>::OpEnd(non_const_that) : OperandTraits<U>::OpBegin(non_const_that);
			return uses[INDEX];
		}
		template <int INDEX>
		Use& Op()
		{
			return this->OpFrom<INDEX>(this);
		}
		template <int INDEX>
		Use const & Op() const
		{
			return this->OpFrom<INDEX>(this);
		}

	protected:
		std::vector<Use> operands_;
		std::vector<BasicBlock*> phi_bbs_;
		User* user_;
		bool ref_;

		// DILITHIUM_NOT_IMPLEMENTED
	};

	template <>
	struct simplify_type<User::op_iterator>
	{
		typedef Value* SimpleType;
		static SimpleType SimplifiedValue(User::op_iterator& val)
		{
			return val->Get();
		}
	};
	template <>
	struct simplify_type<User::const_op_iterator>
	{
		typedef Value /*const*/ * SimpleType;
		static SimpleType SimplifiedValue(User::const_op_iterator& val)
		{
			return val->Get();
		}
	};
}

#endif		// _DILITHIUM_USER_HPP
