/**
 * @file User.cpp
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
#include <Dilithium/User.hpp>

namespace Dilithium 
{
	User::User(Type* ty, uint32_t vty, uint32_t num_ops, uint32_t num_uses)
		: Value(ty, vty)
	{
		BOOST_ASSERT_MSG(num_ops < (1U << NUM_USER_OPERANDS_BITS), "Too many operands");
		num_user_operands_ = num_ops;
		// If we have hung off uses, then the operand list should initially be
		// null.
		BOOST_ASSERT_MSG(!this->OperandList(), "Error in initializing hung off uses for User");

		operands_.resize(num_uses);
		if (num_uses > 0)
		{
			Use::InitTags(operands_.data(), operands_.data() + num_uses);
		}
	}

	User::~User()
	{
	}

	Value* User::Operand(uint32_t idx) const
	{
		BOOST_ASSERT_MSG(idx < num_user_operands_, "Operand() out of range!");
		return this->OperandList()[idx];
	}

	void User::Operand(uint32_t idx, Value* val)
	{
		BOOST_ASSERT_MSG(idx < num_user_operands_, "setOperand() out of range!");
		BOOST_ASSERT_MSG(!isa<Constant>((const Value*)this) || isa<GlobalValue>((const Value*)this),
			"Cannot mutate a constant with setOperand!");
		this->OperandList()[idx].Set(val);
	}

	Use const & User::OperandUse(uint32_t idx) const
	{
		BOOST_ASSERT_MSG(idx < num_user_operands_, "getOperandUse() out of range!");
		return this->OperandList()[idx];
	}

	Use& User::OperandUse(uint32_t idx)
	{
		BOOST_ASSERT_MSG(idx < num_user_operands_, "getOperandUse() out of range!");
		return this->OperandList()[idx];
	}

	void User::DropAllReferences()
	{
		for (auto& u : this->Operands())
		{
			u.Set(nullptr);
		}
	}
}
