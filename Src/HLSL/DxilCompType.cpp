/**
 * @file DxilCompType.cpp
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

#include <Dilithium/dxc/HLSL/DxilCompType.hpp>
#include <Dilithium/ErrorHandling.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/DerivedType.hpp>

#include <boost/assert.hpp>

namespace Dilithium
{
	DxilCompType::DxilCompType()
		: kind_(ComponentType::Invalid)
	{
	}

	DxilCompType::DxilCompType(ComponentType kind)
		: kind_(kind)
	{
		BOOST_ASSERT_MSG((kind >= ComponentType::Invalid) && (kind_ < ComponentType::LastEntry), "The caller passed out-of-range value");
	}

	DxilCompType::DxilCompType(uint32_t kind)
		: DxilCompType(static_cast<ComponentType>(kind))
	{
	}

	bool DxilCompType::operator==(DxilCompType const & rhs) const
	{
		return kind_ == rhs.kind_;
	}

	ComponentType DxilCompType::GetKind() const
	{
	  return kind_;
	}

	DxilCompType DxilCompType::GetInvalid()
	{
		return DxilCompType();
	}

	DxilCompType DxilCompType::GetF16()
	{
		return DxilCompType(ComponentType::F16);
	}

	DxilCompType DxilCompType::GetF32()
	{
		return DxilCompType(ComponentType::F32);
	}

	DxilCompType DxilCompType::GetF64()
	{
		return DxilCompType(ComponentType::F64);
	}

	DxilCompType DxilCompType::GetI16()
	{
		return DxilCompType(ComponentType::I16);
	}

	DxilCompType DxilCompType::GetI32()
	{
		return DxilCompType(ComponentType::I32);
	}

	DxilCompType DxilCompType::GetI64()
	{
		return DxilCompType(ComponentType::I64);
	}

	DxilCompType DxilCompType::GetU16()
	{
		return DxilCompType(ComponentType::U16);
	}

	DxilCompType DxilCompType::GetU32()
	{
		return DxilCompType(ComponentType::U32);
	}

	DxilCompType DxilCompType::GetU64()
	{
		return DxilCompType(ComponentType::U64);
	}

	DxilCompType DxilCompType::GetI1()
	{
		return DxilCompType(ComponentType::I1);
	}

	DxilCompType DxilCompType::GetSNormF16()
	{
		return DxilCompType(ComponentType::SNormF16);
	}

	DxilCompType DxilCompType::GetUNormF16()
	{
		return DxilCompType(ComponentType::UNormF16);
	}

	DxilCompType DxilCompType::GetSNormF32()
	{
		return DxilCompType(ComponentType::SNormF32);
	}

	DxilCompType DxilCompType::GetUNormF32()
	{
		return DxilCompType(ComponentType::UNormF32);
	}

	DxilCompType DxilCompType::GetSNormF64()
	{
		return DxilCompType(ComponentType::SNormF64);
	}

	DxilCompType DxilCompType::GetUNormF64()
	{
		return DxilCompType(ComponentType::UNormF64);
	}

	bool DxilCompType::IsInvalid() const
	{
		return kind_ == ComponentType::Invalid;
	}

	bool DxilCompType::IsFloatTy() const
	{
		return (kind_ == ComponentType::F16) || (kind_ == ComponentType::F32) || (kind_ == ComponentType::F64);
	}

	bool DxilCompType::IsIntTy() const
	{
		return this->IsSIntTy() || this->IsUIntTy();
	}

	bool DxilCompType::IsSIntTy() const
	{
		return (kind_ == ComponentType::I16) || (kind_ == ComponentType::I32) || (kind_ == ComponentType::I64);
	}

	bool DxilCompType::IsUIntTy() const
	{
		return (kind_ == ComponentType::U16) || (kind_ == ComponentType::U32) || (kind_ == ComponentType::U64);
	}

	bool DxilCompType::IsBoolTy() const
	{
		return kind_ == ComponentType::I1;
	}

	bool DxilCompType::IsSNorm() const
	{
		return (kind_ == ComponentType::SNormF16) || (kind_ == ComponentType::SNormF32) || (kind_ == ComponentType::SNormF64);
	}

	bool DxilCompType::IsUNorm() const
	{
		return (kind_ == ComponentType::UNormF16) || (kind_ == ComponentType::UNormF32) || (kind_ == ComponentType::UNormF64);
	}

	bool DxilCompType::Is64Bit() const
	{
		switch (kind_)
		{
		case ComponentType::F64:
		case ComponentType::SNormF64:
		case ComponentType::UNormF64:
		case ComponentType::I64:
		case ComponentType::U64:
			return true;

		default:
			return false;
		}
	}

	DxilCompType DxilCompType::GetBaseCompType() const
	{
		switch (kind_)
		{
		case ComponentType::I1:
			return DxilCompType(ComponentType::I1);

		case ComponentType::I16:
		case ComponentType::I32:
			return DxilCompType(ComponentType::I32);

		case ComponentType::I64:
			return DxilCompType(ComponentType::I64);

		case ComponentType::U16:
		case ComponentType::U32:
			return DxilCompType(ComponentType::U32);

		case ComponentType::U64:
			return DxilCompType(ComponentType::U64);

		case ComponentType::SNormF16:
		case ComponentType::UNormF16:
		case ComponentType::F16:
		case ComponentType::SNormF32:
		case ComponentType::UNormF32:
		case ComponentType::F32:
			return DxilCompType(ComponentType::F32);

		case ComponentType::SNormF64:
		case ComponentType::UNormF64:
		case ComponentType::F64:
			return DxilCompType(ComponentType::F64);

		default:
			DILITHIUM_UNREACHABLE("Invalid type kind");
		}
	}

	bool DxilCompType::HasMinPrec() const
	{
		switch (kind_)
		{
		case ComponentType::I16:
		case ComponentType::U16:
		case ComponentType::F16:
		case ComponentType::SNormF16:
		case ComponentType::UNormF16:
			return true;

		case ComponentType::I1:
		case ComponentType::I32:
		case ComponentType::U32:
		case ComponentType::I64:
		case ComponentType::U64:
		case ComponentType::F32:
		case ComponentType::F64:
		case ComponentType::SNormF32:
		case ComponentType::UNormF32:
		case ComponentType::SNormF64:
		case ComponentType::UNormF64:
			return false;

		default:
			DILITHIUM_UNREACHABLE("Invalid comp type");
		}
	}

	Type* DxilCompType::GetLLVMType(LLVMContext& context) const
	{
		// TODO: decide if we should distinguish between signed and unsigned types in this api.

		switch (kind_)
		{
		case ComponentType::I1:
			return Type::Int1Type(context);

		case ComponentType::I16:
		case ComponentType::U16:
			return Type::Int16Type(context);

		case ComponentType::I32:
		case ComponentType::U32:
			return Type::Int32Type(context);

		case ComponentType::I64:
		case ComponentType::U64:
			return Type::Int64Type(context);

		case ComponentType::SNormF16:
		case ComponentType::UNormF16:
		case ComponentType::F16:
			return Type::HalfType(context);

		case ComponentType::SNormF32:
		case ComponentType::UNormF32:
		case ComponentType::F32:
			return Type::FloatType(context);

		case ComponentType::SNormF64:
		case ComponentType::UNormF64:
		case ComponentType::F64:
			return Type::DoubleType(context);

		default:
			DILITHIUM_UNREACHABLE("Invalid type kind");
		}
	}

	PointerType* DxilCompType::GetLLVMPtrType(LLVMContext& context, uint32_t addr_space) const
	{
		switch (kind_)
		{
		case ComponentType::I1:
			return Type::Int1PtrType(context, addr_space);

		case ComponentType::I16:
		case ComponentType::U16:
			return Type::Int16PtrType(context, addr_space);

		case ComponentType::I32:
		case ComponentType::U32:
			return Type::Int32PtrType(context, addr_space);

		case ComponentType::I64:
		case ComponentType::U64:
			return Type::Int64PtrType(context, addr_space);

		case ComponentType::SNormF16:
		case ComponentType::UNormF16:
		case ComponentType::F16:
			return Type::HalfPtrType(context, addr_space);

		case ComponentType::SNormF32:
		case ComponentType::UNormF32:
		case ComponentType::F32:
			return Type::FloatPtrType(context, addr_space);

		case ComponentType::SNormF64:
		case ComponentType::UNormF64:
		case ComponentType::F64:
			return Type::DoublePtrType(context, addr_space);

		default:
			DILITHIUM_UNREACHABLE("Invalid type kind");
		}
	}

	Type* DxilCompType::GetLLVMBaseType(LLVMContext& context) const
	{
		return this->GetBaseCompType().GetLLVMType(context);
	}

	DxilCompType DxilCompType::GetCompType(Type* type)
	{
		LLVMContext& context = type->Context();
		if (type == Type::Int1Type(context))
		{
			return DxilCompType(ComponentType::I1);
		}
		else if (type == Type::Int16Type(context))
		{
			return DxilCompType(ComponentType::I16);
		}
		else if (type == Type::Int32Type(context))
		{
			return DxilCompType(ComponentType::I32);
		}
		else if (type == Type::Int64Type(context))
		{
			return DxilCompType(ComponentType::I64);
		}
		else if (type == Type::HalfType(context))
		{
			return DxilCompType(ComponentType::F16);
		}
		else if (type == Type::FloatType(context))
		{
			return DxilCompType(ComponentType::F32);
		}
		else if (type == Type::DoubleType(context))
		{
			return DxilCompType(ComponentType::F64);
		}
		else
		{
			DILITHIUM_UNREACHABLE("Invalid type kind");
		}
	}

	char const * DxilCompType::GetName() const
	{
		static char const * type_kind_names[] =
		{
			"invalid",
			"i1", "i16", "u16", "i32", "u32", "i64", "u64",
			"f16", "f32", "f64",
			"snorm_f16", "unorm_f16", "snorm_f32", "unorm_f32", "snorm_f64", "unorm_f64",
		};
		static_assert(sizeof(type_kind_names) / sizeof(type_kind_names[0]) == static_cast<uint32_t>(ComponentType::LastEntry),
			"Wrong size of s_TypeKindNames");

		return type_kind_names[static_cast<uint32_t>(kind_)];
	}

	char const * DxilCompType::GetHLSLName() const
	{
		static char const * type_kind_hlsl_names[] =
		{
			"unknown",
			"bool", "min16i", "min16i", "int", "uint", "int64_t", "uint64_t",
			"min16f", "float", "double",
			"snorm_min16f", "unorm_min16f", "snorm_float", "unorm_float", "snorm_double", "unorm_double",
		};
		static_assert(sizeof(type_kind_hlsl_names) / sizeof(type_kind_hlsl_names[0]) == static_cast<uint32_t>(ComponentType::LastEntry),
			"Wrong size of s_TypeKindHLSLNames");

		return type_kind_hlsl_names[static_cast<uint32_t>(kind_)];
	}
}
