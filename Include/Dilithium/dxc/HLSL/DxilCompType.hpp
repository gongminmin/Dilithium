/**
 * @file DxilCompType.hpp
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

#ifndef _DILITHIUM_DXIL_COMP_TYPE_HPP
#define _DILITHIUM_DXIL_COMP_TYPE_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilConstants.hpp>

namespace Dilithium
{
	class Type;
	class PointerType;
	class LLVMContext;

	class DxilCompType
	{
	public:
		DxilCompType();
		DxilCompType(ComponentType kind);
		DxilCompType(uint32_t kind);

		bool operator==(DxilCompType const & rhs) const;

		ComponentType GetKind() const;

		static DxilCompType GetInvalid();
		static DxilCompType GetF16();
		static DxilCompType GetF32();
		static DxilCompType GetF64();
		static DxilCompType GetI16();
		static DxilCompType GetI32();
		static DxilCompType GetI64();
		static DxilCompType GetU16();
		static DxilCompType GetU32();
		static DxilCompType GetU64();
		static DxilCompType GetI1();
		static DxilCompType GetSNormF16();
		static DxilCompType GetUNormF16();
		static DxilCompType GetSNormF32();
		static DxilCompType GetUNormF32();
		static DxilCompType GetSNormF64();
		static DxilCompType GetUNormF64();

		bool IsInvalid() const;
		bool IsFloatTy() const;
		bool IsIntTy() const;
		bool IsSIntTy() const;
		bool IsUIntTy() const;
		bool IsBoolTy() const;

		bool IsSNorm() const;
		bool IsUNorm() const;
		bool Is64Bit() const;

		DxilCompType GetBaseCompType() const;
		bool HasMinPrec() const;
		Type* GetLLVMType(LLVMContext& context) const;
		PointerType* GetLLVMPtrType(LLVMContext& context, uint32_t addr_space = 0) const;
		Type* GetLLVMBaseType(LLVMContext& context) const;

		static DxilCompType GetCompType(Type* type);

		char const * GetName() const;
		char const * GetHLSLName() const;

	private:
		ComponentType kind_;
	};
}

#endif		// _DILITHIUM_DXIL_COMP_TYPE_HPP
