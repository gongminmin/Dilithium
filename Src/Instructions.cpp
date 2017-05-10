/**
 * @file Instructions.cpp
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

#include <Dilithium/Instructions.hpp>

#include <Dilithium/ErrorHandling.hpp>
#include <Dilithium/Util.hpp>

namespace Dilithium 
{
	ReturnInst* ReturnInst::Create(LLVMContext& context, Value* ret_val, Instruction* insert_before)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(ret_val);
		DILITHIUM_UNUSED(insert_before);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	ReturnInst* ReturnInst::Create(LLVMContext& context, Value* ret_val, BasicBlock* insert_at_end)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(ret_val);
		DILITHIUM_UNUSED(insert_at_end);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	ReturnInst* ReturnInst::Create(LLVMContext& context, BasicBlock* insert_at_end)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(insert_at_end);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	CallInst* CallInst::Create(Value* func, ArrayRef<Value*> args, std::string_view name_str, Instruction* insert_before)
	{
		DILITHIUM_UNUSED(func);
		DILITHIUM_UNUSED(args);
		DILITHIUM_UNUSED(name_str);
		DILITHIUM_UNUSED(insert_before);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	CallInst* CallInst::Create(FunctionType* ty, Value* func, ArrayRef<Value*> args, std::string_view name_str,
		Instruction* insert_before)
	{
		DILITHIUM_UNUSED(ty);
		DILITHIUM_UNUSED(func);
		DILITHIUM_UNUSED(args);
		DILITHIUM_UNUSED(name_str);
		DILITHIUM_UNUSED(insert_before);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	CallInst* CallInst::Create(Value* func, ArrayRef<Value*> args, std::string_view name_str, BasicBlock* insert_at_end)
	{
		DILITHIUM_UNUSED(func);
		DILITHIUM_UNUSED(args);
		DILITHIUM_UNUSED(name_str);
		DILITHIUM_UNUSED(insert_at_end);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	CallInst* CallInst::Create(Value* func, std::string_view name_str, Instruction* insert_before)
	{
		DILITHIUM_UNUSED(func);
		DILITHIUM_UNUSED(name_str);
		DILITHIUM_UNUSED(insert_before);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	CallInst* CallInst::Create(Value* func, std::string_view name_str, BasicBlock* insert_at_end)
	{
		DILITHIUM_UNUSED(func);
		DILITHIUM_UNUSED(name_str);
		DILITHIUM_UNUSED(insert_at_end);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	CallInst::TailCallKind CallInst::GetTailCallKind() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void CallInst::SetTailCallKind(TailCallKind tck)
	{
		DILITHIUM_UNUSED(tck);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	CallingConv::ID CallInst::GetCallingConv() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void CallInst::SetCallingConv(CallingConv::ID cc)
	{
		DILITHIUM_UNUSED(cc);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
