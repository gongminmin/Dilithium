/**
 * @file DerivedType.hpp
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

#ifndef _DILITHIUM_DERIVED_TYPE_HPP
#define _DILITHIUM_DERIVED_TYPE_HPP

#pragma once

#include <Dilithium/Type.hpp>

#include <boost/core/noncopyable.hpp>

namespace Dilithium
{
	class LLVMContext;

	class IntegerType : public Type
	{
	public:
		enum
		{
			MIN_INT_BITS = 1,
			MAX_INT_BITS = (1 << 23) - 1
		};

		static IntegerType* Get(LLVMContext& context, uint32_t num_bits);
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class FunctionType : boost::noncopyable, public Type
	{
	public:
		static FunctionType* Get(Type* return_type, ArrayRef<Type*> params, bool is_var_args);
		static FunctionType* Get(Type* return_type, bool is_var_args);

		static bool IsValidReturnType(Type* return_type);
		static bool IsValidArgumentType(Type* arg_type);

		bool IsVarArg() const
		{
			return this->SubclassData() != 0;
		}
		Type* ReturnType() const
		{
			return contained_types_[0];
		}

		Type* ParamType(uint32_t i) const
		{
			return contained_types_[i + 1];
		}
		uint32_t NumParams() const
		{
			return static_cast<uint32_t>(contained_types_.size() - 1);
		}

		static bool classof(Type const * ty)
		{
			return ty->GetTypeId() == TID_Function;
		}

		// DILITHIUM_NOT_IMPLEMENTED
	};

	class CompositeType : public Type
	{
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class StructType : boost::noncopyable, public CompositeType
	{
	public:
		static StructType* Create(LLVMContext& context, std::string_view name);
		static StructType* Create(LLVMContext& context);

		static StructType* Create(ArrayRef<Type*> elements, std::string_view name, bool is_packed = false);
		static StructType* Create(ArrayRef<Type*> elements);
		static StructType* Create(LLVMContext& context, ArrayRef<Type*> elements, std::string_view name, bool is_packed = false);
		static StructType* Create(LLVMContext& context, ArrayRef<Type*> elements);
		static StructType* Create(std::string_view name, Type* type, ...);

		static StructType* Get(LLVMContext& context, ArrayRef<Type*> elements, bool is_packed = false);
		static StructType* Get(LLVMContext& context, bool is_packed = false);
		static StructType* Get(Type* type, ...);

		static bool IsValidElementType(Type* elem_type);

		// DILITHIUM_NOT_IMPLEMENTED
	};

	class SequentialType : boost::noncopyable, public CompositeType
	{
	public:
		Type* ElementType() const;
		// DILITHIUM_NOT_IMPLEMENTED
	};
	
	class ArrayType : public SequentialType
	{
	public:
		static ArrayType* Get(Type* elem_type, uint64_t num_elements);

		static bool IsValidElementType(Type* elem_type);
		// DILITHIUM_NOT_IMPLEMENTED
	};
	
	class VectorType : public SequentialType
	{
	public:
		static VectorType* Get(Type* elem_type, uint32_t num_elements);

		static bool IsValidElementType(Type* elem_type);
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class PointerType : public SequentialType
	{
	public:
		static PointerType* Get(Type* elem_type, uint32_t address_space);

		static bool IsValidElementType(Type* elem_type);

		static bool classof(Type const * v)
		{
			return v->GetTypeId() == TID_Pointer;
		}
		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_DERIVED_TYPE_HPP
