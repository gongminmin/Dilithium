/**
 * @file Function.cpp
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
#include <Dilithium/Function.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/SymbolTableList.hpp>
#include "LLVMContextImpl.hpp"

namespace Dilithium 
{
	Function::Function(FunctionType* ty, LinkageTypes linkage, std::string_view name, LLVMModule* mod)
		: GlobalObject(PointerType::Get(ty, 0), Value::FunctionVal, 0, 1, linkage, name),
			ty_(ty)
	{
		BOOST_ASSERT_MSG(FunctionType::IsValidReturnType(this->GetReturnType()), "invalid return type");
		this->GlobalObjectSubClassData(0);

		if (ty->NumParams())
		{
			this->SetValueSubclassData(1);
		}

		if (mod)
		{
			mod->FunctionList().push_back(std::unique_ptr<Function>(this));
			AddToSymbolTableList(this, mod);
		}
	}

	Function::~Function()
	{
		RemoveFromSymbolTableList(this);

		this->DropAllReferences();
		argument_list_.clear();

		// FIXME: needed by operator delete
		this->GlobalVariableOrFunctionNumOperands(1);
	}

	Function* Function::Create(FunctionType* ty, LinkageTypes linkage, std::string_view name, LLVMModule* mod)
	{
		return new Function(ty, linkage, name, mod);
	}

	bool Function::HasPersonalityFn() const
	{
		return this->NumOperands() != 0;
	}

	Constant* Function::GetPersonalityFn() const
	{
		BOOST_ASSERT(this->HasPersonalityFn());
		return cast<Constant>(this->Op<0>());
	}

	void Function::SetPersonalityFn(Constant* c)
	{
		if (!c)
		{
			if (this->HasPersonalityFn())
			{
				this->Op<0>().Set(nullptr);
				this->GlobalVariableOrFunctionNumOperands(0);
			}
		}
		else
		{
			if (!this->HasPersonalityFn())
			{
				this->GlobalVariableOrFunctionNumOperands(1);
			}
			this->Op<0>().Set(c);
		}
	}

	Type* Function::GetReturnType() const
	{
		return ty_->ReturnType();
	}

	FunctionType* Function::GetFunctionType() const
	{
		return ty_;
	}

	LLVMContext& Function::Context() const
	{
		return this->GetType()->Context();
	}

	bool Function::IsVarArg() const
	{
		return ty_->IsVarArg();
	}

	bool Function::IsMaterializable() const
	{
		return this->GlobalObjectSubClassData() & IsMaterializableBit;
	}

	void Function::IsMaterializable(bool m)
	{
		this->GlobalObjectSubClassData((~IsMaterializableBit & this->GlobalObjectSubClassData()) | (m ? IsMaterializableBit : 0));
	}

	CallingConv::ID Function::GetCallingConv() const
	{
		return static_cast<CallingConv::ID>(this->GetSubclassDataFromValue() >> 3);
	}

	void Function::SetCallingConv(CallingConv::ID cc)
	{
		this->SetValueSubclassData((this->GetSubclassDataFromValue() & 7) | (static_cast<uint16_t>(cc) << 3));
	}

	Function::ArgumentListType const & Function::ArgumentList() const
	{
		CheckLazyArguments();
		return argument_list_;
	}

	Function::ArgumentListType& Function::ArgumentList()
	{
		CheckLazyArguments();
		return argument_list_;
	}

	Function::arg_iterator Function::ArgBegin()
	{
		CheckLazyArguments();
		return argument_list_.begin();
	}

	Function::const_arg_iterator Function::ArgBegin() const
	{
		CheckLazyArguments();
		return argument_list_.begin();
	}

	Function::arg_iterator Function::ArgEnd()
	{
		CheckLazyArguments();
		return argument_list_.end();
	}

	Function::const_arg_iterator Function::ArgEnd() const
	{
		CheckLazyArguments();
		return argument_list_.end();
	}

	bool Function::HasPrefixData() const
	{
		return GetSubclassDataFromValue() & HasMetadataBit;
	}

	Constant* Function::GetPrefixData() const
	{
		BOOST_ASSERT(this->HasPrefixData());
		auto const & pd_map = this->Context().Impl().prefix_data_map;
		BOOST_ASSERT(pd_map.find(this) != pd_map.end());
		return cast<Constant>(pd_map.find(this)->second->ReturnValue());
	}

	void Function::SetPrefixData(Constant* prefix_data)
	{
		if (!prefix_data && !this->HasPrefixData())
		{
			return;
		}

		uint16_t sc_data = this->GetSubclassDataFromValue();
		auto& pd_map = this->Context().Impl().prefix_data_map;
		auto& pd_holder = pd_map[this];
		if (prefix_data)
		{
			if (pd_holder)
			{
				pd_holder->Operand(0, prefix_data);
			}
			else
			{
				pd_holder = ReturnInst::Create(this->Context(), prefix_data);
			}
			sc_data |= HasMetadataBit;
		}
		else
		{
			delete pd_holder;
			pd_map.erase(this);
			sc_data &= ~HasMetadataBit;
		}
		this->SetValueSubclassData(sc_data);
	}

	bool Function::HasPrologueData() const
	{
		return GetSubclassDataFromValue() & HasPrologueDataBit;
	}

	Constant* Function::GetPrologueData() const
	{
		BOOST_ASSERT(this->HasPrologueData());

		auto const & pd_map = this->Context().Impl().prologue_data_map;
		BOOST_ASSERT(pd_map.find(this) != pd_map.end());
		return cast<Constant>(pd_map.find(this)->second->ReturnValue());
	}

	void Function::SetPrologueData(Constant* prologue_data)
	{
		if (!prologue_data && !this->HasPrologueData())
		{
			return;
		}

		uint16_t pd_data = this->GetSubclassDataFromValue();
		auto& pd_map = this->Context().Impl().prologue_data_map;
		auto& pd_holder = pd_map[this];
		if (prologue_data)
		{
			if (pd_holder)
			{
				pd_holder->Operand(0, prologue_data);
			}
			else
			{
				pd_holder = ReturnInst::Create(this->Context(), prologue_data);
			}
			pd_data |= HasPrologueDataBit;
		}
		else
		{
			delete pd_holder;
			pd_map.erase(this);
			pd_data &= ~HasPrologueDataBit;
		}
		this->SetValueSubclassData(pd_data);
	}

	void Function::DropAllReferences()
	{
		this->IsMaterializable(false);

		for (auto iter = this->begin(), end_iter = this->end(); iter != end_iter; ++ iter)
		{
			iter->get()->DropAllReferences();
		}

		basic_blocks_.clear();

		this->SetPrefixData(nullptr);
		this->SetPrologueData(nullptr);

		this->ClearMetadata();

		this->SetPersonalityFn(nullptr);
	}

	void Function::GetAllMetadata(boost::container::small_vector_base<std::pair<uint32_t, MDNode*>>& mds) const
	{
		mds.clear();

		if (!this->HasMetadata())
		{
			return;
		}

		Context().Impl().function_metadata[this].GetAll(mds);
	}

	void Function::CheckLazyArguments() const
	{
		if (this->HasLazyArguments())
		{
			this->BuildLazyArguments();
		}
	}

	void Function::BuildLazyArguments() const
	{
		FunctionType* ft = this->GetFunctionType();
		for (uint32_t i = 0, e = ft->NumParams(); i != e; ++ i)
		{
			BOOST_ASSERT_MSG(!ft->ParamType(i)->IsVoidType(), "Cannot have void typed arguments!");
			argument_list_.push_back(std::make_unique<Argument>(ft->ParamType(i)));
		}

		uint16_t sdc = this->GetSubclassDataFromValue();
		sdc &= ~(1 << 0);
		const_cast<Function*>(this)->SetValueSubclassData(sdc);
	}

	void Function::ClearMetadata()
	{
		if (this->HasMetadata())
		{
			this->Context().Impl().function_metadata.erase(this);
			this->HasMetadataHashEntry(false);
		}
	}
}
