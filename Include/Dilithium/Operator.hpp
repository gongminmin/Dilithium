/**
 * @file Operator.hpp
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

#ifndef _DILITHIUM_OPERATOR_HPP
#define _DILITHIUM_OPERATOR_HPP

#pragma once

#include <Dilithium/Constants.hpp>
#include <Dilithium/Instruction.hpp>
#include <Dilithium/User.hpp>
#include <Dilithium/Util.hpp>

#include <boost/assert.hpp>

namespace Dilithium
{
	class Operator : public User
	{
	public:
		static uint32_t Opcode(Value const * v)
		{
			Instruction const * inst = dyn_cast<Instruction>(v);
			if (inst)
			{
				return inst->Opcode();
			}
			else
			{
				ConstantExpr const * ce = dyn_cast<ConstantExpr>(v);
				if (ce)
				{
					return ce->Opcode();
				}
				else
				{
					return Instruction::UserOp1;
				}
			}
		}

		static bool classof(Instruction const * inst)
		{
			DILITHIUM_UNUSED(inst);
			return true;
		}
		static bool classof(ConstantExpr const * ce)
		{
			DILITHIUM_UNUSED(ce);
			return true;
		}
		static bool classof(Value const * val)
		{
			return isa<Instruction>(val) || isa<ConstantExpr>(val);
		}
	};

	template <typename SuperClass, uint32_t Opc>
	class ConcreteOperator : public SuperClass
	{
	public:
		static bool classof(Instruction const * inst)
		{
			return inst->Opcode() == Opc;
		}
		static bool classof(ConstantExpr const * ce)
		{
			return ce->Opcode() == Opc;
		}
		static bool classof(Value const * v)
		{
			return (isa<Instruction>(v) && classof(cast<Instruction>(v)))
				|| (isa<ConstantExpr>(v) && classof(cast<ConstantExpr>(v)));
		}
	};

	class GEPOperator : public ConcreteOperator<Operator, Instruction::GetElementPtr>
	{
	public:
		Value* PointerOperand()
		{
			return this->Operand(0);
		}

		bool HasAllZeroIndices() const;
	};
}

#endif		// _DILITHIUM_OPERATOR_HPP
