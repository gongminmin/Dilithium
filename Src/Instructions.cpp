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

#include <Dilithium/Dilithium.hpp>
#include <Dilithium/Casting.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/Instructions.hpp>

namespace Dilithium 
{

	ReturnInst::ReturnInst(LLVMContext& context, Value* ret_val, Instruction* insert_before)
		: TerminatorInst(Type::VoidType(context), Instruction::Ret, !!ret_val, !!ret_val, insert_before)
	{
		if (ret_val)
		{
			this->Op<0>().Set(ret_val);
		}
	}

	ReturnInst::ReturnInst(LLVMContext& context, Value* ret_val, BasicBlock* insert_at_end)
		: TerminatorInst(Type::VoidType(context), Instruction::Ret, !!ret_val, !!ret_val, insert_at_end)
	{
		if (ret_val)
		{
			this->Op<0>().Set(ret_val);
		}
	}

	ReturnInst::ReturnInst(LLVMContext& context, BasicBlock* insert_at_end)
		: TerminatorInst(Type::VoidType(context), Instruction::Ret, 0, 0, insert_at_end)
	{
	}

	ReturnInst::ReturnInst(ReturnInst const & rhs)
		: TerminatorInst(Type::VoidType(rhs.Context()), Instruction::Ret, rhs.NumOperands(), rhs.NumOperands())
	{
		if (rhs.NumOperands())
		{
			this->Op<0>() = rhs.Op<0>();
		}
		subclass_optional_data_ = rhs.subclass_optional_data_;
	}

	ReturnInst::~ReturnInst()
	{
	}

	ReturnInst* ReturnInst::Create(LLVMContext& context, Value* ret_val, Instruction* insert_before)
	{
		return new ReturnInst(context, ret_val, insert_before);
	}

	ReturnInst* ReturnInst::Create(LLVMContext& context, Value* ret_val, BasicBlock* insert_at_end)
	{
		return new ReturnInst(context, ret_val, insert_at_end);
	}

	ReturnInst* ReturnInst::Create(LLVMContext& context, BasicBlock* insert_at_end)
	{
		return new ReturnInst(context, insert_at_end);
	}


	CallInst::CallInst(FunctionType* ty, Value* func, ArrayRef<Value*> args, std::string_view name, Instruction* insert_before)
		: Instruction(ty->ReturnType(), Instruction::Call,
			static_cast<uint32_t>(args.size() + 1), static_cast<uint32_t>(args.size() + 1), insert_before)
	{
		this->Init(ty, func, args, name);
	}

	CallInst::CallInst(Value* func, ArrayRef<Value*> args, std::string_view name, Instruction* insert_before)
		: CallInst(cast<FunctionType>(cast<PointerType>(func->GetType())->ElementType()), func, args, name, insert_before)
	{
	}

	CallInst::CallInst(Value* func, ArrayRef<Value*> args, std::string_view name, BasicBlock* insert_at_end)
		: Instruction(cast<FunctionType>(cast<PointerType>(func->GetType())->ElementType())->ReturnType(), Instruction::Call,
			static_cast<uint32_t>(args.size() + 1), static_cast<uint32_t>(args.size() + 1), insert_at_end)
	{
		this->Init(func, args, name);
	}

	CallInst::CallInst(Value* func, std::string_view name, Instruction* insert_before)
		: Instruction(cast<FunctionType>(cast<PointerType>(func->GetType())->ElementType())->ReturnType(), Instruction::Call,
			1, 1, insert_before)
	{
		this->Init(func, name);
	}

	CallInst::CallInst(Value* func, std::string_view name, BasicBlock* insert_at_end)
		: Instruction(cast<FunctionType>(cast<PointerType>(func->GetType())->ElementType())->ReturnType(), Instruction::Call,
			1, 1, insert_at_end)
	{
		this->Init(func, name);
	}

	CallInst::CallInst(CallInst const & rhs)
		: Instruction(rhs.GetType(), Instruction::Call, rhs.NumOperands(), rhs.NumOperands()),
			attr_list_(rhs.attr_list_), fty_(rhs.fty_)
	{
		this->SetTailCallKind(rhs.GetTailCallKind());
		this->SetCallingConv(rhs.GetCallingConv());

		std::copy(rhs.OpBegin(), rhs.OpEnd(), this->OpBegin());
		subclass_optional_data_ = rhs.subclass_optional_data_;
	}

