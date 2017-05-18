/**
 * @file Constant.cpp
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
#include <Dilithium/Constants.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/MPFloat.hpp>

namespace Dilithium 
{
	Constant::Constant(Type* ty, ValueTy vty, uint32_t num_ops, uint32_t num_uses)
		: User(ty, vty, num_ops, num_uses)
	{
	}

	void Constant::HandleOperandChange(Value* from, Value* to, Use* u)
	{
		DILITHIUM_UNUSED(from);
		DILITHIUM_UNUSED(to);
		DILITHIUM_UNUSED(u);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Constant* Constant::NullValue(Type* ty)
	{
		switch (ty->GetTypeId())
		{
		case Type::TID_Integer:
			return ConstantInt::Get(ty, 0);
		case Type::TID_Half:
			return ConstantFP::Get(ty->Context(), MPFloat::Zero(MPFloat::IEEEHalf));
		case Type::TID_Float:
			return ConstantFP::Get(ty->Context(), MPFloat::Zero(MPFloat::IEEESingle));
		case Type::TID_Double:
			return ConstantFP::Get(ty->Context(), MPFloat::Zero(MPFloat::IEEEDouble));
		case Type::TID_Pointer:
			return ConstantPointerNull::Get(cast<PointerType>(ty));
		case Type::TID_Struct:
		case Type::TID_Array:
		case Type::TID_Vector:
			return ConstantAggregateZero::Get(ty);

		default:
			// Function, Label, or Opaque type?
			DILITHIUM_UNREACHABLE("Cannot create a null constant of that type!");
		}
	}
}
