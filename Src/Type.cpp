/**
 * @file Type.cpp
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
#include <Dilithium/Type.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/DerivedType.hpp>
#include "LLVMContextImpl.hpp"

namespace Dilithium
{
	Type* Type::ScalarType()
	{
		VectorType* vec_type = dyn_cast<VectorType>(this);
		if (vec_type)
		{
			return vec_type->ElementType();
		}
		return this;
	}

	Type const * Type::ScalarType() const
	{
		VectorType const * vec_type = dyn_cast<VectorType>(this);
		if (vec_type)
		{
			return vec_type->ElementType();
		}
		return this;
	}

	/// IsIntegerType - Return true if this is an IntegerType of the specified width.
	bool Type::IsIntegerType(uint32_t bit_width) const
	{
		return this->IsIntegerType() && (cast<IntegerType>(this)->BitWidth() == bit_width);
	}

	bool Type::IsEmptyType() const
	{
		ArrayType const * arr_type = dyn_cast<ArrayType>(this);
		if (arr_type)
		{
			return (arr_type->NumElements() == 0) || arr_type->ElementType()->IsEmptyType();
		}
		else
		{
			StructType const * struct_type = dyn_cast<StructType>(this);
			if (struct_type)
			{
				uint32_t mum_elems = struct_type->NumElements();
				for (uint32_t i = 0; i < mum_elems; ++i)
				{
					if (!struct_type->ElementType(i)->IsEmptyType())
					{
						return false;
					}
				}
				return true;
			}
			return false;
		}
	}

	bool Type::IsSized() const
	{
		// If it's a primitive, it is always sized.
		if ((this->GetTypeId() == TID_Integer) || this->IsFloatingPointType() || (this->GetTypeId() == TID_Pointer)
			|| (this->GetTypeId() == TID_X86Mmx))
		{
			return true;
		}
		// If it is not something that can have a size (e.g. a function or label),
		// it doesn't have a size.
		if ((this->GetTypeId() != TID_Struct) && (this->GetTypeId() != TID_Array) && (this->GetTypeId() != TID_Vector))
		{
			return false;
		}
		// Otherwise we have to try harder to decide.
		return this->IsSizedDerivedType();
	}

	uint32_t Type::PrimitiveSizeInBits() const
	{
		switch (this->GetTypeId())
		{
		case Type::TID_Half:
			return 16;
		case Type::TID_Float:
			return 32;
		case Type::TID_Double:
			return 64;
		case Type::TID_X86Fp80:
			return 80;
		case Type::TID_Fp128:
			return 128;
		case Type::TID_PpcFp128:
			return 128;
		case Type::TID_X86Mmx:
			return 64;
		case Type::TID_Integer:
			return cast<IntegerType>(this)->BitWidth();
		case Type::TID_Vector:
			return cast<VectorType>(this)->BitWidth();

		default:
			return 0;
		}
	}

	uint32_t Type::ScalarSizeInBits() const
	{
		return this->ScalarType()->PrimitiveSizeInBits();
	}

	int Type::FpMantissaWidth() const
	{
		VectorType const * vec_type = dyn_cast<VectorType>(this);
		if (vec_type)
		{
			return vec_type->ElementType()->FpMantissaWidth();
		}
		else
		{
			BOOST_ASSERT_MSG(this->IsFloatingPointType(), "Not a floating point type!");
			switch (this->GetTypeId())
			{
			case TID_Half:
				return 11;
			case TID_Float:
				return 24;
			case TID_Double:
				return 53;
			case TID_X86Fp80:
				return 64;
			case TID_Fp128:
				return 113;

			case TID_PpcFp128:
				DILITHIUM_NOT_IMPLEMENTED;

			default:
				DILITHIUM_UNREACHABLE("Unknown fp type");
			}
		}
	}

	/// IsSizedDerivedType - Derived types like structures and arrays are sized
	/// iff all of the members of the type are sized as well.  Since asking for
	/// their size is relatively uncommon, move this operation out of line.
	bool Type::IsSizedDerivedType() const
	{
		Type const * final_type;
		ArrayType const * arr_type = dyn_cast<ArrayType>(this);
		if (arr_type)
		{
			final_type = arr_type->ElementType();
		}
		else
		{
			VectorType const * vec_type = dyn_cast<VectorType>(this);
			if (vec_type)
			{
				final_type = vec_type->ElementType();
			}
			else
			{
				final_type = cast<StructType>(this);
			}
		}
		return final_type->IsSized();
	}

	uint32_t Type::IntegerBitWidth() const
	{
		return cast<IntegerType>(this)->BitWidth();
	}

	bool Type::IsFunctionVarArg() const
	{
		return cast<FunctionType>(this)->IsVarArg();
	}

	Type* Type::FunctionParamType(uint32_t i) const
	{
		return cast<FunctionType>(this)->ParamType(i);
	}

	uint32_t Type::FunctionNumParams() const
	{
		return cast<FunctionType>(this)->NumParams();
	}

	std::string_view Type::StructName() const
	{
		return cast<StructType>(this)->Name();
	}

	uint32_t Type::StructNumElements() const
	{
		return cast<StructType>(this)->NumElements();
	}

	Type* Type::StructElementType(uint32_t i) const
	{
		return cast<StructType>(this)->ElementType(i);
	}

	Type* Type::SequentialElementType() const
	{
		return cast<SequentialType>(this)->ElementType();
	}

	uint64_t Type::ArrayNumElements() const
	{
		return cast<ArrayType>(this)->NumElements();
	}

	uint32_t Type::VectorNumElements() const
	{
		return cast<VectorType>(this)->NumElements();
	}

	uint32_t Type::PointerAddressSpace() const
	{
		return cast<PointerType>(this->ScalarType())->AddressSpace();
	}


	//===----------------------------------------------------------------------===//
	//                          Primitive 'Type' data
	//===----------------------------------------------------------------------===//

	Type* Type::VoidType(LLVMContext& context)
	{
		return &context.Impl().void_ty;
	}
	Type* Type::LabelType(LLVMContext& context)
	{
		return &context.Impl().label_ty;
	}
	Type* Type::HalfType(LLVMContext& context)
	{
		return &context.Impl().half_ty;
	}
	Type* Type::FloatType(LLVMContext& context)
	{
		return &context.Impl().float_ty;
	}
	Type* Type::DoubleType(LLVMContext& context)
	{
		return &context.Impl().double_ty;
	}
	Type* Type::MetadataType(LLVMContext& context)
	{
		return &context.Impl().metadata_ty;
	}

	IntegerType* Type::Int1Type(LLVMContext& context)
	{
		return &context.Impl().int1_ty;
	}
	IntegerType* Type::Int8Type(LLVMContext& context)
	{
		return &context.Impl().int8_ty;
	}
	IntegerType* Type::Int16Type(LLVMContext& context)
	{
		return &context.Impl().int16_ty;
	}
	IntegerType* Type::Int32Type(LLVMContext& context)
	{
		return &context.Impl().int32_ty;
	}
	IntegerType* Type::Int64Type(LLVMContext& context)
	{
		return &context.Impl().int64_ty;
	}

	IntegerType* Type::IntNType(LLVMContext& context, uint32_t n)
	{
		return IntegerType::Get(context, n);
	}

	PointerType* Type::HalfPtrType(LLVMContext& context, uint32_t as)
	{
		return Type::HalfType(context)->PointerTo(as);
	}

	PointerType* Type::FloatPtrType(LLVMContext& context, uint32_t as)
	{
		return Type::FloatType(context)->PointerTo(as);
	}

	PointerType* Type::DoublePtrType(LLVMContext& context, uint32_t as)
	{
		return Type::DoubleType(context)->PointerTo(as);
	}

	PointerType* Type::IntNPtrType(LLVMContext& context, uint32_t n, uint32_t as)
	{
		return Type::IntNType(context, n)->PointerTo(as);
	}

	PointerType* Type::Int1PtrType(LLVMContext& context, uint32_t as)
	{
		return Type::Int1Type(context)->PointerTo(as);
	}

	PointerType* Type::Int8PtrType(LLVMContext& context, uint32_t as)
	{
		return Type::Int8Type(context)->PointerTo(as);
	}

	PointerType* Type::Int16PtrType(LLVMContext& context, uint32_t as)
	{
		return Type::Int16Type(context)->PointerTo(as);
	}

	PointerType* Type::Int32PtrType(LLVMContext& context, uint32_t as)
	{
		return Type::Int32Type(context)->PointerTo(as);
	}

	PointerType* Type::Int64PtrType(LLVMContext& context, uint32_t as)
	{
		return Type::Int64Type(context)->PointerTo(as);
	}

	PointerType* Type::PointerTo(uint32_t addr_space)
	{
		return PointerType::Get(this, addr_space);
	}

	void Type::Print(std::ostream& os) const
	{
		DILITHIUM_UNUSED(os);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