	CallInst::~CallInst()
	{
	}

	void CallInst::Init(Value* func, ArrayRef<Value*> args, std::string_view name)
	{
		this->Init(cast<FunctionType>(cast<PointerType>(func->GetType())->ElementType()), func, args, name);
	}

	void CallInst::Init(FunctionType* fty, Value* func, ArrayRef<Value*> args, std::string_view name)
	{
		fty_ = fty;
		BOOST_ASSERT_MSG(this->NumOperands() == args.size() + 1, "NumOperands not set up?");
		this->Op<-1>().Set(func);

#ifdef DILITHIUM_DEBUG
		BOOST_ASSERT_MSG((args.size() == fty_->NumParams()) || (fty_->IsVarArg() && (args.size() > fty_->NumParams())),
			"Calling a function with bad signature!");

		for (uint32_t i = 0; i != static_cast<uint32_t>(args.size()); ++ i)
		{
			BOOST_ASSERT_MSG((i >= fty_->NumParams()) || (fty_->ParamType(i) == args[i]->GetType()),
				"Calling a function with a bad signature!");
		}
#endif

		auto dst_iter = this->OpBegin();
		for (auto v_iter = args.begin(); v_iter != args.end(); ++ v_iter, ++ dst_iter)
		{
			dst_iter->Set(*v_iter);
		}
		this->Name(name);
	}

	void CallInst::Init(Value* func, std::string_view name)
	{
		fty_ = cast<FunctionType>(cast<PointerType>(func->GetType())->ElementType());
		BOOST_ASSERT_MSG(this->NumOperands() == 1, "NumOperands not set up?");
		this->Op<-1>().Set(func);

		BOOST_ASSERT_MSG(fty_->NumParams() == 0, "Calling a function with bad signature");

		this->Name(name);
	}

	CallInst* CallInst::Create(Value* func, ArrayRef<Value*> args, std::string_view name, Instruction* insert_before)
	{
		return Create(cast<FunctionType>(cast<PointerType>(func->GetType())->ElementType()), func, args, name, insert_before);
	}

	CallInst* CallInst::Create(FunctionType* ty, Value* func, ArrayRef<Value*> args, std::string_view name,
		Instruction* insert_before)
	{
		return new CallInst(ty, func, args, name, insert_before);
	}

	CallInst* CallInst::Create(Value* func, ArrayRef<Value*> args, std::string_view name, BasicBlock* insert_at_end)
	{
		return new CallInst(func, args, name, insert_at_end);
	}

	CallInst* CallInst::Create(Value* func, std::string_view name, Instruction* insert_before)
	{
		return new CallInst(func, name, insert_before);
	}

	CallInst* CallInst::Create(Value* func, std::string_view name, BasicBlock* insert_at_end)
	{
		return new CallInst(func, name, insert_at_end);
	}

	CallInst::TailCallKind CallInst::GetTailCallKind() const
	{
		return static_cast<TailCallKind>(this->SubclassDataFromInstruction() & 3);
	}

	bool CallInst::IsTailCall() const
	{
		return (this->SubclassDataFromInstruction() & 3) != TCK_None;
	}
	bool CallInst::IsMustTailCall() const
	{
		return (this->SubclassDataFromInstruction() & 3) == TCK_MustTail;
	}
	void CallInst::SetTailCall(bool is_tc)
	{
		this->InstructionSubclassData(static_cast<uint16_t>((this->SubclassDataFromInstruction() & ~3)
			| static_cast<uint32_t>(is_tc ? TCK_Tail : TCK_None)));
	}

	void CallInst::SetTailCallKind(TailCallKind tck)
	{
		this->InstructionSubclassData(static_cast<uint16_t>((this->SubclassDataFromInstruction() & ~3) | static_cast<uint32_t>(tck)));
	}

	CallingConv::ID CallInst::GetCallingConv() const
	{
		return static_cast<CallingConv::ID>(this->SubclassDataFromInstruction() >> 2);
	}

	void CallInst::SetCallingConv(CallingConv::ID cc)
	{
		this->InstructionSubclassData(
			static_cast<uint16_t>((this->SubclassDataFromInstruction() & 3) | (static_cast<uint32_t>(cc) << 2)));
	}
}
