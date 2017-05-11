/**
 * @file DerivedType.cpp
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
#include <Dilithium/DerivedType.hpp>

namespace Dilithium 
{
	IntegerType::IntegerType(LLVMContext& context, uint32_t num_bits)
		: Type(context, TID_Integer)
	{
		this->SubclassData(num_bits);
	}

	IntegerType* IntegerType::Get(LLVMContext& context, uint32_t num_bits)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(num_bits);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	FunctionType::FunctionType(Type* return_type, ArrayRef<Type*> params, bool is_var_args)
		: Type(return_type->Context(), TID_Function)
	{
		contained_types_.resize(params.size() + 1);
		BOOST_ASSERT_MSG(this->IsValidReturnType(return_type), "invalid return type for function");
		this->SubclassData(is_var_args);

		contained_types_[0] = return_type;

		for (uint32_t i = 0, e = static_cast<uint32_t>(params.size()); i != e; ++ i)
		{
			BOOST_ASSERT_MSG(this->IsValidArgumentType(params[i]), "Not a valid type for function argument!");
			contained_types_[i + 1] = params[i];
		}
	}

	FunctionType* FunctionType::Get(Type* return_type, ArrayRef<Type*> params, bool is_var_args)
	{
		DILITHIUM_UNUSED(return_type);
		DILITHIUM_UNUSED(params);
		DILITHIUM_UNUSED(is_var_args);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	FunctionType* FunctionType::Get(Type* return_type, bool is_var_args)
	{
		DILITHIUM_UNUSED(return_type);
		DILITHIUM_UNUSED(is_var_args);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool FunctionType::IsValidReturnType(Type* return_type)
	{
		DILITHIUM_UNUSED(return_type);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool FunctionType::IsValidArgumentType(Type* arg_type)
	{
		DILITHIUM_UNUSED(arg_type);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	CompositeType::CompositeType(LLVMContext& context, TypeId tid)
		: Type(context, tid)
	{
	}

	Type* CompositeType::TypeAtIndex(Value const * val)
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Type* CompositeType::TypeAtIndex(uint32_t idx)
	{
		DILITHIUM_UNUSED(idx);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool CompositeType::IndexValid(Value const * val) const
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool CompositeType::IndexValid(uint32_t idx) const
	{
		DILITHIUM_UNUSED(idx);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	StructType::StructType(LLVMContext& context)
		: CompositeType(context, TID_Struct)
	{
	}

	StructType* StructType::Create(LLVMContext& context, std::string_view name)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(name);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Create(LLVMContext& context)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Create(ArrayRef<Type*> elements, std::string_view name, bool is_packed)
	{
		DILITHIUM_UNUSED(elements);
		DILITHIUM_UNUSED(name);
		DILITHIUM_UNUSED(is_packed);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Create(ArrayRef<Type*> elements)
	{
		DILITHIUM_UNUSED(elements);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Create(LLVMContext& context, ArrayRef<Type*> elements, std::string_view name, bool is_packed)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(elements);
		DILITHIUM_UNUSED(name);
		DILITHIUM_UNUSED(is_packed);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Create(LLVMContext& context, ArrayRef<Type*> elements)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(elements);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Create(std::string_view name, Type* type, ...)
	{
		DILITHIUM_UNUSED(name);
		DILITHIUM_UNUSED(type);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Get(LLVMContext& context, ArrayRef<Type*> elements, bool is_packed)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(elements);
		DILITHIUM_UNUSED(is_packed);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Get(LLVMContext& context, bool is_packed)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(is_packed);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	StructType* StructType::Get(Type* type, ...)
	{
		DILITHIUM_UNUSED(type);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool StructType::IsSized() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	std::string_view StructType::Name() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void StructType::Name(std::string_view name)
	{
		DILITHIUM_UNUSED(name);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void StructType::Body(ArrayRef<Type*> elements, bool is_packed)
	{
		DILITHIUM_UNUSED(elements);
		DILITHIUM_UNUSED(is_packed);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void StructType::Body(Type* type, ...)
	{
		DILITHIUM_UNUSED(type);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool StructType::IsValidElementType(Type* elem_type)
	{
		DILITHIUM_UNUSED(elem_type);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool StructType::IsLayoutIdentical(StructType* rhs) const
	{
		DILITHIUM_UNUSED(rhs);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	SequentialType::SequentialType(TypeId tid, Type* elem_type)
		: CompositeType(elem_type->Context(), tid)
	{
		contained_types_.push_back(elem_type);
	}


	ArrayType::ArrayType(Type* elem_type, uint64_t num_elements)
		: SequentialType(TID_Array, elem_type),
			num_elements_(num_elements)
	{
	}

	ArrayType* ArrayType::Get(Type* elem_type, uint64_t num_elements)
	{
		DILITHIUM_UNUSED(elem_type);
		DILITHIUM_UNUSED(num_elements);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool ArrayType::IsValidElementType(Type* elem_type)
	{
		DILITHIUM_UNUSED(elem_type);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	VectorType::VectorType(Type* elem_type, uint32_t num_elements)
		: SequentialType(TID_Vector, elem_type),
			num_elements_(num_elements)
	{
	}

	VectorType* VectorType::Get(Type* elem_type, uint32_t num_elements)
	{
		DILITHIUM_UNUSED(elem_type);
		DILITHIUM_UNUSED(num_elements);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	VectorType* VectorType::Integer(VectorType* vec_type)
	{
		uint32_t elem_bits = vec_type->ElementType()->PrimitiveSizeInBits();
		BOOST_ASSERT_MSG(elem_bits, "Element size must be of a non-zero size");
		Type* elem_type = IntegerType::Get(vec_type->Context(), elem_bits);
		return VectorType::Get(elem_type, vec_type->NumElements());
	}

	VectorType* VectorType::ExtendedElementVectorType(VectorType* vec_type)
	{
		uint32_t elem_bits = vec_type->ElementType()->PrimitiveSizeInBits();
		Type* elem_type = IntegerType::Get(vec_type->Context(), elem_bits * 2);
		return VectorType::Get(elem_type, vec_type->NumElements());
	}

	VectorType* VectorType::TruncatedElementVectorType(VectorType* vec_type)
	{
		uint32_t elem_bits = vec_type->ElementType()->PrimitiveSizeInBits();
		BOOST_ASSERT_MSG((elem_bits & 1) == 0, "Cannot truncate vector element with odd bit-width");
		Type* elem_type = IntegerType::Get(vec_type->Context(), elem_bits / 2);
		return VectorType::Get(elem_type, vec_type->NumElements());
	}
	
	VectorType* VectorType::HalfElementsVectorType(VectorType* vec_type)
	{
		uint32_t num_elements = vec_type->NumElements();
		BOOST_ASSERT_MSG((num_elements & 1) == 0, "Cannot halve vector with odd number of elements.");
		return VectorType::Get(vec_type->ElementType(), num_elements / 2);
	}
	
	VectorType* VectorType::DoubleElementsVectorType(VectorType* vec_type)
	{
		uint32_t num_elements = vec_type->NumElements();
		return VectorType::Get(vec_type->ElementType(), num_elements * 2);
	}

	bool VectorType::IsValidElementType(Type* elem_type)
	{
		DILITHIUM_UNUSED(elem_type);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	PointerType::PointerType(Type* elem_type, uint32_t address_space)
		: SequentialType(TID_Pointer, elem_type)
	{
		this->SubclassData(address_space);
	}

	PointerType* PointerType::Get(Type* elem_type, uint32_t address_space)
	{
		DILITHIUM_UNUSED(elem_type);
		DILITHIUM_UNUSED(address_space);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool PointerType::IsValidElementType(Type* elem_type)
	{
		DILITHIUM_UNUSED(elem_type);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool PointerType::IsLoadableOrStorableType(Type* elem_type)
	{
		DILITHIUM_UNUSED(elem_type);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
