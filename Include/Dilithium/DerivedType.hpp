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
	struct LLVMContextImpl;
	class Value;

	class IntegerType : public Type
	{
		friend struct LLVMContextImpl;

	public:
		enum
		{
			MIN_INT_BITS = 1,
			MAX_INT_BITS = (1 << 23) - 1
		};

	public:
		IntegerType(LLVMContext& context, uint32_t num_bits)
			: Type(context, TID_Integer)
		{
			this->SubclassData(num_bits);
		}

		static IntegerType* Get(LLVMContext& context, uint32_t num_bits);

		uint32_t BitWidth() const
		{
			return this->SubclassData();
		}
		uint64_t BitMask() const
		{
			return ~0ULL >> (64 - this->BitWidth());
		}
		uint64_t SignBit() const
		{
			return 1ULL << (this->BitWidth() - 1);
		}

		static bool classof(Type const * ty)
		{
			return ty->GetTypeId() == TID_Integer;
		}

		// DILITHIUM_NOT_IMPLEMENTED
	};

	class FunctionType : boost::noncopyable, public Type
	{
	public:
		FunctionType(Type* return_type, ArrayRef<Type*> params, bool is_var_args);

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

		typedef Type::subtype_iterator param_iterator;
		param_iterator ParamBegin() const
		{
			return contained_types_.data() + 1;
		}
		param_iterator ParamEnd() const
		{
			return contained_types_.data() + contained_types_.size();
		}
		ArrayRef<Type*> Params() const
		{
			return ArrayRef<Type*>(this->ParamBegin(), this->ParamEnd());
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
	};

	class CompositeType : public Type
	{
	public:
		Type* TypeAtIndex(Value const * val);
		Type* TypeAtIndex(uint32_t idx);
		bool IndexValid(Value const * val) const;
		bool IndexValid(uint32_t idx) const;

		static bool classof(Type const * ty)
		{
			return (ty->GetTypeId() == TID_Array) || (ty->GetTypeId() == TID_Struct)
				|| (ty->GetTypeId() == TID_Pointer) || (ty->GetTypeId() == TID_Vector);
		}

	protected:
		explicit CompositeType(LLVMContext& context, TypeId tid)
			: Type(context, tid)
		{
		}
	};

	class StructType : boost::noncopyable, public CompositeType
	{
		enum
		{
			SCDB_HasBody = 1,
			SCDB_Packed = 2,
			SCDB_IsLiteral = 4,
			SCDB_IsSized = 8
		};

	public:
		StructType(LLVMContext& context)
			: CompositeType(context, TID_Struct)
		{
		}

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

		bool IsPacked() const
		{
			return (this->SubclassData() & SCDB_Packed) != 0;
		}
		bool IsLiteral() const
		{
			return (this->SubclassData() & SCDB_IsLiteral) != 0;
		}
		bool IsOpaque() const
		{
			return (this->SubclassData() & SCDB_HasBody) == 0;
		}
		bool IsSized() const;

		bool HasName() const
		{
			return !symbol_table_name_.empty();
		}
		std::string_view Name() const;
		void Name(std::string_view name);

		void Body(ArrayRef<Type*> elements, bool is_packed = false);
		void Body(Type* type, ...);

		static bool IsValidElementType(Type* elem_type);

		typedef Type::subtype_iterator element_iterator;
		element_iterator ElementBegin() const
		{
			return contained_types_.data();
		}
		element_iterator ElementEnd() const
		{
			return contained_types_.data() + contained_types_.size();
		}
		ArrayRef<Type*> const elements() const
		{
			return ArrayRef<Type*>(this->ElementBegin(), this->ElementEnd());
		}

		bool IsLayoutIdentical(StructType* rhs) const;

		uint32_t NumElements() const
		{
			return static_cast<uint32_t>(contained_types_.size());
		}
		Type* ElementType(uint32_t i) const
		{
			return contained_types_[i];
		}

		static bool classof(Type const * ty)
		{
			return ty->GetTypeId() == TID_Struct;
		}

	private:
		std::string symbol_table_name_;
	};

	class SequentialType : boost::noncopyable, public CompositeType
	{
	public:
		Type* ElementType() const
		{
			return contained_types_[0];
		}

		static bool classof(Type const * ty)
		{
			return (ty->GetTypeId() == TID_Array) || (ty->GetTypeId() == TID_Pointer) || (ty->GetTypeId() == TID_Vector);
		}

	protected:
		SequentialType(TypeId tid, Type* elem_type)
			: CompositeType(elem_type->Context(), tid)
		{
			contained_types_.push_back(elem_type);
		}
	};
	
	class ArrayType : public SequentialType
	{
	public:
		ArrayType(Type* elem_type, uint64_t num_elements);

		static ArrayType* Get(Type* elem_type, uint64_t num_elements);

		static bool IsValidElementType(Type* elem_type);

		uint64_t NumElements() const
		{
			return num_elements_;
		}

		static bool classof(Type const * ty)
		{
			return ty->GetTypeId() == TID_Array;
		}

	private:
		uint64_t num_elements_;
	};
	
	class VectorType : public SequentialType
	{
	public:
		VectorType(Type* elem_type, uint32_t num_elements);

		static VectorType* Get(Type* elem_type, uint32_t num_elements);

		static VectorType* Integer(VectorType* vec_type)
		{
			uint32_t elem_bits = vec_type->ElementType()->PrimitiveSizeInBits();
			BOOST_ASSERT_MSG(elem_bits, "Element size must be of a non-zero size");
			Type* elem_type = IntegerType::Get(vec_type->Context(), elem_bits);
			return VectorType::Get(elem_type, vec_type->NumElements());
		}
		static VectorType* ExtendedElementVectorType(VectorType* vec_type)
		{
			uint32_t elem_bits = vec_type->ElementType()->PrimitiveSizeInBits();
			Type* elem_type = IntegerType::Get(vec_type->Context(), elem_bits * 2);
			return VectorType::Get(elem_type, vec_type->NumElements());
		}
		static VectorType* TruncatedElementVectorType(VectorType* vec_type)
		{
			uint32_t elem_bits = vec_type->ElementType()->PrimitiveSizeInBits();
			BOOST_ASSERT_MSG((elem_bits & 1) == 0, "Cannot truncate vector element with odd bit-width");
			Type* elem_type = IntegerType::Get(vec_type->Context(), elem_bits / 2);
			return VectorType::Get(elem_type, vec_type->NumElements());
		}
		static VectorType* HalfElementsVectorType(VectorType* vec_type)
		{
			uint32_t num_elements = vec_type->NumElements();
			BOOST_ASSERT_MSG((num_elements & 1) == 0, "Cannot halve vector with odd number of elements.");
			return VectorType::Get(vec_type->ElementType(), num_elements / 2);
		}
		static VectorType* DoubleElementsVectorType(VectorType* vec_type)
		{
			uint32_t num_elements = vec_type->NumElements();
			return VectorType::Get(vec_type->ElementType(), num_elements * 2);
		}

		static bool IsValidElementType(Type* elem_type);

		uint32_t NumElements() const
		{
			return num_elements_;
		}
		uint32_t BitWidth() const
		{
			return num_elements_ * this->ElementType()->PrimitiveSizeInBits();
		}

		static bool classof(Type const * ty)
		{
			return ty->GetTypeId() == TID_Vector;
		}

	private:
		uint32_t num_elements_;
	};

	class PointerType : public SequentialType
	{
	public:
		PointerType(Type* elem_type, uint32_t address_space);

		static PointerType* Get(Type* elem_type, uint32_t address_space);

		static PointerType* GetUnqual(Type* elem_type)
		{
			return PointerType::Get(elem_type, 0);
		}

		static bool IsValidElementType(Type* elem_type);
		static bool IsLoadableOrStorableType(Type* elem_type);

		uint32_t AddressSpace() const
		{
			return this->SubclassData();
		}

		static bool classof(Type const * v)
		{
			return v->GetTypeId() == TID_Pointer;
		}
		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_DERIVED_TYPE_HPP
