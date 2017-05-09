/**
 * @file Type.hpp
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

#ifndef _DILITHIUM_TYPE_HPP
#define _DILITHIUM_TYPE_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/ArrayRef.hpp>
#include <Dilithium/Casting.hpp>
#include <Dilithium/Hashing.hpp>

namespace Dilithium
{
	class IntegerType;
	class LLVMContext;
	struct LLVMContextImpl;
	class PointerType;

	class Type
	{
		friend struct LLVMContextImpl;

	public:
		enum TypeId
		{
			// PrimitiveTypes - make sure LastPrimitive stays up to date.
			TID_Void = 0,    ///<  0: type with no size
			TID_Half,        ///<  1: 16-bit floating point type
			TID_Float,       ///<  2: 32-bit floating point type
			TID_Double,      ///<  3: 64-bit floating point type
			TID_X86Fp80,     ///<  4: 80-bit floating point type (X87)
			TID_Fp128,       ///<  5: 128-bit floating point type (112-bit mantissa)
			TID_PpcFp128,    ///<  6: 128-bit floating point type (two 64-bits, PowerPC)
			TID_Label,       ///<  7: Labels
			TID_Metadata,    ///<  8: Metadata
			TID_X86Mmx,      ///<  9: MMX vectors (64 bits, X86 specific)

			// Derived types... see DerivedTypes.h file.
			// Make sure FirstDerived stays up to date!
			TID_Integer,     ///< 10: Arbitrary bit width integers
			TID_Function,    ///< 11: Functions
			TID_Struct,      ///< 12: Structures
			TID_Array,       ///< 13: Arrays
			TID_Pointer,     ///< 14: Pointers
			TID_Vector       ///< 15: SIMD 'packed' format, or other vector type
		};

	public:
		void Print(std::ostream& os) const;

		LLVMContext& Context() const
		{
			return context_;
		}

		TypeId GetTypeId() const
		{
			return type_id_;
		}

		bool IsVoidType() const
		{
			return this->GetTypeId() == TID_Void;
		}
		bool IsHalfType() const
		{
			return this->GetTypeId() == TID_Half;
		}
		bool IsFloatType() const
		{
			return this->GetTypeId() == TID_Float;
		}
		bool IsDoubleType() const
		{
			return this->GetTypeId() == TID_Double;
		}
		bool IsX86Fp80Type() const
		{
			return this->GetTypeId() == TID_X86Fp80;
		}
		bool IsFp128Type() const
		{
			return this->GetTypeId() == TID_Fp128;
		}
		bool IsPpcFp128Type() const
		{
			return this->GetTypeId() == TID_PpcFp128;
		}
		bool IsFloatingPointType() const
		{
			return (this->GetTypeId() == TID_Half) || (this->GetTypeId() == TID_Float) || (this->GetTypeId() == TID_Double)
				|| (this->GetTypeId() == TID_X86Fp80) || (this->GetTypeId() == TID_Fp128) || (this->GetTypeId() == TID_PpcFp128);
		}
		bool IsX86MmxType() const
		{
			return this->GetTypeId() == TID_X86Mmx;
		}
		bool IsFpOrFpVectorType() const
		{
			return this->ScalarType()->IsFloatingPointType();
		}
		bool IsLabelType() const
		{
			return this->GetTypeId() == TID_Label;
		}
		bool IsMetadataType() const
		{
			return this->GetTypeId() == TID_Metadata;
		}
		bool IsIntegerType() const
		{
			return this->GetTypeId() == TID_Integer;
		}
		bool IsIntegerType(uint32_t bitwidth) const;
		bool IsIntOrIntVectorType() const
		{
			return this->ScalarType()->IsIntegerType();
		}
		bool IsFunctionType() const
		{
			return this->GetTypeId() == TID_Function;
		}
		bool IsStructType() const
		{
			return this->GetTypeId() == TID_Struct;
		}
		bool IsArrayType() const
		{
			return this->GetTypeId() == TID_Array;
		}
		bool IsPointerType() const
		{
			return this->GetTypeId() == TID_Pointer;
		}
		bool IsPtrOrPtrVectorType() const
		{
			return this->ScalarType()->IsPointerType();
		}
		bool IsVectorType() const
		{
			return this->GetTypeId() == TID_Vector;
		}

		bool IsEmptyType() const;
		bool IsFirstClassType() const
		{
			return (this->GetTypeId() != TID_Function) && (this->GetTypeId() != TID_Void);
		}
		bool IsSingleValueType() const
		{
			return this->IsFloatingPointType() || this->IsX86MmxType() || this->IsIntegerType()
				|| this->IsPointerType() || this->IsVectorType();
		}
		bool IsAggregateType() const
		{
			return (this->GetTypeId() == TID_Struct) || (this->GetTypeId() == TID_Array);
		}
		bool IsSized() const;

		uint32_t PrimitiveSizeInBits() const;
		uint32_t ScalarSizeInBits() const;
		int FpMantissaWidth() const;

		Type* ScalarType();
		Type const * ScalarType() const;

		typedef Type * const * subtype_iterator;
		subtype_iterator SubtypeBegin() const
		{
			return contained_types_.data();
		}
		subtype_iterator SubtypeEnd() const
		{
			return SubtypeBegin() + contained_types_.size();
		}
		ArrayRef<Type*> Subtypes() const
		{
			return ArrayRef<Type*>(contained_types_);
		}

		typedef std::reverse_iterator<subtype_iterator> subtype_reverse_iterator;
		subtype_reverse_iterator SubtypeRBegin() const
		{
			return subtype_reverse_iterator(SubtypeEnd());
		}
		subtype_reverse_iterator SubtypeREnd() const
		{
			return subtype_reverse_iterator(SubtypeBegin());
		}

		Type* ContainedType(uint32_t i) const
		{
			return contained_types_[i];
		}

		uint32_t NumContainedTypes() const
		{
			return static_cast<uint32_t>(contained_types_.size());
		}

		uint32_t IntegerBitWidth() const;

		Type* FunctionParamType(uint32_t i) const;
		uint32_t FunctionNumParams() const;
		bool IsFunctionVarArg() const;

		std::string_view StructName() const;
		uint32_t StructNumElements() const;
		Type* StructElementType(uint32_t i) const;

		Type* SequentialElementType() const;

		uint64_t ArrayNumElements() const;
		Type* ArrayElementType() const
		{
			return this->SequentialElementType();
		}

		uint32_t VectorNumElements() const;
		Type* VectorElementType() const
		{
			return this->SequentialElementType();
		}

		Type* PointerElementType() const
		{
			return this->SequentialElementType();
		}

		uint32_t PointerAddressSpace() const;

		static Type* VoidType(LLVMContext& context);
		static Type* LabelType(LLVMContext& context);
		static Type* HalfType(LLVMContext& context);
		static Type* FloatType(LLVMContext& context);
		static Type* DoubleType(LLVMContext& context);
		static Type* MetadataType(LLVMContext& context);
		static IntegerType* IntNType(LLVMContext& context, uint32_t n);
		static IntegerType* Int1Type(LLVMContext& context);
		static IntegerType* Int8Type(LLVMContext& context);
		static IntegerType* Int16Type(LLVMContext& context);
		static IntegerType* Int32Type(LLVMContext& context);
		static IntegerType* Int64Type(LLVMContext& context);

		static PointerType* HalfPtrType(LLVMContext& context, uint32_t as = 0);
		static PointerType* FloatPtrType(LLVMContext& context, uint32_t as = 0);
		static PointerType* DoublePtrType(LLVMContext& context, uint32_t as = 0);
		static PointerType* IntNPtrType(LLVMContext& context, uint32_t n, uint32_t as = 0);
		static PointerType* Int1PtrType(LLVMContext& context, uint32_t as = 0);
		static PointerType* Int8PtrType(LLVMContext& context, uint32_t as = 0);
		static PointerType* Int16PtrType(LLVMContext& context, uint32_t as = 0);
		static PointerType* Int32PtrType(LLVMContext& context, uint32_t as = 0);
		static PointerType* Int64PtrType(LLVMContext& context, uint32_t as = 0);

		PointerType* PointerTo(uint32_t addr_space = 0);

	protected:
		Type(LLVMContext& context, TypeId tid)
			: context_(context), subclass_data_(0), type_id_(tid)
		{
		}

		~Type() = default;

		uint32_t SubclassData() const
		{
			return subclass_data_;
		}
		void SubclassData(uint32_t val)
		{
			subclass_data_ = val;
		}

	private:
		bool IsSizedDerivedType() const;

	protected:
		std::vector<Type*> contained_types_;

	private:
		LLVMContext& context_;

		uint32_t subclass_data_ : 24;
		TypeId type_id_ : 8;

		// DILITHIUM_NOT_IMPLEMENTED
	};

	inline std::ostream &operator<<(std::ostream& os, Type& ty)
	{
		ty.Print(os);
		return os;
	}

	template <>
	struct isa_impl<PointerType, Type>
	{
		static bool doit(Type const & ty)
		{
			return ty.GetTypeId() == Type::TID_Pointer;
		}
	};
}

#endif		// _DILITHIUM_TYPE_HPP
