/**
 * @file DxilOperations.hpp
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

#ifndef _DILITHIUM_DXIL_OPERATIONS_HPP
#define _DILITHIUM_DXIL_OPERATIONS_HPP

#pragma once

namespace Dilithium
{
	class LLVMContext;
	class LLVMModule;
	class Type;
	class Function;
	class Constant;
	class Value;
	class Instruction;
};

#include <Dilithium/Attributes.hpp>
#include <Dilithium/dxc/HLSL/DxilConstants.hpp>

namespace Dilithium
{
	class OP
	{
	public:
		OP() = delete;
		OP(LLVMContext& ctx, LLVMModule* module);

		LLVMContext& GetContext()
		{
			return context_;
		}

		static char const * GetOpCodeName(OpCode op);
		static bool IsDxilOpFunc(Function const * func);

	private:
		LLVMContext& context_;
		LLVMModule* module_;

		static uint32_t constexpr NumTypeOverloads = 9;

	private:
		// Static properties.
		struct OpCodeProperty
		{
			OpCode op_code;
			char const * op_code_name;
			OpCodeClass op_code_class;
			char const * op_code_class_name;
			bool allow_overload[NumTypeOverloads];   // void, h,f,d, i1, i8,i16,i32,i64
			Attribute::AttrKind func_attr;
		};
		static OpCodeProperty const op_code_props_[static_cast<uint32_t>(OpCode::NumOpCodes)];

		static char const * name_prefix_;
	};
}

#endif		// _DILITHIUM_DXIL_OPERATIONS_HPP
