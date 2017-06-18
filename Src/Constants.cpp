/**
 * @file Constants.cpp
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
#include <Dilithium/Casting.hpp>
#include <Dilithium/Constants.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/LLVMContext.hpp>
#include "LLVMContextImpl.hpp"

namespace Dilithium 
{
	ConstantInt::ConstantInt(IntegerType* ty, MPInt const & v)
		: Constant(ty, ConstantIntVal, 0, 0),
			val_(v)
	{
		BOOST_ASSERT_MSG(v.BitWidth() == ty->BitWidth(), "Invalid constant for type");
	}

	Constant* ConstantInt::Get(Type* ty, uint64_t v, bool is_signed)
	{
		Constant* ret = Get(cast<IntegerType>(ty->ScalarType()), v, is_signed);
		VectorType* vty = dyn_cast<VectorType>(ty);
		if (vty)
		{
			return ConstantVector::GetSplat(vty->NumElements(), ret);
		}
		else
		{
			return ret;
		}
	}

	ConstantInt* ConstantInt::Get(IntegerType* ty, uint64_t v, bool is_signed)
	{
		return ConstantInt::Get(ty->Context(), MPInt(ty->BitWidth(), v, is_signed));
	}

	ConstantInt* ConstantInt::Get(LLVMContext& context, MPInt const & v)
	{
		auto& impl = context.Impl();
		auto& slot = impl.int_constants[v];
		if (!slot)
		{
			IntegerType* ity = IntegerType::Get(context, v.BitWidth());
			slot = new ConstantInt(ity, v);
		}
		BOOST_ASSERT(slot->GetType() == IntegerType::Get(context, v.BitWidth()));
		return slot;
	}

	ConstantInt* ConstantInt::Get(IntegerType* ty, std::string_view str, uint8_t radix)
	{
		return ConstantInt::Get(ty->Context(), MPInt(ty->BitWidth(), str, radix));
	}

	Constant* ConstantInt::Get(Type* ty, MPInt const & v)
	{
		ConstantInt* ret = ConstantInt::Get(ty->Context(), v);
		BOOST_ASSERT_MSG(ret->GetType() == ty->ScalarType(), "ConstantInt type doesn't match the type implied by its value!");

		VectorType* vty = dyn_cast<VectorType>(ty);
		if (vty)
		{
			return ConstantVector::GetSplat(vty->NumElements(), ret);
		}
		else
		{
			return ret;
		}
	}


	Constant* ConstantFP::Get(Type* ty, double v)
	{
		DILITHIUM_UNUSED(ty);
		DILITHIUM_UNUSED(v);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Constant* ConstantFP::Get(Type* ty, std::string_view str)
	{
		DILITHIUM_UNUSED(ty);
		DILITHIUM_UNUSED(str);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	ConstantFP* ConstantFP::Get(LLVMContext& context, MPFloat const & v)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(v);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	ConstantAggregateZero* ConstantAggregateZero::Get(Type* ty)
	{
		DILITHIUM_UNUSED(ty);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	Constant* ConstantVector::Get(ArrayRef<Constant*> elems)
	{
		DILITHIUM_UNUSED(elems);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Constant* ConstantVector::GetSplat(uint32_t num_elem, Constant* elem)
	{
		DILITHIUM_UNUSED(num_elem);
		DILITHIUM_UNUSED(elem);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Constant* ConstantVector::GetImpl(ArrayRef<Constant*> v)
	{
		DILITHIUM_UNUSED(v);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	ConstantPointerNull* ConstantPointerNull::Get(PointerType* t)
	{
		DILITHIUM_UNUSED(t);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	SequentialType* ConstantDataSequential::GetType() const
	{
		return cast<SequentialType>(Value::GetType());
	}

	Type* ConstantDataSequential::GetElementType() const
	{
		return this->GetType()->ElementType();
	}

	uint32_t ConstantDataSequential::NumElements() const
	{
		ArrayType* arr = dyn_cast<ArrayType>(this->GetType());
		if (arr)
		{
			return static_cast<uint32_t>(arr->NumElements());
		}
		else
		{
			return this->GetType()->VectorNumElements();
		}
	}

	uint32_t ConstantDataSequential::GetElementByteSize() const
	{
		return this->GetElementType()->PrimitiveSizeInBits() / 8;
	}

	std::string_view ConstantDataSequential::GetRawDataValues() const
	{
		return std::string_view(data_elements_, this->NumElements() * this->GetElementByteSize());
	}


	UndefValue::UndefValue(Type* ty)
		: Constant(ty, UndefValueVal, 0, 0)
	{
	}

	UndefValue* UndefValue::Get(Type* ty)
	{
		auto& entry = ty->Context().Impl().uv_constants[ty];
		if (!entry)
		{
			entry = new UndefValue(ty);
		}
		return entry;
	}
}
